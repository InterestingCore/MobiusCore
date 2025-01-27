/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#ifndef GUNDRAK_H_
#define GUNDRAK_H_

#include "CreatureAIImpl.h"

#define GundrakScriptName "instance_gundrak"
#define DataHeader "GD"

uint32 const EncounterCount = 5;

enum GDDataTypes
{
    // Encounter Ids // Encounter States // Boss GUIDs
    DATA_SLAD_RAN                    = 0,
    DATA_DRAKKARI_COLOSSUS           = 1,
    DATA_MOORABI                     = 2,
    DATA_GAL_DARAH                   = 3,
    DATA_ECK_THE_FEROCIOUS           = 4,

    // Additional Objects
    DATA_SLAD_RAN_ALTAR              = 5,
    DATA_DRAKKARI_COLOSSUS_ALTAR     = 6,
    DATA_MOORABI_ALTAR               = 7,

    DATA_SLAD_RAN_STATUE             = 8,
    DATA_DRAKKARI_COLOSSUS_STATUE    = 9,
    DATA_MOORABI_STATUE              = 10,
    DATA_GAL_DARAH_STATUE            = 11,

    DATA_TRAPDOOR                    = 12,
    DATA_COLLISION                   = 13,
    DATA_BRIDGE                      = 14,

    DATA_STATUE_ACTIVATE             = 15,
};

enum GDCreatureIds
{
    NPC_SLAD_RAN                     = 29304,
    NPC_MOORABI                      = 29305,
    NPC_GAL_DARAH                    = 29306,
    NPC_DRAKKARI_COLOSSUS            = 29307,
    NPC_RUIN_DWELLER                 = 29920,
    NPC_ECK_THE_FEROCIOUS            = 29932,
    NPC_ALTAR_TRIGGER                = 30298,
    NPC_RHINO_SPIRIT                 = 29791
};

enum GDGameObjectIds
{
    GO_SLAD_RAN_ALTAR                = 192518,
    GO_MOORABI_ALTAR                 = 192519,
    GO_DRAKKARI_COLOSSUS_ALTAR       = 192520,
    GO_SLAD_RAN_STATUE               = 192564,
    GO_MOORABI_STATUE                = 192565,
    GO_GAL_DARAH_STATUE              = 192566,
    GO_DRAKKARI_COLOSSUS_STATUE      = 192567,
    GO_ECK_THE_FEROCIOUS_DOOR        = 192632,
    GO_ECK_THE_FEROCIOUS_DOOR_BEHIND = 192569,
    GO_GAL_DARAH_DOOR_1              = 193208,
    GO_GAL_DARAH_DOOR_2              = 193209,
    GO_GAL_DARAH_DOOR_3              = 192568,
    GO_TRAPDOOR                      = 193188,
    GO_COLLISION                     = 192633,
};

enum GDSpellIds
{
    SPELL_FIRE_BEAM_MAMMOTH          = 57068,
    SPELL_FIRE_BEAM_SNAKE            = 57071,
    SPELL_FIRE_BEAM_ELEMENTAL        = 57072
};

enum GDInstanceMisc
{
    TIMER_STATUE_ACTIVATION          = 3500
};

template<typename AI>
inline AI* GetGundrakAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, GundrakScriptName);
}

#endif // GUNDRAK_H_
