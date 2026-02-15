#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "uuid_helper.hpp"

#include <algorithm>
#include <bit>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

#include <cstdint>

namespace cyrex::nw::io
{

class BinaryWriter
{
    std::vector<uint8_t> buffer;

    template <typename T>
        requires(std::is_trivially_copyable_v<T>)
    inline void writeRawLE(T value)
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(value);
            std::reverse(bytes.begin(), bytes.end());
            buffer.insert(buffer.end(), bytes.begin(), bytes.end());
        }
        else
        {
            auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(value);
            buffer.insert(buffer.end(), bytes.begin(), bytes.end());
        }
    }

    template <typename T>
        requires(std::is_trivially_copyable_v<T>)
    inline void writeRawBE(T value)
    {
        if constexpr (std::endian::native == std::endian::little)
        {
            auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(value);
            std::reverse(bytes.begin(), bytes.end());
            buffer.insert(buffer.end(), bytes.begin(), bytes.end());
        }
        else
        {
            auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(value);
            buffer.insert(buffer.end(), bytes.begin(), bytes.end());
        }
    }

public:
    BinaryWriter() = default;

    explicit BinaryWriter(size_t reserveSize)
    {
        buffer.reserve(reserveSize);
    }

    inline void reserve(size_t size)
    {
        buffer.reserve(size);
    }

    inline void clear()
    {
        buffer.clear();
    }

    inline const uint8_t* data() const
    {
        return buffer.data();
    }

    inline size_t size() const
    {
        return buffer.size();
    }

    inline const std::vector<uint8_t>& getBuffer() const
    {
        return buffer;
    }

    inline void writeU8(uint8_t v)
    {
        buffer.emplace_back(v);
    }

    inline void writeI8(int8_t v)
    {
        buffer.emplace_back(static_cast<uint8_t>(v));
    }

    inline void writeBool(bool v)
    {
        buffer.emplace_back(static_cast<uint8_t>(v));
    }

    inline void writeU16LE(uint16_t v)
    {
        writeRawLE(v);
    }
    inline void writeU16BE(uint16_t v)
    {
        writeRawBE(v);
    }
    inline void writeI16LE(int16_t v)
    {
        writeRawLE(v);
    }
    inline void writeI16BE(int16_t v)
    {
        writeRawBE(v);
    }

    inline void writeU32LE(uint32_t v)
    {
        writeRawLE(v);
    }
    inline void writeU32BE(uint32_t v)
    {
        writeRawBE(v);
    }
    inline void writeI32LE(int32_t v)
    {
        writeRawLE(v);
    }
    inline void writeI32BE(int32_t v)
    {
        writeRawBE(v);
    }

    inline void writeU64LE(uint64_t v)
    {
        writeRawLE(v);
    }
    inline void writeU64BE(uint64_t v)
    {
        writeRawBE(v);
    }
    inline void writeI64LE(int64_t v)
    {
        writeRawLE(v);
    }
    inline void writeI64BE(int64_t v)
    {
        writeRawBE(v);
    }

    inline void writeFloatLE(float v)
    {
        writeRawLE(std::bit_cast<uint32_t>(v));
    }

    inline void writeDoubleLE(double v)
    {
        writeRawLE(std::bit_cast<uint64_t>(v));
    }

    inline void writeVector2(const glm::vec2& v)
    {
        writeFloatLE(v.x);
        writeFloatLE(v.y);
    }

    inline void writeVector3(const glm::vec3& v)
    {
        writeFloatLE(v.x);
        writeFloatLE(v.y);
        writeFloatLE(v.z);
    }

    inline void writeBytes(std::span<const uint8_t> bytes)
    {
        buffer.insert(buffer.end(), bytes.begin(), bytes.end());
    }

    inline void writeBytes(const uint8_t* data, size_t len)
    {
        buffer.insert(buffer.end(), data, data + len);
    }

    inline void writeVarUInt(uint32_t v)
    {
        while (v >= 0x80)
        {
            buffer.emplace_back(static_cast<uint8_t>(v) | 0x80);
            v >>= 7;
        }
        buffer.emplace_back(static_cast<uint8_t>(v));
    }

    inline void writeVarInt(int32_t v)
    {
        writeVarUInt(static_cast<uint32_t>((v << 1) ^ (v >> 31)));
    }

    inline void writeVarULong(uint64_t v)
    {
        while (v >= 0x80)
        {
            buffer.emplace_back(static_cast<uint8_t>(v) | 0x80);
            v >>= 7;
        }
        buffer.emplace_back(static_cast<uint8_t>(v));
    }

    inline void writeVarLong(int64_t v)
    {
        writeVarULong(static_cast<uint64_t>((v << 1) ^ (v >> 63)));
    }

    static inline size_t getVarUIntSize(uint32_t v)
    {
        size_t len = 1;
        while (v >= 0x80)
        {
            v >>= 7;
            ++len;
        }
        return len;
    }

    static inline size_t getVarULongSize(uint64_t v)
    {
        size_t len = 1;
        while (v >= 0x80)
        {
            v >>= 7;
            ++len;
        }
        return len;
    }

    static inline size_t getVarIntSize(int32_t v)
    {
        return getVarUIntSize(static_cast<uint32_t>((v << 1) ^ (v >> 31)));
    }

    static inline size_t getVarLongSize(int64_t v)
    {
        return getVarULongSize(static_cast<uint64_t>((v << 1) ^ (v >> 63)));
    }

    inline void writeString(std::string_view str)
    {
        writeVarUInt(static_cast<uint32_t>(str.size()));
        buffer.insert(buffer.end(), str.begin(), str.end());
    }

    inline void writeUUID(const UUID& uuid)
    {
        auto bytes = uuidToBytes(uuid);
        std::reverse(bytes.begin(), bytes.begin() + 8);
        std::reverse(bytes.begin() + 8, bytes.end());
        writeBytes(std::vector<uint8_t>(bytes.begin(), bytes.end()));
    }

    template <typename T, typename F>
    inline void writeOptional(const std::optional<T>& value, F&& writer)
    {
        if (value)
        {
            writeBool(true);
            writer(*value);
        }
        else
        {
            writeBool(false);
        }
    }
};

} // namespace cyrex::nw::io
