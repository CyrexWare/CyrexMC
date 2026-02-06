#pragma once

#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "snappy_compressor.hpp"
#include "zlib_compressor.hpp"

namespace cyrex::network::mcbe::compression
{
inline Compressor* getCompressor(cyrex::mcpe::protocol::types::CompressionAlgorithm algo)
{
    if (algo == cyrex::mcpe::protocol::types::CompressionAlgorithm::ZLIB)
    {
        static ZlibCompressor zlibCompressor;
        return &zlibCompressor;
    }
    else if (algo == cyrex::mcpe::protocol::types::CompressionAlgorithm::SNAPPY)
    {
        static SnappyCompressor snappyCompressor;
        return &snappyCompressor;
    }

    return nullptr;
}
} // namespace cyrex::network::mcbe::compression
