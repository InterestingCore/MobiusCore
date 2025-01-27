/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#include "ScriptMgr.h"
#include "black_temple.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum Spells
{
    // Wrathbone Flayer
    SPELL_CLEAVE                = 15496,
    SPELL_IGNORED               = 39544,
    SPELL_SUMMON_CHANNEL        = 40094,

    // Angered Soul Fragment
    SPELL_GREATER_INVISIBILITY  = 41253,
    SPELL_ANGER                 = 41986,

    // Illidari Nightlord
    SPELL_SHADOW_INFERNO_DAMAGE = 39646
};

enum Creatures
{
    NPC_BLOOD_MAGE               = 22945,
    NPC_DEATHSHAPER              = 22882
};

enum Events
{
    // Wrathbone Flayer
    EVENT_GET_CHANNELERS = 1,
    EVENT_SET_CHANNELERS,
    EVENT_CLEAVE,
    EVENT_IGNORED
};

enum Misc
{
    GROUP_OUT_OF_COMBAT = 1
};

// ########################################################
// Wrathbone Flayer
// ########################################################

class npc_wrathbone_flayer : public CreatureScript
{
public:
    npc_wrathbone_flayer() : CreatureScript("npc_wrathbone_flayer") { }

    struct npc_wrathbone_flayerAI : public ScriptedAI
    {
        npc_wrathbone_flayerAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
            _instance = creature->GetInstanceScript();
        }

        void Initialize()
        {
            _enteredCombat = false;
        }

        void Reset() override
        {
            _events.ScheduleEvent(EVENT_GET_CHANNELERS, 3000);
            Initialize();
            _bloodmageList.clear();
            _deathshaperList.clear();
        }

        void JustDied(Unit* /*killer*/) override { }

        void EnterCombat(Unit* /*who*/) override
        {
            _events.ScheduleEvent(EVENT_CLEAVE, 5000);
            _events.ScheduleEvent(EVENT_IGNORED, 7000);
            _enteredCombat = true;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!_enteredCombat)
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_GET_CHANNELERS:
                        {
                            std::list<Creature*> BloodMageList;
                            me->GetCreatureListWithEntryInGrid(BloodMageList, NPC_BLOOD_MAGE, 15.0f);

                            if (!BloodMageList.empty())
                                for (std::list<Creature*>::const_iterator itr = BloodMageList.begin(); itr != BloodMageList.end(); ++itr)
                                {
                                    _bloodmageList.push_back((*itr)->GetGUID());
                                    if ((*itr)->isDead())
                                        (*itr)->Respawn();
                                }

                            std::list<Creature*> DeathShaperList;
                            me->GetCreatureListWithEntryInGrid(DeathShaperList, NPC_DEATHSHAPER, 15.0f);

                            if (!DeathShaperList.empty())
                                for (std::list<Creature*>::const_iterator itr = DeathShaperList.begin(); itr != DeathShaperList.end(); ++itr)
                                {
                                    _deathshaperList.push_back((*itr)->GetGUID());
                                    if ((*itr)->isDead())
                                        (*itr)->Respawn();
                                }

                            _events.ScheduleEvent(EVENT_SET_CHANNELERS, 3000);

                            break;
                        }
                        case EVENT_SET_CHANNELERS:
                        {
                            for (ObjectGuid guid : _bloodmageList)
                                if (Creature* bloodmage = ObjectAccessor::GetCreature(*me, guid))
                                    bloodmage->CastSpell((Unit*)NULL, SPELL_SUMMON_CHANNEL);

                            for (ObjectGuid guid : _deathshaperList)
                                if (Creature* deathshaper = ObjectAccessor::GetCreature(*me, guid))
                                    deathshaper->CastSpell((Unit*)NULL, SPELL_SUMMON_CHANNEL);

                            _events.ScheduleEvent(EVENT_SET_CHANNELERS, 12000);

                            break;
                        }
                        default:
                            break;
                    }
                }
            }

            if (!UpdateVictim())
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CLEAVE:
                        DoCastVictim(SPELL_CLEAVE);
                        _events.ScheduleEvent(EVENT_CLEAVE, urand (1000, 2000));
                        break;
                    case EVENT_IGNORED:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_IGNORED);
                        _events.ScheduleEvent(EVENT_IGNORED, 10000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

        private:
            InstanceScript* _instance;
            EventMap _events;
            GuidList _bloodmageList;
            GuidList _deathshaperList;
            bool _enteredCombat;
        };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetBlackTempleAI<npc_wrathbone_flayerAI>(creature);
    }
};

