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
        uint16_t const v = buffer[offset] | (buffer[offset + 1] << 8);
        offset += 2;
        return v;
    }

    uint16_t readU16BE()
    {
        ensureReadable(2);
        uint16_t const v = (buffer[offset] << 8) | buffer[offset + 1];
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
        uint32_t const v = buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
        offset += 4;
        return v;
    }

    uint32_t readU32BE()
    {
        ensureReadable(4);
        uint32_t const v = (buffer[offset] << 24) | (buffer[offset + 1] << 16) | (buffer[offset + 2] << 8) | buffer[offset + 3];
        offset += 4;
        return v;
    }

    uint64_t readU64LE()
    {
        ensureReadable(8);
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i)
            v |= uint64_t(buffer[offset + i]) << (i * 8);
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
        uint32_t bits = readU32LE();
        float f = NAN;
        std::memcpy(&f, &bits, sizeof(f));
        return f;
    }

    double readDoubleLE()
    {
        uint64_t bits = readU64LE();
        double d = NAN;
        std::memcpy(&d, &bits, sizeof(d));
        return d;
    }

    uint32_t readVarUInt()
    {
        uint32_t value = 0;
        int shift = 0;

        for (int i = 0; i < 5; ++i)
        {
            uint8_t const b = readU8();
            value |= uint32_t(b & 0x7F) << shift;
            if ((b & 0x80) == 0)
                return value;
            shift += 7;
        }

        throw std::runtime_error("VarUInt overflow");
    }

    int32_t readVarInt()
    {
        return static_cast<int32_t>(readVarUInt());
    }

    uint64_t readVarULong()
    {
        uint64_t value = 0;
        int shift = 0;

        for (int i = 0; i < 10; ++i)
        {
            uint8_t const b = readU8();
            value |= uint64_t(b & 0x7F) << shift;
            if ((b & 0x80) == 0)
                return value;
            shift += 7;
        }

        throw std::runtime_error("VarULong overflow");
    }

    int64_t readVarLong()
    {
        return static_cast<int64_t>(readVarULong());
    }

    std::string readString()
    {
        uint32_t const len = readVarUInt();
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
