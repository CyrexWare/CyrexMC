#include "zlib_compressor.hpp"

#include <stdexcept>

namespace cyrex::nw::proto
{

ZlibCompressor::ZlibCompressor(const int level, const size_t maxDecompressionSize) :
    m_maxDecompressionSize(maxDecompressionSize),
    m_compressor{libdeflate_alloc_compressor(level)},
    m_decompressor{libdeflate_alloc_decompressor()}
{
    if (!m_compressor || !m_decompressor)
    {
        throw std::runtime_error("Failed to initialize libdeflate");
    }
}

std::optional<std::vector<uint8_t>> ZlibCompressor::compress(const std::span<const uint8_t> input) const
{
    const size_t bound = libdeflate_deflate_compress_bound(m_compressor.get(), input.size());
    std::vector<uint8_t> output(bound);

    const size_t written = libdeflate_deflate_compress(m_compressor.get(), input.data(), input.size(), output.data(), bound);

    if (written == 0)
    {
        return std::nullopt;
    }

    output.resize(written);
    return output;
}

std::optional<std::vector<uint8_t>> ZlibCompressor::decompress(const std::span<const uint8_t> input) const
{
    std::vector<uint8_t> output(m_maxDecompressionSize);

    size_t actualSize = 0;
    auto res = libdeflate_deflate_decompress(m_decompressor.get(), input.data(), input.size(), output.data(), output.size(), &actualSize);

    if (res != LIBDEFLATE_SUCCESS)
    {
        return std::nullopt;
    }

    output.resize(actualSize);
    return output;
}
} // namespace cyrex::nw::proto
