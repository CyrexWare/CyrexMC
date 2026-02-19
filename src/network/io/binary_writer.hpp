#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "util/uuid.hpp"

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
    std::vector<uint8_t> m_buffer;

    template <typename T>
        requires(std::is_trivially_copyable_v<T>)
    inline void writeRawLE(T value)
    {
        if constexpr (std::endian::native == std::endian::big)
            value = std::byteswap(value);

        auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(value);
        m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end());
    }

    template <typename T>
        requires(std::is_trivially_copyable_v<T>)
    inline void writeRawBE(T value)
    {
        if constexpr (std::endian::native == std::endian::little)
            value = std::byteswap(value);

        auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(value);
        m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end());
    }

public:
    BinaryWriter() = default;

    explicit BinaryWriter(const size_t reserveSize)
    {
        m_buffer.reserve(reserveSize);
    }

    inline void reserve(const size_t size)
    {
        m_buffer.reserve(size);
    }

    inline void clear()
    {
        m_buffer.clear();
    }

    inline const uint8_t* data() const
    {
        return m_buffer.data();
    }

    inline size_t size() const
    {
        return m_buffer.size();
    }

    inline const std::vector<uint8_t>& getBuffer() const
    {
        return m_buffer;
    }

    inline void writeU8(uint8_t v)
    {
        m_buffer.emplace_back(v);
    }

    inline void writeI8(const int8_t v)
    {
        m_buffer.emplace_back(static_cast<uint8_t>(v));
    }

    inline void writeBool(const bool v)
    {
        m_buffer.emplace_back(static_cast<uint8_t>(v));
    }

    inline void writeU16LE(const uint16_t v)
    {
        writeRawLE(v);
    }

    inline void writeU16BE(const uint16_t v)
    {
        writeRawBE(v);
    }

    inline void writeI16LE(const int16_t v)
    {
        writeRawLE(v);
    }

    inline void writeI16BE(const int16_t v)
    {
        writeRawBE(v);
    }

    inline void writeU32LE(const uint32_t v)
    {
        writeRawLE(v);
    }

    inline void writeU32BE(const uint32_t v)
    {
        writeRawBE(v);
    }

    inline void writeI32LE(const int32_t v)
    {
        writeRawLE(v);
    }

    inline void writeI32BE(const int32_t v)
    {
        writeRawBE(v);
    }

    inline void writeFloatLE(const float v)
    {
        writeRawLE(std::bit_cast<uint32_t>(v));
    }

    inline void writeU64LE(const uint64_t v)
    {
        writeRawLE(v);
    }

    inline void writeU64BE(const uint64_t v)
    {
        writeRawBE(v);
    }

    inline void writeI64LE(const int64_t v)
    {
        writeRawLE(v);
    }

    inline void writeI64BE(const int64_t v)
    {
        writeRawBE(v);
    }

    inline void writeDoubleLE(const double v)
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
        m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end());
    }

    inline void writeBytes(const uint8_t* data, size_t len)
    {
        m_buffer.insert(m_buffer.end(), data, data + len);
    }

    inline void writeVarUInt(uint32_t v)
    {
        while (v >= 0x80)
        {
            m_buffer.emplace_back(static_cast<uint8_t>(v) | 0x80);
            v >>= 7;
        }
        m_buffer.emplace_back(static_cast<uint8_t>(v));
    }

    inline void writeVarInt(const int32_t v)
    {
        writeVarUInt(static_cast<uint32_t>(v << 1 ^ -(v & 1)));
    }

    inline void writeVarULong(uint64_t v)
    {
        while (v >= 0x80)
        {
            m_buffer.emplace_back(static_cast<uint8_t>(v) | 0x80);
            v >>= 7;
        }
        m_buffer.emplace_back(static_cast<uint8_t>(v));
    }

    inline void writeVarLong(const int64_t v)
    {
        writeVarULong(static_cast<uint64_t>(v << 1 ^ -(v & 1)));
    }

    static inline size_t getVarUIntSize(uint32_t v)
    {
        size_t len = 0;
        do
        {
            len++;
            v >>= 7;
        } while (v != 0);
        return len;
    }

    static inline size_t getVarULongSize(uint64_t v)
    {
        size_t len = 0;
        do
        {
            len++;
            v >>= 7;
        } while (v != 0);
        return len;
    }

    static inline size_t getVarIntSize(const int32_t v)
    {
        return getVarUIntSize(static_cast<uint32_t>(v << 1 ^ -(v & 1)));
    }

    static inline size_t getVarLongSize(const int64_t v)
    {
        return getVarULongSize(static_cast<uint64_t>(v << 1 ^ -(v & 1)));
    }

    inline void writeString(const std::string_view str)
    {
        writeVarUInt(static_cast<uint32_t>(str.size()));
        m_buffer.insert(m_buffer.end(), str.begin(), str.end());
    }

    inline void writeUUID(const uuid::UUID& uuid)
    {
        auto bytes = uuid::toBytes(uuid);
        std::reverse(bytes.begin(), bytes.begin() + 8);
        std::reverse(bytes.begin() + 8, bytes.end());
        writeBytes(bytes);
    }

    inline void writeStringU32LE(const std::string_view str)
    {
        writeU32LE(static_cast<uint32_t>(str.size()));
        writeBytes(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }

    template <typename T, typename F>
    inline void writeOptional(const std::optional<T>& value, F&& writer)
    {
        writeBool(value.has_value());
        if (value)
            std::invoke(std::forward<F>(writer), *value);
    }
};

} // namespace cyrex::nw::io
