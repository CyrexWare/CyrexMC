#pragma once

#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"

#include <optional>
#include <span>
#include <vector>

#include <cstddef>
#include <cstdint>

namespace cyrex::network::mcbe::compression
{

class Compressor
{
public:
    virtual ~Compressor() = default;

    [[nodiscard]] virtual std::optional<std::vector<uint8_t>> decompress(std::span<const uint8_t> input) const = 0;
    [[nodiscard]] virtual std::optional<std::vector<uint8_t>> compress(std::span<const uint8_t> input) const = 0;

    [[nodiscard]] bool shouldCompress(const std::size_t inputSize) const
    {
        const bool compressible = !minCompressionSize.has_value() || inputSize >= *minCompressionSize;
        return compressible;
    }

    static constexpr size_t defaultThreshold = 256;
    std::optional<size_t> minCompressionSize = defaultThreshold;
};
} // namespace cyrex::network::mcbe::compression
