#pragma once

#include "compressor.hpp"

#include <libdeflate.h>
#include <memory>

namespace cyrex::nw::protocol
{
class ZlibCompressor final : public Compressor
{
public:
    static constexpr int defaultLevel = 7;
    static constexpr size_t defaultMaxDecompressionSize = 8 * 1024 * 1024;

    explicit ZlibCompressor(int level = defaultLevel, size_t maxDecompressionSize = defaultMaxDecompressionSize);

    std::optional<std::vector<uint8_t>> decompress(std::span<const uint8_t> input) const override;
    std::optional<std::vector<uint8_t>> compress(std::span<const uint8_t> input) const override;

private:
    size_t m_maxDecompressionSize;

    struct CustomDeleter_libdeflate_compressor
    {
        void operator()(libdeflate_compressor* compressor) const
        {
            if (compressor)
            {
                libdeflate_free_compressor(compressor);
            }
        }
    };

    struct CustomDeleter_libdeflate_decompressor
    {
        void operator()(libdeflate_decompressor* decompressor) const
        {
            if (decompressor)
            {
                libdeflate_free_decompressor(decompressor);
            }
        }
    };

    std::unique_ptr<libdeflate_compressor, CustomDeleter_libdeflate_compressor> m_compressor;
    std::unique_ptr<libdeflate_decompressor, CustomDeleter_libdeflate_decompressor> m_decompressor;
};
} // namespace cyrex::nw::protocol
