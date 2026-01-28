#include "zlib_compressor.hpp"

#include <stdexcept>

namespace cyrex::network::mcbe::compression
{

ZlibCompressor::ZlibCompressor(int level, std::optional<size_t> minSize, size_t maxDecompressionSize) :
    m_level(level),
    m_minCompressionSize(minSize),
    m_maxDecompressionSize(maxDecompressionSize)
{
    m_compressor = libdeflate_alloc_compressor(level);
    m_decompressor = libdeflate_alloc_decompressor();

    if (!m_compressor || !m_decompressor)
        throw std::runtime_error("Failed to initialize libdeflate");
}

ZlibCompressor::~ZlibCompressor()
{
    libdeflate_free_compressor(m_compressor);
    libdeflate_free_decompressor(m_decompressor);
}

bool ZlibCompressor::compress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output)
{
    bool compressible = !m_minCompressionSize.has_value() || inputSize >= *m_minCompressionSize;

    if (!compressible)
    {
        output.assign(input, input + inputSize);
        return true;
    }

    size_t bound = libdeflate_deflate_compress_bound(m_compressor, inputSize);
    output.resize(bound);

    size_t written = libdeflate_deflate_compress(m_compressor, input, inputSize, output.data(), bound);

    if (written == 0)
        return false;

    output.resize(written);
    return true;
}

bool ZlibCompressor::decompress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output)
{
    output.resize(m_maxDecompressionSize);

    size_t actualSize = 0;
    auto res = libdeflate_deflate_decompress(m_decompressor, input, inputSize, output.data(), output.size(), &actualSize);

    if (res != LIBDEFLATE_SUCCESS)
        return false;

    output.resize(actualSize);
    return true;
}

cyrex::mcpe::protocol::types::CompressionAlgorithm ZlibCompressor::networkId() const noexcept
{
    return cyrex::mcpe::protocol::types::CompressionAlgorithm::ZLIB;
}

std::optional<size_t> ZlibCompressor::compressionThreshold() const noexcept
{
    return m_minCompressionSize;
}
} // namespace cyrex::network::mcbe::compression
