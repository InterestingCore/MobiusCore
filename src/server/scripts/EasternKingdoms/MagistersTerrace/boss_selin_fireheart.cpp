/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "magisters_terrace.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"

enum Says
{
    SAY_AGGRO                       = 0,
    SAY_ENERGY                      = 1,
    SAY_EMPOWERED                   = 2,
    SAY_KILL                        = 3,
    SAY_DEATH                       = 4,
    EMOTE_CRYSTAL                   = 5
};

enum Spells
{
    // Crystal effect spells
    SPELL_FEL_CRYSTAL_DUMMY         = 44329,
    SPELL_MANA_RAGE                 = 44320,               // This spell triggers 44321, which changes scale and regens mana Requires an entry in spell_script_target

    // Selin's spells
    SPELL_DRAIN_LIFE                = 44294,
    SPELL_FEL_EXPLOSION             = 44314,

    SPELL_DRAIN_MANA                = 46153               // Heroic only
};

enum Phases
{
    PHASE_NORMAL                    = 1,
    PHASE_DRAIN                     = 2
};

enum Events
{
    EVENT_FEL_EXPLOSION             = 1,
    EVENT_DRAIN_CRYSTAL,
    EVENT_DRAIN_MANA,
    EVENT_DRAIN_LIFE,
    EVENT_EMPOWER
};

enum Misc
{
    ACTION_SWITCH_PHASE             = 1
};

class boss_selin_fireheart : public CreatureScript
{
    public:
        boss_selin_fireheart() : CreatureScript("boss_selin_fireheart") { }

        struct boss_selin_fireheartAI : public BossAI
        {
            boss_selin_fireheartAI(Creature* creature) : BossAI(creature, DATA_SELIN)
            {
                _scheduledEvents = false;
            }

            void Reset() override
            {
                Crystals.clear();
                me->GetCreatureListWithEntryInGrid(Crystals, NPC_FEL_CRYSTAL, 250.0f);

                for (Creature* creature : Crystals)
                {
                    if (!creature->IsAlive())
                        creature->Respawn();

                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                }

                _Reset();
                CrystalGUID.Clear();
                _scheduledEvents = false;
            }

            void DoAction(int32 action) override
            {
                switch (action)
                {
                    case ACTION_SWITCH_PHASE:
                        events.SetPhase(PHASE_NORMAL);
                        events.ScheduleEvent(EVENT_FEL_EXPLOSION, 2000, 0, PHASE_NORMAL);
                        AttackStart(me->GetVictim());
                        me->GetMotionMaster()->MoveChase(me->GetVictim());
                        break;
                    default:
                        break;
                }
            }

            void SelectNearestCrystal()
            {
                if (Crystals.empty())
                    return;

                Crystals.sort(Server::ObjectDistanceOrderPred(me));
                if (Creature* CrystalChosen = Crystals.front())
                {
                    Talk(SAY_ENERGY);
                    Talk(EMOTE_CRYSTAL);

                    DoCast(CrystalChosen, SPELL_FEL_CRYSTAL_DUMMY);
                    CrystalGUID = CrystalChosen->GetGUID();
                    Crystals.remove(CrystalChosen);

                    float x, y, z;
                    CrystalChosen->GetClosePoint(x, y, z, me->GetObjectSize(), CONTACT_DISTANCE);

                    events.SetPhase(PHASE_DRAIN);
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(1, x, y, z);
                }
            }

            void ShatterRemainingCrystals()
            {
                if (Crystals.empty())
                    return;

                for (Creature* crystal : Crystals)
                {
                    if (crystal && crystal->IsAlive())
                        crystal->KillSelf();
                }
            }

            void EnterCombat(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);
                _EnterCombat();

                events.SetPhase(PHASE_NORMAL);
                events.ScheduleEvent(EVENT_FEL_EXPLOSION, 2100, 0, PHASE_NORMAL);
             }

