#pragma once

#include <cstdint>

namespace cyrex::network::protocol
{
enum class CompressionAlgorithm : std::uint8_t
{
    ZLIB = 0,
    SNAPPY = 1,
    NONE = 0xFF
};
} // namespace cyrex::network::protocol
