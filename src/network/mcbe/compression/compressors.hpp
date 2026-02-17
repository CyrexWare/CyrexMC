#pragma once

#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "snappy_compressor.hpp"
#include "zlib_compressor.hpp"

namespace cyrex::nw::protocol
{
inline Compressor* getCompressor(CompressionAlgorithm algo)
{
    if (algo == CompressionAlgorithm::ZLIB)
    {
        static ZlibCompressor zlibCompressor;
        return &zlibCompressor;
    }
    if (algo == CompressionAlgorithm::SNAPPY)
    {
        static SnappyCompressor snappyCompressor;
        return &snappyCompressor;
    }

    return nullptr;
}
} // namespace cyrex::nw::protocol