            void KilledUnit(Unit* victim) override
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type == POINT_MOTION_TYPE && id == 1)
                {
                    Unit* CrystalChosen = ObjectAccessor::GetUnit(*me, CrystalGUID);
                    if (CrystalChosen && CrystalChosen->IsAlive())
                    {
                        CrystalChosen->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        CrystalChosen->CastSpell(me, SPELL_MANA_RAGE, true);
                        events.ScheduleEvent(EVENT_EMPOWER, 10000, PHASE_DRAIN);
                    }
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH);
                _JustDied();

                ShatterRemainingCrystals();
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FEL_EXPLOSION:
                            DoCastAOE(SPELL_FEL_EXPLOSION);
                            events.ScheduleEvent(EVENT_FEL_EXPLOSION, 2000, 0, PHASE_NORMAL);
                            break;
                        case EVENT_DRAIN_CRYSTAL:
                            SelectNearestCrystal();
                            _scheduledEvents = false;
                            break;
                        case EVENT_DRAIN_MANA:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 45.0f, true))
                                DoCast(target, SPELL_DRAIN_MANA);
                            events.ScheduleEvent(EVENT_DRAIN_MANA, 10000, 0, PHASE_NORMAL);
                            break;
                        case EVENT_DRAIN_LIFE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 20.0f, true))
                                DoCast(target, SPELL_DRAIN_LIFE);
                            events.ScheduleEvent(EVENT_DRAIN_LIFE, 10000, 0, PHASE_NORMAL);
                            break;
                        case EVENT_EMPOWER:
                        {
                            Talk(SAY_EMPOWERED);

                            Creature* CrystalChosen = ObjectAccessor::GetCreature(*me, CrystalGUID);
                            if (CrystalChosen && CrystalChosen->IsAlive())
                                CrystalChosen->KillSelf();

                            CrystalGUID.Clear();

                            me->GetMotionMaster()->Clear();
                            me->GetMotionMaster()->MoveChase(me->GetVictim());
                            break;
                        }
                        default:
                            break;
                    }

                    if (me->HasUnitState(UNIT_STATE_CASTING))
                        return;
                }

                if (me->GetPowerPct(POWER_MANA) < 10.f)
                {
                    if (events.IsInPhase(PHASE_NORMAL) && !_scheduledEvents)
                    {
                        _scheduledEvents = true;
                        uint32 timer = urand(3000, 7000);
                        events.ScheduleEvent(EVENT_DRAIN_LIFE, timer, 0, PHASE_NORMAL);

                        if (IsHeroic())
                        {
                            events.ScheduleEvent(EVENT_DRAIN_CRYSTAL, urand(10000, 15000), 0, PHASE_NORMAL);
                            events.ScheduleEvent(EVENT_DRAIN_MANA, timer + 5000, 0, PHASE_NORMAL);
                        }
                        else
                            events.ScheduleEvent(EVENT_DRAIN_CRYSTAL, urand(20000, 25000), 0, PHASE_NORMAL);
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            std::list<Creature*> Crystals;
            ObjectGuid CrystalGUID;
            bool _scheduledEvents;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetMagistersTerraceAI<boss_selin_fireheartAI>(creature);
        };
};

class npc_fel_crystal : public CreatureScript
{
    public:
        npc_fel_crystal() : CreatureScript("npc_fel_crystal") { }

        struct npc_fel_crystalAI : public ScriptedAI
        {
            npc_fel_crystalAI(Creature* creature) : ScriptedAI(creature) { }

            void JustDied(Unit* /*killer*/) override
            {
                if (InstanceScript* instance = me->GetInstanceScript())
                {
                    Creature* Selin = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_SELIN));
                    if (Selin && Selin->IsAlive())
                        Selin->AI()->DoAction(ACTION_SWITCH_PHASE);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetMagistersTerraceAI<npc_fel_crystalAI>(creature);
        };
};

void AddSC_boss_selin_fireheart()
{
    new boss_selin_fireheart();
    new npc_fel_crystal();
}
