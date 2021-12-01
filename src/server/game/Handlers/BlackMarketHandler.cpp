/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#include "WorldSession.h"
#include "BlackMarketMgr.h"
#include "BlackMarketPackets.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "WorldPacket.h"

void WorldSession::HandleBlackMarketOpen(WorldPackets::BlackMarket::BlackMarketOpen& blackMarketOpen)
{
    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(blackMarketOpen.Guid, UNIT_NPC_FLAG_BLACK_MARKET | UNIT_NPC_FLAG_BLACK_MARKET_VIEW);
    if (!unit)
    {
        LOG_DEBUG("network", "WORLD: HandleBlackMarketHello - Unit (GUID: %s) not found or you can't interact with him.", blackMarketOpen.Guid.ToString().c_str());
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendBlackMarketOpenResult(blackMarketOpen.Guid, unit);
}

void WorldSession::SendBlackMarketOpenResult(ObjectGuid guid, Creature* /*auctioneer*/)
{
    WorldPackets::BlackMarket::BlackMarketOpenResult packet;
    packet.Guid = guid;
    packet.Enable = sBlackMarketMgr->IsEnabled();
    SendPacket(packet.Write());
}

void WorldSession::HandleBlackMarketRequestItems(WorldPackets::BlackMarket::BlackMarketRequestItems& blackMarketRequestItems)
{
    if (!sBlackMarketMgr->IsEnabled())
        return;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(blackMarketRequestItems.Guid, UNIT_NPC_FLAG_BLACK_MARKET | UNIT_NPC_FLAG_BLACK_MARKET_VIEW);
    if (!unit)
    {
        LOG_DEBUG("network", "WORLD: HandleBlackMarketRequestItems - Unit (GUID: %s) not found or you can't interact with him.", blackMarketRequestItems.Guid.ToString().c_str());
        return;
    }

    WorldPackets::BlackMarket::BlackMarketRequestItemsResult result;
    sBlackMarketMgr->BuildItemsResponse(result, GetPlayer());
    SendPacket(result.Write());
}

void WorldSession::HandleBlackMarketBidOnItem(WorldPackets::BlackMarket::BlackMarketBidOnItem& blackMarketBidOnItem)
{
    if (!sBlackMarketMgr->IsEnabled())
        return;

    Player* player = GetPlayer();
    Creature* unit = player->GetNPCIfCanInteractWith(blackMarketBidOnItem.Guid, UNIT_NPC_FLAG_BLACK_MARKET);
    if (!unit)
    {
        LOG_DEBUG("network", "WORLD: HandleBlackMarketBidOnItem - Unit (GUID: %s) not found or you can't interact with him.", blackMarketBidOnItem.Guid.ToString().c_str());
        return;
    }

    BlackMarketEntry* entry = sBlackMarketMgr->GetAuctionByID(blackMarketBidOnItem.MarketID);
    if (!entry)
    {
        LOG_DEBUG("network", "WORLD: HandleBlackMarketBidOnItem - Player (%s, name: %s) tried to bid on a nonexistent auction (MarketId: %i).", player->GetGUID().ToString().c_str(), player->GetName().c_str(), blackMarketBidOnItem.MarketID);
        SendBlackMarketBidOnItemResult(ERR_BMAH_ITEM_NOT_FOUND, blackMarketBidOnItem.MarketID, blackMarketBidOnItem.Item);
        return;
    }

    if (entry->GetBidder() == player->GetGUID().GetCounter())
    {
        LOG_DEBUG("network", "WORLD: HandleBlackMarketBidOnItem - Player (%s, name: %s) tried to place a bid on an item he already bid on. (MarketId: %i).", player->GetGUID().ToString().c_str(), player->GetName().c_str(), blackMarketBidOnItem.MarketID);
        SendBlackMarketBidOnItemResult(ERR_BMAH_ALREADY_BID, blackMarketBidOnItem.MarketID, blackMarketBidOnItem.Item);
        return;
    }

    if (!entry->ValidateBid(blackMarketBidOnItem.BidAmount))
    {
        LOG_DEBUG("network", "WORLD: HandleBlackMarketBidOnItem - Player (%s, name: %s) tried to place an invalid bid. Amount: %lu (MarketId: %i).", player->GetGUID().ToString().c_str(), player->GetName().c_str(), blackMarketBidOnItem.BidAmount, blackMarketBidOnItem.MarketID);
        SendBlackMarketBidOnItemResult(ERR_BMAH_HIGHER_BID, blackMarketBidOnItem.MarketID, blackMarketBidOnItem.Item);
        return;
    }

    if (!player->HasEnoughMoney(blackMarketBidOnItem.BidAmount))
    {
        LOG_DEBUG("network", "WORLD: HandleBlackMarketBidOnItem - Player (%s, name: %s) does not have enough money to place bid. (MarketId: %i).", player->GetGUID().ToString().c_str(), player->GetName().c_str(), blackMarketBidOnItem.MarketID);
        SendBlackMarketBidOnItemResult(ERR_BMAH_NOT_ENOUGH_MONEY, blackMarketBidOnItem.MarketID, blackMarketBidOnItem.Item);
        return;
    }

    if (entry->GetSecondsRemaining() <= 0)
    {
        LOG_DEBUG("network", "WORLD: HandleBlackMarketBidOnItem - Player (%s, name: %s) tried to bid on a completed auction. (MarketId: %i).", player->GetGUID().ToString().c_str(), player->GetName().c_str(), blackMarketBidOnItem.MarketID);
        SendBlackMarketBidOnItemResult(ERR_BMAH_DATABASE_ERROR, blackMarketBidOnItem.MarketID, blackMarketBidOnItem.Item);
        return;
    }

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    sBlackMarketMgr->SendAuctionOutbidMail(entry, trans);
    entry->PlaceBid(blackMarketBidOnItem.BidAmount, player, trans);

    CharacterDatabase.CommitTransaction(trans);

    SendBlackMarketBidOnItemResult(ERR_BMAH_OK, blackMarketBidOnItem.MarketID, blackMarketBidOnItem.Item);
}

void WorldSession::SendBlackMarketBidOnItemResult(int32 result, int32 marketId, WorldPackets::Item::ItemInstance& item)
{
    WorldPackets::BlackMarket::BlackMarketBidOnItemResult packet;

    packet.MarketID = marketId;
    packet.Item = item;
    packet.Result = result;

    SendPacket(packet.Write());
}

void WorldSession::SendBlackMarketWonNotification(BlackMarketEntry const* entry, Item const* item)
{
    WorldPackets::BlackMarket::BlackMarketWon packet;

    packet.MarketID = entry->GetMarketId();
    packet.Item.Initialize(item);
    packet.RandomPropertiesID = item->GetItemRandomPropertyId();

    SendPacket(packet.Write());
}

void WorldSession::SendBlackMarketOutbidNotification(BlackMarketTemplate const* templ)
{
    WorldPackets::BlackMarket::BlackMarketOutbid packet;

    packet.MarketID = templ->MarketID;
    packet.Item = templ->Item;
    packet.RandomPropertiesID = templ->Item.RandomPropertiesID;

    SendPacket(packet.Write());
}
