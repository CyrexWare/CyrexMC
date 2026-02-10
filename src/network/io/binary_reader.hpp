#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <cstdint>

namespace cyrex::nw::io
{

using UUID = std::array<uint8_t, 16>;

class BinaryReader
{
public:
    BinaryReader() = default;

    BinaryReader(const uint8_t* data, const size_t len) : buffer(data, data + len)
    {
    }

    std::vector<uint8_t> buffer;
    size_t offset = 0;

    [[nodiscard]] size_t remaining() const
    {
        return buffer.size() - offset;
    }

    void reset()
    {
        offset = 0;
    }

    uint8_t readU8()
    {
        ensureReadable(1);
        return buffer.at(offset++);
    }

    int8_t readI8()
    {
        return static_cast<int8_t>(readU8());
    }

    uint16_t readU16LE()
    {
        ensureReadable(2);
        const uint16_t v = buffer.at(offset) | (buffer.at(offset + 1) << 8);
        offset += 2;
        return v;
    }

    uint16_t readU16BE()
    {
        ensureReadable(2);
        const uint16_t v = (buffer.at(offset) << 8) | buffer.at(offset + 1);
        offset += 2;
        return v;
    }

    int16_t readI16LE()
    {
        return static_cast<int16_t>(readU16LE());
    }

    int16_t readI16BE()
    {
        return static_cast<int16_t>(readU16BE());
    }

    int32_t readI32LE()
    {
        return static_cast<int32_t>(readU32LE());
    }

    int32_t readI32BE()
    {
        return static_cast<int32_t>(readU32BE());
    }

    int64_t readI64LE()
    {
        return static_cast<int64_t>(readU64LE());
    }

    int64_t readI64BE()
    {
        return static_cast<int64_t>(readU64BE());
    }

    int16_t readShort()
    {
        return readI16BE();
    }

    uint16_t readUShort()
    {
        return readU16BE();
    }

    uint32_t readU32LE()
    {
        ensureReadable(4);
        const uint32_t v = buffer.at(offset) | (buffer.at(offset + 1) << 8) | (buffer.at(offset + 2) << 16) |
                           (buffer.at(offset + 3) << 24);
        offset += 4;
        return v;
    }

    uint32_t readU32BE()
    {
        ensureReadable(4);
        const uint32_t v = (buffer.at(offset) << 24) | (buffer.at(offset + 1) << 16) | (buffer.at(offset + 2) << 8) |
                           buffer.at(offset + 3);
        offset += 4;
        return v;
    }

    uint64_t readU64LE()
    {
        ensureReadable(8);
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i)
            v |= static_cast<uint64_t>(buffer.at(offset + i)) << (i * 8);
        offset += 8;
        return v;
    }

    uint64_t readU64BE()
    {
        ensureReadable(8);
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i)
            v = (v << 8) | buffer.at(offset + i);
        offset += 8;
        return v;
    }

    bool readBool()
    {
        return readU8() != 0;
    }

    float readFloatLE()
    {
        return std::bit_cast<float>(readU32LE());
    }

    double readDoubleLE()
    {
        return std::bit_cast<double>(readU64LE());
    }

    uint32_t readVarUInt()
    {
        uint32_t value = 0;
        int shift = 0;

        for (int i = 0; i < 5; ++i)
        {
            const uint8_t b = readU8();
            value |= static_cast<uint32_t>(b & 0x7F) << shift;
            if ((b & 0x80) == 0)
                return value;
            shift += 7;
        }

        throw std::runtime_error("VarUInt overflow");
    }

    int32_t readVarInt()
    {
        const uint32_t v = readVarUInt();
        return static_cast<int32_t>(v >> 1 ^ -1 * (v & 1));
    }

    uint64_t readVarULong()
    {
        uint64_t value = 0;
        int shift = 0;

        for (int i = 0; i < 10; ++i)
        {
            const uint8_t b = readU8();
            value |= static_cast<uint64_t>(b & 0x7F) << shift;
            if ((b & 0x80) == 0)
                return value;
            shift += 7;
        }

        throw std::runtime_error("VarULong overflow");
    }

    int64_t readVarLong()
    {
        const uint64_t v = readVarULong();
        return static_cast<int64_t>(v >> 1 ^ -1 * (v & 1));
    }

    std::string readString()
    {
        const uint32_t len = readVarUInt();
        ensureReadable(len);
        std::string s(reinterpret_cast<char*>(&buffer.at(offset)), len);
        offset += len;
        return s;
    }

    glm::vec2 readVector2()
    {
        return {readFloatLE(), readFloatLE()};
    }

    glm::vec3 readVector3()
    {
        return {readFloatLE(), readFloatLE(), readFloatLE()};
    }

    std::string readBytes(const size_t len)
    {
        ensureReadable(len);
        std::string s(reinterpret_cast<char*>(&buffer.at(offset)), len);
        offset += len;
        return s;
    }

    std::vector<uint8_t> readBytesVector(const size_t len)
    {
        ensureReadable(len);
        std::vector<uint8_t> data(buffer.begin() + offset, buffer.begin() + offset + len);
        offset += len;
        return data;
    }

    UUID readUUID()
    {
        ensureReadable(16);
        UUID uuid{};
        for (size_t i = 0; i < 8; ++i)
        {
            uuid[7 - i] = buffer.at(offset + i);
        }
        for (size_t i = 0; i < 8; ++i)
        {
            uuid[15 - i] = buffer.at(offset + 8 + i);
        }
        offset += 16;
        return uuid;
    }

    template <typename T>
    std::optional<T> readOptional(const std::function<T()>& reader)
    {
        if (readBool())
        {
            return reader();
        }
        return std::nullopt;
    }

private:
    void ensureReadable(const size_t n) const
    {
        if (offset + n > buffer.size())
            throw std::runtime_error("BinaryReader overflow");
    }
};

} // namespace cyrex::nw::io
