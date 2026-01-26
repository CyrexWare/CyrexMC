#pragma once

#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"

#include <cstdint>
#include <print>
#include <functional>

template <>
struct std::hash<RakNet::RakNetGUID>
{
    std::size_t operator()(const RakNet::RakNetGUID& guid) const
    {
        return std::hash<uint64_t>{}(guid.g);
    }
};

namespace RakNetUtils
{

inline void writeVarInt32u(RakNet::BitStream& bs, std::uint32_t value)
{
    while (value > 0x7F) // While more than 7 bits remain
    {
        // Write the lower 7 bits with the continuation bit set (0x80)
        bs.Write(std::uint8_t(value & 0x7F | 0x80));
        value >>= 7; // Shift right by 7 bits
    }
    // Write the last byte with the continuation bit unset
    bs.Write(std::uint8_t(value));
}

inline std::uint32_t readVarInt32u(RakNet::BitStream& bs)
{
    std::uint32_t value{};
    std::uint32_t shift{};
    std::uint8_t b;
    do
    {
        bs.Read<std::uint8_t>(b);

        // Take the lower 7 bits and shift them into position
        value |= (b & 0x7F) << shift;
        shift += 7;
    } while ((b & 0x80) != 0); // Check the continuation bit (MSB)

    return value;
}

inline void dumpPacketHex(RakNet::Packet* packet)
{
    std::print("Packet from {}:{}, length={}\n",
               packet->systemAddress.ToString(false),
               packet->systemAddress.GetPort(),
               packet->length);

    for (unsigned int i = 0; i < packet->length; i++)
    {
        std::print("0x{:02X} ", static_cast<unsigned char>(packet->data[i]));
        if ((i + 1) % 16 == 0)
            std::print("\n");
    }
    std::print("\n");
}

} // namespace RakNetUtils