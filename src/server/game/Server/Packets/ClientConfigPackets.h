/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#ifndef ClientConfigPackets_h__
#define ClientConfigPackets_h__

#include "Packet.h"
#include "WorldSession.h"

namespace WorldPackets
{
    namespace ClientConfig
    {
        class AccountDataTimes final : public ServerPacket
        {
        public:
            AccountDataTimes() : ServerPacket(SMSG_ACCOUNT_DATA_TIMES, 4 + 4 * NUM_ACCOUNT_DATA_TYPES) { }

            WorldPacket const* Write() override;

            ObjectGuid PlayerGuid;
            uint32 ServerTime = 0;
            uint32 AccountTimes[NUM_ACCOUNT_DATA_TYPES];
        };

        class ClientCacheVersion final : public ServerPacket
        {
        public:
            ClientCacheVersion() : ServerPacket(SMSG_CACHE_VERSION, 4) { }

            WorldPacket const* Write() override;

            uint32 CacheVersion = 0;
        };

        class RequestAccountData final : public ClientPacket
        {
        public:
            RequestAccountData(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_ACCOUNT_DATA, std::move(packet)) { }

            void Read() override;

            ObjectGuid PlayerGuid;
            uint8 DataType = 0; ///< @see enum AccountDataType
        };

        class UpdateAccountData final : public ServerPacket
        {
        public:
            UpdateAccountData() : ServerPacket(SMSG_UPDATE_ACCOUNT_DATA) { }

            WorldPacket const* Write() override;

            ObjectGuid Player;
            uint32 Time    = 0; ///< UnixTime
            uint32 Size    = 0; ///< decompressed size
            uint8 DataType = 0; ///< @see enum AccountDataType
            ByteBuffer CompressedData;
        };

        class UserClientUpdateAccountData final : public ClientPacket
        {
        public:
            UserClientUpdateAccountData(WorldPacket&& packet) : ClientPacket(CMSG_UPDATE_ACCOUNT_DATA, std::move(packet)) { }

            void Read() override;

            ObjectGuid PlayerGuid;
            uint32 Time    = 0; ///< UnixTime
            uint32 Size    = 0; ///< decompressed size
            uint8 DataType = 0; ///< @see enum AccountDataType
            ByteBuffer CompressedData;
        };

        class SetAdvancedCombatLogging final : public ClientPacket
        {
        public:
            SetAdvancedCombatLogging(WorldPacket&& packet) : ClientPacket(CMSG_SET_ADVANCED_COMBAT_LOGGING, std::move(packet)) { }

            void Read() override;

            bool Enable = false;
        };
    }
}

#endif // ClientConfigPackets_h__
