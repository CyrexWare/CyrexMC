#pragma once

#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"

#include <optional>
#include <vector>

#include <cstddef>
#include <cstdint>

namespace cyrex::network::mcbe::compression
{

class Compressor
{
public:
    virtual ~Compressor() = default;

    virtual bool decompress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output) = 0;
    virtual bool compress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output) = 0;
    virtual cyrex::mcpe::protocol::types::CompressionAlgorithm networkId() const noexcept = 0;
    virtual std::optional<size_t> compressionThreshold() const noexcept = 0;
};
} // namespace cyrex::network::mcbe::compression