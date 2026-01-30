#pragma once

#include <vector>

#include <cstddef>
#include <cstdint>

namespace cyrex::network::mcbe
{

struct McbeBatchPacket
{
    std::vector<uint8_t> data;
};

class McbeBatchDecoder
{
public:
    static bool decode(const uint8_t* data, size_t len, std::vector<McbeBatchPacket>& outPackets);
};
} // namespace cyrex::network::mcbe
