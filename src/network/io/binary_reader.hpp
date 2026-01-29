#pragma once

#include "math/vector2.hpp"
#include "math/vector3.hpp"

#include <math.h>
#include <stdexcept>
#include <string>
#include <vector>

#include <cstdint>
#include <cstring>

namespace cyrex::network::io
{

class BinaryReader
{
public:
    BinaryReader() = default;

    BinaryReader(const uint8_t* data, size_t len) : buffer(data, data + len)
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
        return buffer[offset++];
    }

    int8_t readI8()
    {
        return static_cast<int8_t>(readU8());
    }

    uint16_t readU16LE()
    {
        ensureReadable(2);
        const uint16_t v = buffer[offset] | (buffer[offset + 1] << 8);
        offset += 2;
        return v;
    }

    uint16_t readU16BE()
    {
        ensureReadable(2);
        const uint16_t v = (buffer[offset] << 8) | buffer[offset + 1];
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
        const uint32_t v = buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) |
                           (buffer[offset + 3] << 24);
        offset += 4;
        return v;
    }

    uint32_t readU32BE()
    {
        ensureReadable(4);
        const uint32_t v = (buffer[offset] << 24) | (buffer[offset + 1] << 16) | (buffer[offset + 2] << 8) |
                           buffer[offset + 3];
        offset += 4;
        return v;
    }

    uint64_t readU64LE()
    {
        ensureReadable(8);
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i)
            v |= static_cast<uint64_t>(buffer[offset + i]) << (i * 8);
        offset += 8;
        return v;
    }

    uint64_t readU64BE()
    {
        ensureReadable(8);
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i)
            v = (v << 8) | buffer[offset + i];
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
        std::string s(reinterpret_cast<char*>(&buffer[offset]), len);
        offset += len;
        return s;
    }

    cyrex::math::Vector2 readVector2()
    {
        return {readFloatLE(), readFloatLE()};
    }

    cyrex::math::Vector3 readVector3()
    {
        return {readFloatLE(), readFloatLE(), readFloatLE()};
    }

    std::string readBytes(size_t len)
    {
        ensureReadable(len);
        std::string s(reinterpret_cast<char*>(&buffer[offset]), len);
        offset += len;
        return s;
    }

private:
    void ensureReadable(size_t n) const
    {
        if (offset + n > buffer.size())
            throw std::runtime_error("BinaryReader overflow");
    }
};

} // namespace cyrex::network::io
