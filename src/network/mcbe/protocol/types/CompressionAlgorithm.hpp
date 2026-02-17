#pragma once

#include <cstdint>

namespace cyrex::nw::protocol
{
enum class CompressionAlgorithm : std::uint8_t
{
    ZLIB = 0,
    SNAPPY = 1,
    NONE = 0xFF
};
} // namespace cyrex::nw::protocol
