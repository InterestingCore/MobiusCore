/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

/*
This placeholder for the instance is needed for dungeon finding to be able
to give credit after the boss defined in lastEncounterDungeon is killed.
Without it, the party doing random dungeon won't get satchel of spoils and
gets instead the deserter debuff.
*/

#include "ScriptMgr.h"
#include "InstanceScript.h"

// Bosses (East)
// 0 - Pusillin
// 1 - Lethtendris
// 2 - Hydrospawn
// 3 - Zevrim Thornhoof
// 4 - Alzzin the Wildshaper

// West
// 5 - Tendris Warpwood
// 6 - Magister Kalendris
// 7 - Tsu'zee
// 8 - Illyanna Ravenoak
// 9 - Immol'thar
// 10 - Prince Tortheldrin

// North
// 11 - Guard Mol'dar
// 12 - Stomper Kreeg
// 13 - Guard Fengus
// 14 - Guard Slip'kik
// 15 - Captain Kromcrush
// 16 - King Gordok

uint8 const EncounterCount = 17;

class instance_dire_maul : public InstanceMapScript
{
public:
    instance_dire_maul() : InstanceMapScript("instance_dire_maul", 429) { }

    struct instance_dire_maul_InstanceMapScript : public InstanceScript
    {
        instance_dire_maul_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetBossNumber(EncounterCount);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_dire_maul_InstanceMapScript(map);
    }

};

void AddSC_instance_dire_maul()
{
    new instance_dire_maul();
}
