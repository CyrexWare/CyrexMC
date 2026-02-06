#pragma once

#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "snappy_compressor.hpp"
#include "zlib_compressor.hpp"

namespace cyrex::network::mcbe::compression
{
inline Compressor* getCompressor(mcpe::protocol::types::CompressionAlgorithm algo)
{
    if (algo == mcpe::protocol::types::CompressionAlgorithm::ZLIB)
    {
        static ZlibCompressor zlibCompressor;
        return &zlibCompressor;
    }
    if (algo == mcpe::protocol::types::CompressionAlgorithm::SNAPPY)
    {
        static SnappyCompressor snappyCompressor;
        return &snappyCompressor;
    }

    return nullptr;
}
} // namespace cyrex::network::mcbe::compression
