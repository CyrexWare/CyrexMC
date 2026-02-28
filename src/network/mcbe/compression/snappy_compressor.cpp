#include "snappy_compressor.hpp"

namespace cyrex::network::protocol
{

std::optional<std::vector<uint8_t>> SnappyCompressor::compress(const std::span<const uint8_t> input) const
{
    size_t written = snappy::MaxCompressedLength(input.size());
    std::vector<uint8_t> output(written);
    snappy::RawCompress(reinterpret_cast<const char*>(input.data()),
                        input.size(),
                        reinterpret_cast<char*>(output.data()),
                        &written);
    output.resize(written);
    return output;
}

std::optional<std::vector<uint8_t>> SnappyCompressor::decompress(const std::span<const uint8_t> input) const
{
    if (!snappy::IsValidCompressedBuffer(reinterpret_cast<const char*>(input.data()), input.size()))
    {
        return std::nullopt;
    }

    size_t actualSize = 0;
    if (!snappy::GetUncompressedLength(reinterpret_cast<const char*>(input.data()), input.size(), &actualSize))
    {
        return std::nullopt;
    }

    std::vector<uint8_t> output(actualSize);
    if (!snappy::RawUncompress(reinterpret_cast<const char*>(input.data()),
                               input.size(),
                               reinterpret_cast<char*>(output.data())))
    {
        return std::nullopt;
    }

    return output;
}
} // namespace cyrex::network::protocol