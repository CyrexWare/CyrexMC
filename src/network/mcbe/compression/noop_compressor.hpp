#pragma once

#include "compressor.hpp"

namespace cyrex::network::mcbe::compression
{

class NoopCompressor final : public Compressor
{
public:
    CompressionStatus decompress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output) override
    {
        output.assign(input, input + inputSize);
        return CompressionStatus::RAW;
    }

    CompressionStatus compress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output) override
    {
        output.assign(input, input + inputSize);
        return CompressionStatus::SUCCESS;
    }

    [[nodiscard]] cyrex::mcpe::protocol::types::CompressionAlgorithm networkId() const noexcept override
    {
        return cyrex::mcpe::protocol::types::CompressionAlgorithm::NONE;
    }

    [[nodiscard]] std::optional<size_t> compressionThreshold() const noexcept override
    {
        return std::nullopt;
    }
};
} // namespace cyrex::network::mcbe::compression
