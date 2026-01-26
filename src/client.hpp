#pragma once

#include "RakNet/BitStream.h"
#include "raknet-utils.hpp"

struct Client
{
    bool hasSentNetworkSettings{};

    void handleNetworkSettingsRequest(RakNet::RakPeerInterface* peer, RakNet::Packet* packet)
    {
        RakNet::BitStream in(packet->data, packet->length, false);
        in.IgnoreBytes(1);

        std::uint32_t length = RakNetUtils::readVarInt32u(in);
        std::uint32_t id = RakNetUtils::readVarInt32u(in);

        // network request
        assert(id == 0xC1);

        RakNet::BitStream bs;

        bs.Write(std::uint8_t{0xFE});

        bs.Write(std::int8_t{12});
        bs.Write(std::uint8_t{0x8F});

        bs.Write(std::int8_t{1}); // 1
        bs.Write(std::int8_t{0}); // 1
        bs.Write(std::int16_t{00});
        bs.Write(false);
        bs.Write(std::int8_t{0});
        bs.Write(float{0});
        bs.Write(std::int8_t{0});

        peer->Send((const char*)bs.GetData(), bs.GetNumberOfBytesUsed(), LOW_PRIORITY, UNRELIABLE, 0, packet->systemAddress, false);
    }

    RakNet::BitStream decompress(RakNet::BitStream& stream, std::uint8_t compressionMethod)
    {
        if (compressionMethod == 0xFF) //uncompressed
        {
            return RakNet::BitStream(stream.GetData() + stream.GetReadOffset() / 8, stream.GetNumberOfUnreadBits() / 8,  false);
        }
        else if (compressionMethod == 0x00) // ZLib
        {

        }
    }

    void handleGamePacket(RakNet::RakPeerInterface* peer, RakNet::Packet* packet)
    {
        if (!hasSentNetworkSettings)
        {
            handleNetworkSettingsRequest(peer, packet);
            hasSentNetworkSettings = true;
            return;
        }

        RakNet::BitStream in(packet->data, packet->length, false);
        in.IgnoreBytes(1);

        std::uint8_t compressionMethod;
        in.Read(compressionMethod);

        std::uint32_t length = RakNetUtils::readVarInt32u(in); // 240279276
        std::uint32_t id = RakNetUtils::readVarInt32u(in);

        // network request
        if (id == 0xC1)
        {
            RakNet::BitStream bs;

            bs.Write(std::uint8_t{0xFE});

            bs.Write(std::int8_t{12});
            bs.Write(std::uint8_t{0x8F});

            bs.Write(std::int8_t{1}); // 1
            bs.Write(std::int8_t{1}); // 1
            bs.Write(std::int16_t{00});
            bs.Write(false);
            bs.Write(std::int8_t{0});
            bs.Write(float{0});
            bs.Write(std::int8_t{0});

            peer->Send((const char*)bs.GetData(), bs.GetNumberOfBytesUsed(), LOW_PRIORITY, UNRELIABLE, 0, packet->systemAddress, false);

            hasSentNetworkSettings = true;
        }
    }
};