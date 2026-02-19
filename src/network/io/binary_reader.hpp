#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "util/uuid.hpp"

#include <array>
#include <bit>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include <cstdint>

namespace cyrex::nw::io
{

class BinaryReader
{
    const uint8_t* m_dataPtr = nullptr;
    size_t m_dataSize = 0;

    inline void ensureReadable(size_t n) const
    {
        if (offset + n > m_dataSize)
            throw std::runtime_error("BinaryReader overflow");
    }

    template <typename T>
        requires std::is_trivially_copyable_v<T>
    inline T readRawLE()
    {
        ensureReadable(sizeof(T));
        T value;
        std::memcpy(&value, m_dataPtr + offset, sizeof(T));
        offset += sizeof(T);

        if constexpr (std::endian::native == std::endian::big)
            value = std::byteswap(value);
        return value;
    }

    template <typename T>
        requires std::is_trivially_copyable_v<T>
    inline T readRawBE()
    {
        ensureReadable(sizeof(T));
        T value;
        std::memcpy(&value, m_dataPtr + offset, sizeof(T));
        offset += sizeof(T);

        if constexpr (std::endian::native == std::endian::little)
            value = std::byteswap(value);
        return value;
    }

public:
    BinaryReader() = default;
    size_t offset = 0;

    BinaryReader(const uint8_t* data, const size_t len) : m_dataPtr(data), m_dataSize(len)
    {
    }

    explicit BinaryReader(const std::span<const uint8_t> span) : m_dataPtr(span.data()), m_dataSize(span.size())
    {
    }

    inline void reset()
    {
        offset = 0;
    }

    inline size_t remaining() const
    {
        return m_dataSize - offset;
    }

    inline size_t position() const
    {
        return offset;
    }

    inline const uint8_t* currentPtr() const
    {
        return m_dataPtr + offset;
    }

    inline uint8_t readU8()
    {
        ensureReadable(1);
        return m_dataPtr[offset++];
    }

    inline int8_t readI8()
    {
        return static_cast<int8_t>(readU8());
    }

    inline bool readBool()
    {
        return readU8() != 0;
    }

    inline uint16_t readU16LE()
    {
        return readRawLE<uint16_t>();
    }

    inline uint16_t readU16BE()
    {
        return readRawBE<uint16_t>();
    }

    inline int16_t readI16LE()
    {
        return readRawLE<int16_t>();
    }

    inline int16_t readI16BE()
    {
        return readRawBE<int16_t>();
    }

    inline uint32_t readU32LE()
    {
        return readRawLE<uint32_t>();
    }

    inline uint32_t readU32BE()
    {
        return readRawBE<uint32_t>();
    }

    inline int32_t readI32LE()
    {
        return readRawLE<int32_t>();
    }

    inline int32_t readI32BE()
    {
        return readRawBE<int32_t>();
    }

    inline float readFloatLE()
    {
        return std::bit_cast<float>(readU32LE());
    }

    inline uint64_t readU64LE()
    {
        return readRawLE<uint64_t>();
    }

    inline uint64_t readU64BE()
    {
        return readRawBE<uint64_t>();
    }

    inline int64_t readI64LE()
    {
        return readRawLE<int64_t>();
    }

    inline int64_t readI64BE()
    {
        return readRawBE<int64_t>();
    }

    inline double readDoubleLE()
    {
        return std::bit_cast<double>(readU64LE());
    }

    inline glm::vec2 readVector2()
    {
        return {readFloatLE(), readFloatLE()};
    }

    inline glm::vec3 readVector3()
    {
        return {readFloatLE(), readFloatLE(), readFloatLE()};
    }

    inline uint32_t readVarUInt()
    {
        uint32_t value = 0;
        uint32_t shift = 0;

        for (uint32_t i = 0; i < 5; ++i)
        {
            const uint8_t b = readU8();
            value |= static_cast<uint32_t>(b & 0x7F) << shift;

            if ((b & 0x80) == 0)
                return value;

            shift += 7;
        }

        throw std::runtime_error("VarUInt overflow");
    }

    inline int32_t readVarInt()
    {
        const uint32_t v = readVarUInt();
        return static_cast<int32_t>(v >> 1 ^ -1 * (v & 1));
    }

    inline uint64_t readVarULong()
    {
        uint64_t value = 0;
        uint32_t shift = 0;

        for (uint32_t i = 0; i < 10; ++i)
        {
            const uint8_t b = readU8();
            value |= static_cast<uint64_t>(b & 0x7F) << shift;

            if ((b & 0x80) == 0)
                return value;

            shift += 7;
        }

        throw std::runtime_error("VarULong overflow");
    }

    inline int64_t readVarLong()
    {
        const uint64_t v = readVarULong();
        return static_cast<int64_t>(v >> 1 ^ -1 * (v & 1));
    }

    inline std::string readString()
    {
        const uint32_t len = readVarUInt();
        ensureReadable(len);

        std::string out(reinterpret_cast<const char*>(m_dataPtr + offset), len);
        offset += len;
        return out;
    }

    inline std::vector<uint8_t> readBytesVector(const size_t len)
    {
        ensureReadable(len);
        std::vector out(m_dataPtr + offset, m_dataPtr + offset + len);
        offset += len;
        return out;
    }

    inline uuid::UUID readUUID()
    {
        const auto mostSignificant = readRawBE<uint64_t>();
        const auto leastSignificant = readRawBE<uint64_t>();

        std::array<uint8_t, 16> bytes{};
        std::memcpy(bytes.data(), &mostSignificant, 8);
        std::memcpy(bytes.data() + 8, &leastSignificant, 8);

        return uuid::UUID{bytes.begin(), bytes.end()};
    }

    inline std::string readStringU32LE()
    {
        const uint32_t len = readU32LE();
        ensureReadable(len);

        std::string out(reinterpret_cast<const char*>(m_dataPtr + offset), len);
        offset += len;
        return out;
    }


    template <typename T, typename F>
    inline std::optional<T> readOptional(F&& reader)
    {
        if (readBool())
            return std::invoke(std::forward<F>(reader));

        return std::nullopt;
    }
};

} // namespace cyrex::nw::io
