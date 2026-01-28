#pragma once

#include "math/vector2.hpp"
#include "math/vector3.hpp"

#include <string>
#include <vector>

#include <cstdint>
#include <cstring>

namespace cyrex::network::io
{

class BinaryWriter
{
public:
    std::vector<uint8_t> buffer;

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

    void writeShort(int16_t v)
    {
        writeI16BE(v);
    }

    void writeUShort(uint16_t v)
    {
        writeU16BE(v);
    }

    void writeU32LE(uint32_t v)
    {
        writeU8(v);
        writeU8(v >> 8);
        writeU8(v >> 16);
        writeU8(v >> 24);
    }

    void writeU32BE(uint32_t v)
    {
        writeU8(v >> 24);
        writeU8(v >> 16);
        writeU8(v >> 8);
        writeU8(v);
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

    void writeVector2(const cyrex::math::Vector2& v)
    {
        writeFloatLE(v.x);
        writeFloatLE(v.y);
    }

    void writeVector3(const cyrex::math::Vector3& v)
    {
        writeFloatLE(v.x);
        writeFloatLE(v.y);
        writeFloatLE(v.z);
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

    void writeVarLong(int64_t v)
    {
        writeVarULong(static_cast<uint64_t>(v));
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
};

} // namespace cyrex::network::io
