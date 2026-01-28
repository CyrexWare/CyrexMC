#pragma once

#include <cstdint>

namespace cyrex::mcpe::protocol::types
{
enum class CompressionAlgorithm : std::uint8_t
{
    ZLIB = 0,
    SNAPPY = 1,
    NONE = 0xFF
};
} // namespace cyrex::mcpe::protocol::types
