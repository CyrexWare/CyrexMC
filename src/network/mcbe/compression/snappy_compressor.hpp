#pragma once

#include "compressor.hpp"

#include <snappy.h>

namespace cyrex::nw::protocol
{
class SnappyCompressor final : public Compressor
{
public:
    [[nodiscard]] std::optional<std::vector<uint8_t>> decompress(std::span<const uint8_t> input) const override;
    [[nodiscard]] std::optional<std::vector<uint8_t>> compress(std::span<const uint8_t> input) const override;
};
} // namespace cyrex::nw::protocol