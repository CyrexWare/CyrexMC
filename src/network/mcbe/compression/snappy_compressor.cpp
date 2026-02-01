#include "snappy_compressor.hpp"

namespace cyrex::network::mcbe::compression
{

SnappyCompressor::SnappyCompressor(std::optional<size_t> minSize, size_t maxDecompressionSize) :
    m_minCompressionSize(minSize),
    m_maxDecompressionSize(maxDecompressionSize)
{
}

CompressionStatus SnappyCompressor::compress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output)
{
    const bool compressible = !m_minCompressionSize.has_value() || inputSize >= *m_minCompressionSize;

    if (!compressible)
    {
        output.assign(input, input + inputSize);
        return CompressionStatus::RAW;
    }
    size_t written = snappy::MaxCompressedLength(inputSize);
    snappy::RawCompress(reinterpret_cast<const char*>(input), inputSize, reinterpret_cast<char*>(output.data()), &written);
    output.resize(written);
    return CompressionStatus::SUCCESS;
}

CompressionStatus SnappyCompressor::decompress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output)
{
    output.resize(m_maxDecompressionSize);
    if (!snappy::IsValidCompressedBuffer(reinterpret_cast<const char*>(input), inputSize))
    {
        return CompressionStatus::FAILED;
    }
    size_t actualSize = 0;
    if (!snappy::GetUncompressedLength(reinterpret_cast<const char*>(input), inputSize, &actualSize))
    {
        return CompressionStatus::FAILED;
    }
    if (!snappy::RawUncompress(reinterpret_cast<const char*>(input), inputSize, reinterpret_cast<char*>(output.data())))
    {
        return CompressionStatus::FAILED;
    }
    output.resize(actualSize);
    return CompressionStatus::SUCCESS;
}

cyrex::mcpe::protocol::types::CompressionAlgorithm SnappyCompressor::networkId() const noexcept
{
    return cyrex::mcpe::protocol::types::CompressionAlgorithm::SNAPPY;
}

std::optional<size_t> SnappyCompressor::compressionThreshold() const noexcept
{
    return m_minCompressionSize;
}
} // namespace cyrex::network::mcbe::compression