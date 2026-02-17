#pragma once
#include <cstdint>

namespace cyrex::nw::proto
{

enum class PacketDirection : uint8_t
{
    Clientbound = 1,
    Serverbound = 2,
    Bidirectional = 3
};

} // namespace cyrex::nw::proto
