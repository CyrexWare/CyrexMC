#pragma once

#include <bit>

#include "RakNet/BitStream.h"
#include "raknet-utils.hpp"

#include <libdeflate.h>

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
        bs.Write(std::int8_t{1}); // 1
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
            return RakNet::BitStream(stream.GetData() + stream.GetReadOffset() / 8, stream.GetNumberOfUnreadBits() / 8, false);
        }
        else if (compressionMethod == 0x00) // ZLib
        {
            std::vector<std::uint8_t> buff(1000000);

            auto decompressor = libdeflate_alloc_decompressor();

            std::size_t count;
            libdeflate_deflate_decompress(decompressor,
                                          stream.GetData() + stream.GetReadOffset() / 8,
                                          stream.GetNumberOfUnreadBits() / 8,
                                          buff.data(),
                                          buff.size(),
                                          &count);

            return RakNet::BitStream(buff.data(), buff.size(), true);
        }

        assert(false);
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

        auto data = decompress(in, compressionMethod);

        std::uint32_t length = RakNetUtils::readVarInt32u(data);
        std::uint32_t id = RakNetUtils::readVarInt32u(data);

        if (id == 0x01)
        {
            std::string view;
            view.resize(200);
            data.Read(view.data(), view.size());
            data.SetReadOffset(data.GetReadOffset() - view.size() * 8);

            std::uint32_t protocol;
            data.Read(protocol);

            std::uint32_t blockLength = RakNetUtils::readVarInt32u(data);


            std::uint32_t strLength;
            data.Read(strLength);
            strLength = std::byteswap(strLength);

            std::string str;
            str.resize(strLength);
            data.Read(str.data(), strLength);

            std::println("JSON DATA\n{}", str);


            std::uint32_t strLength2;
            data.Read(strLength2);
            strLength2 = std::byteswap(strLength2);

            std::string str2;
            str2.resize(strLength2);
            data.Read(str2.data(), strLength2);

            std::println("jswt DATA\n{}", str2);
        }
    }
};