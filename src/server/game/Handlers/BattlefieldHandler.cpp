/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Object.h"

#include "Battlefield.h"
#include "BattlefieldMgr.h"
#include "BattlefieldPackets.h"

/**
 * @fn void WorldSession::SendBfInvitePlayerToWar(uint64 queueId, uint32 zoneId, uint32 acceptTime)
 *
 * @brief This send to player windows for invite player to join the war.
 *
 * @param queueId       The queue id of Bf
 * @param zoneId        The zone where the battle is (4197 for wg)
 * @param acceptTime    Time in second that the player have for accept
 */
void WorldSession::SendBfInvitePlayerToWar(uint64 queueId, uint32 zoneId, uint32 acceptTime)
{
    WorldPackets::Battlefield::BFMgrEntryInvite bfMgrEntryInvite;
    bfMgrEntryInvite.QueueID = queueId;
    bfMgrEntryInvite.AreaID = zoneId;
    bfMgrEntryInvite.ExpireTime = time(nullptr) + acceptTime;
    SendPacket(bfMgrEntryInvite.Write());
}

/**
 * @fn void WorldSession::SendBfInvitePlayerToQueue(uint64 queueId, int8 battleState)
 *
 * @brief This send invitation to player to join the queue.
 *
 * @param queueId       The queue id of Bf
 */
void WorldSession::SendBfInvitePlayerToQueue(uint64 queueId, int8 battleState)
{
    WorldPackets::Battlefield::BFMgrQueueInvite bfMgrQueueInvite;
    bfMgrQueueInvite.QueueID = queueId;
    bfMgrQueueInvite.BattleState = battleState;
    SendPacket(bfMgrQueueInvite.Write());
}

/**
 * @fn void WorldSession::SendBfQueueInviteResponse(uint64 queueId, uint32 zoneId, int8 battleStatus, bool canQueue, bool loggingIn)
 *
 * @brief This send packet for inform player that he join queue.
 *
 * @param queueId       The queue id of Bf
 * @param zoneId        The zone where the battle is (4197 for wg)
 * @param battleStatus  Battlefield status
 * @param canQueue      if able to queue
 * @param loggingIn     on log in send queue status
 */
void WorldSession::SendBfQueueInviteResponse(uint64 queueId, uint32 zoneId, int8 battleStatus, bool canQueue /*= true*/, bool loggingIn /*= false*/)
{
    WorldPackets::Battlefield::BFMgrQueueRequestResponse bfMgrQueueRequestResponse;
    bfMgrQueueRequestResponse.QueueID = queueId;
    bfMgrQueueRequestResponse.AreaID = zoneId;
    bfMgrQueueRequestResponse.Result = canQueue ? 1 : 0;
    bfMgrQueueRequestResponse.BattleState = battleStatus;
    bfMgrQueueRequestResponse.LoggingIn = loggingIn;
    SendPacket(bfMgrQueueRequestResponse.Write());
}

/**
 * @fn void WorldSession::SendBfEntered(uint64 queueId, bool relocated, bool onOffense)
 *
 * @brief This is call when player accept to join war.
 *
 * @param queueId       The queue id of Bf
 * @param relocated     Whether player is added to Bf on the spot or teleported from queue
 * @param onOffense     Whether player belongs to attacking team or not
 */
void WorldSession::SendBfEntered(uint64 queueId, bool relocated, bool onOffense)
{
    WorldPackets::Battlefield::BFMgrEntering bfMgrEntering;
    bfMgrEntering.ClearedAFK = _player->isAFK();
    bfMgrEntering.Relocated = relocated;
    bfMgrEntering.OnOffense = onOffense;
    bfMgrEntering.QueueID = queueId;
    SendPacket(bfMgrEntering.Write());
}

/**
 * @fn void WorldSession::SendBfLeaveMessage(uint64 queueId, int8 battleState, bool relocated, BFLeaveReason reason)
 *
 * @brief This is call when player leave battlefield zone.
 *
 * @param queueId       The queue id of Bf
 * @param battleState   Battlefield status
 * @param relocated     Whether player is added to Bf on the spot or teleported from queue
 * @param reason        Reason why player left battlefield
 */
void WorldSession::SendBfLeaveMessage(uint64 queueId, int8 battleState, bool relocated, BFLeaveReason reason /*= BF_LEAVE_REASON_EXITED*/)
{
    WorldPackets::Battlefield::BFMgrEjected bfMgrEjected;
    bfMgrEjected.QueueID = queueId;
    bfMgrEjected.Reason = reason;
    bfMgrEjected.BattleState = battleState;
    bfMgrEjected.Relocated = relocated;
    SendPacket(bfMgrEjected.Write());
}

/**
 * @fn void WorldSession::HandleBfEntryInviteResponse(WorldPackets::Battlefield::BFMgrEntryInviteResponse& bfMgrEntryInviteResponse)
 *
 * @brief Send by client on clicking in accept or refuse of invitation windows for join game.
 */
void WorldSession::HandleBfEntryInviteResponse(WorldPackets::Battlefield::BFMgrEntryInviteResponse& bfMgrEntryInviteResponse)
{
    Battlefield* bf = sBattlefieldMgr->GetBattlefieldByQueueId(bfMgrEntryInviteResponse.QueueID);
    if (!bf)
        return;

    // If player accept invitation
    if (bfMgrEntryInviteResponse.AcceptedInvite)
    {
        bf->PlayerAcceptInviteToWar(_player);
    }
    else
    {
        if (_player->GetZoneId() == bf->GetZoneId())
            bf->KickPlayerFromBattlefield(_player->GetGUID());
    }
}

/**
 * @fn void WorldSession::HandleBfQueueInviteResponse(WorldPackets::Battlefield::BFMgrQueueInviteResponse& bfMgrQueueInviteResponse)
 *
 * @brief Send by client when he click on accept for queue.
 */
void WorldSession::HandleBfQueueInviteResponse(WorldPackets::Battlefield::BFMgrQueueInviteResponse& bfMgrQueueInviteResponse)
{
    Battlefield* bf = sBattlefieldMgr->GetBattlefieldByQueueId(bfMgrQueueInviteResponse.QueueID);
    if (!bf)
        return;

    if (bfMgrQueueInviteResponse.AcceptedInvite)
        bf->PlayerAcceptInviteToQueue(_player);
}

/**
 * @fn void WorldSession::HandleBfExitRequest(WorldPackets::Battlefield::BFMgrQueueExitRequest& bfMgrQueueExitRequest)
 *
 * @brief Send by client when exited battlefield
 */
void WorldSession::HandleBfQueueExitRequest(WorldPackets::Battlefield::BFMgrQueueExitRequest& bfMgrQueueExitRequest)
{
    Battlefield* bf = sBattlefieldMgr->GetBattlefieldByQueueId(bfMgrQueueExitRequest.QueueID);
    if (!bf)
        return;

    bf->AskToLeaveQueue(_player);
}
