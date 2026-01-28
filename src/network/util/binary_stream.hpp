#pragma once

#include "math/vector2.hpp"
#include "math/vector3.hpp"

#include <stdexcept>
#include <string>
#include <vector>

#include <cstdint>
#include <cstring>

namespace cyrex::network::util
{

class BinaryStream
{
public:
    BinaryStream() = default;

    BinaryStream(const uint8_t* data, size_t len) : buffer(data, data + len), offset(0)
    {
    }

    std::vector<uint8_t> buffer;
    size_t offset = 0;

    size_t remaining() const
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
        uint16_t v = buffer[offset] | (buffer[offset + 1] << 8);
        offset += 2;
        return v;
    }

    uint16_t readU16BE()
    {
        ensureReadable(2);
        uint16_t v = (buffer[offset] << 8) | buffer[offset + 1];
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

    uint32_t readU32LE()
    {
        ensureReadable(4);
        uint32_t v = buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
        offset += 4;
        return v;
    }

    uint32_t readU32BE()
    {
        ensureReadable(4);
        uint32_t v = (buffer[offset] << 24) | (buffer[offset + 1] << 16) | (buffer[offset + 2] << 8) | buffer[offset + 3];
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
        float f;
        std::memcpy(&f, &bits, sizeof(float));
        return f;
    }

    double readDoubleLE()
    {
        uint64_t bits = readU64LE();
        double d;
        std::memcpy(&d, &bits, sizeof(double));
        return d;
    }

    uint32_t readVarUInt()
    {
        uint32_t value = 0;
        int shift = 0;

        for (int i = 0; i < 5; ++i)
        {
            uint8_t b = readU8();
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
            uint8_t b = readU8();
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
        uint32_t len = readVarUInt();
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

    void writeU8(uint8_t v)
    {
        buffer.push_back(v);
    }

    void writeI8(int8_t v)
    {
        writeU8(static_cast<uint8_t>(v));
    }

    void writeU16LE(uint16_t v)
    {
        writeU8(v);
        writeU8(v >> 8);
    }

    void writeU16BE(uint16_t v)
    {
        writeU8(v >> 8);
        writeU8(v);
    }

    void writeI16LE(int16_t v)
    {
        writeU16LE(static_cast<uint16_t>(v));
    }

    void writeI16BE(int16_t v)
    {
        writeU16BE(static_cast<uint16_t>(v));
    }

    void writeU32BE(uint32_t v)
    {
        writeU8(static_cast<uint8_t>(v >> 24));
        writeU8(static_cast<uint8_t>(v >> 16));
        writeU8(static_cast<uint8_t>(v >> 8));
        writeU8(static_cast<uint8_t>(v));
    }

    void writeU32LE(uint32_t v)
    {
        writeU8(v);
        writeU8(v >> 8);
        writeU8(v >> 16);
        writeU8(v >> 24);
    }

    void writeU64LE(uint64_t v)
    {
        for (int i = 0; i < 8; ++i)
            writeU8(v >> (i * 8));
    }

    void writeBool(bool v)
    {
        writeU8(v ? 1 : 0);
    }

    void writeFloatLE(float v)
    {
        uint32_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        writeU32LE(bits);
    }

    void writeDoubleLE(double v)
    {
        uint64_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        writeU64LE(bits);
    }

    void writeBuffer(const uint8_t* data, size_t len)
    {
        buffer.insert(buffer.end(), data, data + len);
    }

    void writeVarUInt(uint32_t v)
    {
        while (true)
        {
            uint8_t b = v & 0x7F;
            v >>= 7;
            if (v)
                writeU8(b | 0x80);
            else
            {
                writeU8(b);
                break;
            }
        }
    }

    void writeVarInt(int32_t v)
    {
        writeVarUInt(static_cast<uint32_t>(v));
    }

    void writeVarLong(int64_t v)
    {
        writeVarULong(static_cast<uint64_t>(v));
    }

    void writeVarULong(uint64_t v)
    {
        while (true)
        {
            uint8_t b = v & 0x7F;
            v >>= 7;
            if (v)
                writeU8(b | 0x80);
            else
            {
                writeU8(b);
                break;
            }
        }
    }

    void writeString(const std::string& s)
    {
        writeVarUInt(static_cast<uint32_t>(s.size()));
        buffer.insert(buffer.end(), s.begin(), s.end());
    }

    const uint8_t* data() const
    {
        return buffer.data();
    }

    size_t length() const
    {
        return buffer.size();
    }

private:
    void ensureReadable(size_t n)
    {
        if (offset + n > buffer.size())
            throw std::runtime_error("BinaryStream overflow");
    }
};

} // namespace cyrex::network::util