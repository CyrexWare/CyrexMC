#pragma once

#include "compressor.hpp"

#include <libdeflate.h>

namespace cyrex::network::mcbe::compression
{
class ZlibCompressor final : public Compressor
{
public:
    static constexpr int defaultLevel = 7;
    static constexpr size_t defaultThreshold = 256;
    static constexpr size_t defaultMaxDecompressionSize = 8 * 1024 * 1024;

    explicit ZlibCompressor(int level = defaultLevel,
                            std::optional<size_t> minSize = defaultThreshold,
                            size_t maxDecompressionSize = defaultMaxDecompressionSize);

    ~ZlibCompressor() override;

    bool decompress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output) override;

    bool compress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output) override;

    [[nodiscard]] [[nodiscard]] cyrex::mcpe::protocol::types::CompressionAlgorithm networkId() const noexcept override;

    [[nodiscard]] [[nodiscard]] std::optional<size_t> compressionThreshold() const noexcept override;

private:
    int m_level;
    std::optional<size_t> m_minCompressionSize;
    size_t m_maxDecompressionSize;

    libdeflate_compressor* m_compressor;
    libdeflate_decompressor* m_decompressor;
};
} // namespace cyrex::network::mcbe::compression