class npc_angered_soul_fragment : public CreatureScript
{
public:
    npc_angered_soul_fragment() : CreatureScript("npc_angered_soul_fragment") { }

    struct npc_angered_soul_fragmentAI : public ScriptedAI
    {
        npc_angered_soul_fragmentAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            _scheduler.CancelAll();

            _scheduler.Schedule(Seconds(1), GROUP_OUT_OF_COMBAT, [this](TaskContext invi)
            {
                DoCastSelf(SPELL_GREATER_INVISIBILITY);

                /* Workaround - On Retail creature appear and "vanish" again periodically, but i cant find packets
                with UPDATE_AURA on sniffs about it */
                _scheduler.Schedule(Seconds(5), Seconds(10), GROUP_OUT_OF_COMBAT, [this](TaskContext /*context*/)
                {
                    me->RemoveAurasDueToSpell(SPELL_GREATER_INVISIBILITY);
                });

                invi.Repeat(Seconds(15), Seconds(25));
            });
        }

        void EnterCombat(Unit* /*who*/) override
        {
            me->RemoveAurasDueToSpell(SPELL_GREATER_INVISIBILITY);

            _scheduler.CancelGroup(GROUP_OUT_OF_COMBAT);
            _scheduler.Schedule(Seconds(1), [this](TaskContext anger)
            {
                Unit* target = me->GetVictim();
                if (target && me->IsWithinMeleeRange(target))
                    DoCastSelf(SPELL_ANGER);
                else
                    anger.Repeat(Seconds(1));
            });
        }

        void UpdateAI(uint32 diff) override
        {
            _scheduler.Update(diff);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoMeleeAttackIfReady();
        }

    private:
        TaskScheduler _scheduler;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetBlackTempleAI<npc_angered_soul_fragmentAI>(creature);
    }
};

// 41986 - Anger
class spell_soul_fragment_anger : public SpellScriptLoader
{
    public:
        spell_soul_fragment_anger() : SpellScriptLoader("spell_soul_fragment_anger") { }

        class spell_soul_fragment_anger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_soul_fragment_anger_SpellScript);

            void HandleKill()
            {
                if (Creature* caster = GetCaster()->ToCreature())
                    caster->DespawnOrUnsummon(Milliseconds(200));
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_soul_fragment_anger_SpellScript::HandleKill);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_soul_fragment_anger_SpellScript();
        }
};

// 39645 - Shadow Inferno
class spell_illidari_nightlord_shadow_inferno : public SpellScriptLoader
{
    public:
        spell_illidari_nightlord_shadow_inferno() : SpellScriptLoader("spell_illidari_nightlord_shadow_inferno") { }

        class spell_illidari_nightlord_shadow_inferno_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_illidari_nightlord_shadow_inferno_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_SHADOW_INFERNO_DAMAGE });
            }

            void OnPeriodic(AuraEffect const* aurEffect)
            {
                PreventDefaultAction();
                int32 bp = aurEffect->GetTickNumber() * aurEffect->GetAmount();
                GetUnitOwner()->CastCustomSpell(SPELL_SHADOW_INFERNO_DAMAGE, SPELLVALUE_BASE_POINT0, bp, GetUnitOwner(), true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_illidari_nightlord_shadow_inferno_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_illidari_nightlord_shadow_inferno_AuraScript();
        }
};

void AddSC_black_temple()
{
    new npc_wrathbone_flayer();
    new npc_angered_soul_fragment();
    new spell_soul_fragment_anger();
    new spell_illidari_nightlord_shadow_inferno();
}
