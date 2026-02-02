#pragma once

#include "compressor.hpp"

#include <snappy.h>

namespace cyrex::network::mcbe::compression
{
class SnappyCompressor final : public Compressor
{
public:
    static constexpr size_t defaultThreshold = 256;
    static constexpr size_t defaultMaxDecompressionSize = 8 * 1024 * 1024;

    explicit SnappyCompressor(std::optional<size_t> minSize = defaultThreshold,
                              size_t maxDecompressionSize = defaultMaxDecompressionSize);

    CompressionStatus decompress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output) override;

    CompressionStatus compress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output) override;

    [[nodiscard]] [[nodiscard]] cyrex::mcpe::protocol::types::CompressionAlgorithm networkId() const noexcept override;

    [[nodiscard]] [[nodiscard]] std::optional<size_t> compressionThreshold() const noexcept override;

private:
    std::optional<size_t> m_minCompressionSize;
    size_t m_maxDecompressionSize;
};
} // namespace cyrex::network::mcbe::compression