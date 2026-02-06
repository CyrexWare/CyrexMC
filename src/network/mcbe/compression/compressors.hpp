#pragma once

#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "snappy_compressor.hpp"
#include "zlib_compressor.hpp"

namespace cyrex::nw::protocol
{
inline Compressor* getCompressor(cyrex::nw::protocol::CompressionAlgorithm algo)
{
    if (algo == cyrex::nw::protocol::CompressionAlgorithm::ZLIB)
    {
        static ZlibCompressor zlibCompressor;
        return &zlibCompressor;
    }
    else if (algo == cyrex::nw::protocol::CompressionAlgorithm::SNAPPY)
    {
        static SnappyCompressor snappyCompressor;
        return &snappyCompressor;
    }

    return nullptr;
}
} // namespace cyrex::nw::protocol
