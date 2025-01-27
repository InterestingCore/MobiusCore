/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#ifndef OCULUS_H_
#define OCULUS_H_

#include "CreatureAIImpl.h"

#define OculusScriptName "instance_oculus"
#define DataHeader "OC"

uint32 const EncounterCount = 4;

enum OCDataTypes
{
    // Encounter States/Boss GUIDs
    DATA_DRAKOS                 = 0,
    DATA_VAROS                  = 1,
    DATA_UROM                   = 2,
    DATA_EREGOS                 = 3,
    // GPS System
    DATA_CONSTRUCTS             = 4
};

enum OCCreatureIds
{
    NPC_DRAKOS                  = 27654,
    NPC_VAROS                   = 27447,
    NPC_UROM                    = 27655,
    NPC_EREGOS                  = 27656,

    NPC_AZURE_RING_GUARDIAN     = 28236,
    NPC_CENTRIFUGE_CONSTRUCT    = 27641,
    NPC_RUBY_DRAKE_VEHICLE      = 27756,
    NPC_EMERALD_DRAKE_VEHICLE   = 27692,
    NPC_AMBER_DRAKE_VEHICLE     = 27755,
    NPC_VERDISA                 = 27657,
    NPC_BELGARISTRASZ           = 27658,
    NPC_ETERNOS                 = 27659,
    NPC_GREATER_WHELP           = 28276
};

enum OCGameObjectIds
{
    GO_DRAGON_CAGE_DOOR         = 193995,
    GO_EREGOS_CACHE_N           = 191349,
    GO_EREGOS_CACHE_H           = 193603
};

enum OCSpellEvents
{
    EVENT_CALL_DRAGON           = 12229
};

enum OCCreatureActions
{
    ACTION_CALL_DRAGON_EVENT    = 1
};

enum OCWorldStates
{
    WORLD_STATE_CENTRIFUGE_CONSTRUCT_SHOW   = 3524,
    WORLD_STATE_CENTRIFUGE_CONSTRUCT_AMOUNT = 3486
};

enum OCSpells
{
    SPELL_CENTRIFUGE_SHIELD     = 50053,
    SPELL_DEATH_SPELL           = 50415
};

enum OCInstanceTexts
{
    SAY_EREGOS_INTRO_TEXT = 0,
    SAY_VAROS_INTRO_TEXT  = 4
};

enum OCInstanceEvents
{
    EVENT_VAROS_INTRO = 1,
    EVENT_EREGOS_INTRO
};

enum OCConstructKillState
{
    KILL_NO_CONSTRUCT           = 0,
    KILL_ONE_CONSTRUCT          = 1,
    KILL_MORE_CONSTRUCT         = 2
};

enum OCMisc
{
    POINT_MOVE_OUT              = 1
};

template<typename AI>
inline AI* GetOculusAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, OculusScriptName);
}

#endif // OCULUS_H_
