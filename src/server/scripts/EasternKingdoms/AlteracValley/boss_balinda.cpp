/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#include "ScriptMgr.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"

enum Spells
{
    SPELL_ARCANE_EXPLOSION                  = 46608,
    SPELL_CONE_OF_COLD                      = 38384,
    SPELL_FIREBALL                          = 46988,
    SPELL_FROSTBOLT                         = 46987,
    SPELL_SUMMON_WATER_ELEMENTAL            = 45067,
    SPELL_ICEBLOCK                          = 46604
};

enum Texts
{
    SAY_AGGRO                              = 0,
    SAY_EVADE                              = 1,
    SAY_SALVATION                          = 2,
};

enum Action
{
    ACTION_BUFF_YELL                        = -30001 // shared from Battleground
};

enum Events
{
    // Balinda
    EVENT_ARCANE_EXPLOSION = 1,
    EVENT_CONE_OF_COLD,
    EVENT_FIREBOLT,
    EVENT_FROSTBOLT,
    EVENT_SUMMON_WATER_ELEMENTAL,
    EVENT_CHECK_RESET,          // Checks if Balinda or the Water Elemental are outside of building.
};

class boss_balinda : public CreatureScript
{
public:
    boss_balinda() : CreatureScript("boss_balinda") { }

    struct boss_balindaAI : public ScriptedAI
    {
        boss_balindaAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            Initialize();
        }

        void Initialize()
        {
            WaterElementalGUID.Clear();
            HasCastIceblock = false;
        }

        void Reset() override
        {
            Initialize();
            events.Reset();
            summons.DespawnAll();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_ARCANE_EXPLOSION, urand(5 * IN_MILLISECONDS, 15 * IN_MILLISECONDS));
            events.ScheduleEvent(EVENT_CONE_OF_COLD, 8 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_FIREBOLT, 1 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_FROSTBOLT, 4 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SUMMON_WATER_ELEMENTAL, 3 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_CHECK_RESET, 5 * IN_MILLISECONDS);
        }

        void JustSummoned(Creature* summoned) override
        {
            summoned->AI()->AttackStart(SelectTarget(SELECT_TARGET_RANDOM, 0, 50, true));
            summoned->setFaction(me->getFaction());
            WaterElementalGUID = summoned->GetGUID();
            summons.Summon(summoned);
        }

        void SummonedCreatureDespawn(Creature* summoned) override
        {
            summons.Despawn(summoned);
        }

        void JustDied(Unit* /*killer*/) override
        {
            summons.DespawnAll();
        }

        void DoAction(int32 actionId) override
        {
            if (actionId == ACTION_BUFF_YELL)
                Talk(SAY_AGGRO);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage) override
        {
            if (me->HealthBelowPctDamaged(40, damage) && !HasCastIceblock)
            {
                DoCast(SPELL_ICEBLOCK);
                HasCastIceblock = true;
            }
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
                    case EVENT_ARCANE_EXPLOSION:
                        DoCastVictim(SPELL_ARCANE_EXPLOSION);
                        events.ScheduleEvent(EVENT_ARCANE_EXPLOSION, urand(5 * IN_MILLISECONDS, 15 * IN_MILLISECONDS));
                        break;
                    case EVENT_CONE_OF_COLD:
                        DoCastVictim(SPELL_CONE_OF_COLD);
                        events.ScheduleEvent(EVENT_CONE_OF_COLD, urand(10 * IN_MILLISECONDS, 20 * IN_MILLISECONDS));
                        break;
                    case EVENT_FIREBOLT:
                        DoCastVictim(SPELL_FIREBALL);
                        events.ScheduleEvent(EVENT_FIREBOLT, urand(5 * IN_MILLISECONDS, 9 * IN_MILLISECONDS));
                        break;
                    case EVENT_FROSTBOLT:
                        DoCastVictim(SPELL_FROSTBOLT);
                        events.ScheduleEvent(EVENT_FROSTBOLT, urand(4 * IN_MILLISECONDS, 12 * IN_MILLISECONDS));
                        break;
                    case EVENT_SUMMON_WATER_ELEMENTAL:
                        if (summons.empty())
                            DoCast(SPELL_SUMMON_WATER_ELEMENTAL);
                        events.ScheduleEvent(EVENT_SUMMON_WATER_ELEMENTAL, 50 * IN_MILLISECONDS);
                        break;
                    case EVENT_CHECK_RESET:
                        if (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 50)
                        {
                            EnterEvadeMode();
                            Talk(SAY_EVADE);
                        }
                        if (Creature* elemental = ObjectAccessor::GetCreature(*me, WaterElementalGUID))
                            if (elemental->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 50)
                                elemental->AI()->EnterEvadeMode();
                        events.ScheduleEvent(EVENT_CHECK_RESET, 5 * IN_MILLISECONDS);
                        break;
                    default:
                        break;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        SummonList summons;
        ObjectGuid WaterElementalGUID;
        bool HasCastIceblock;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_balindaAI(creature);
    }
};

void AddSC_boss_balinda()
{
    new boss_balinda;
}
