#pragma once

#include "math/vector2.hpp"
#include "math/vector3.hpp"

#include <bit>
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

    void writeU8(const uint8_t v)
    {
        buffer.push_back(v);
    }

    void writeI8(const int8_t v)
    {
        writeU8(static_cast<uint8_t>(v));
    }

    void writeU16LE(const uint16_t v)
    {
        writeU8(v);
        writeU8(v >> 8);
    }

    void writeU16BE(const uint16_t v)
    {
        writeU8(v >> 8);
        writeU8(v);
    }

    void writeI16LE(const int16_t v)
    {
        writeU16LE(static_cast<uint16_t>(v));
    }

    void writeI16BE(const int16_t v)
    {
        writeU16BE(static_cast<uint16_t>(v));
    }

    void writeShort(const int16_t v)
    {
        writeI16BE(v);
    }

    void writeUShort(const uint16_t v)
    {
        writeU16BE(v);
    }

    void writeU32LE(const uint32_t v)
    {
        writeU8(v);
        writeU8(v >> 8);
        writeU8(v >> 16);
        writeU8(v >> 24);
    }

    void writeU32BE(const uint32_t v)
    {
        writeU8(v >> 24);
        writeU8(v >> 16);
        writeU8(v >> 8);
        writeU8(v);
    }

    void writeU64LE(const uint64_t v)
    {
        writeU32BE(v & 0xFFFFFFFF);
        writeU32BE(v >> 32);
    }

    void writeBool(const bool v)
    {
        writeU8(v ? 1 : 0);
    }

    void writeFloatLE(const float v)
    {
        writeU32LE(std::bit_cast<uint32_t>(v));
    }

    void writeDoubleLE(const double v)
    {
        writeU64LE(std::bit_cast<uint64_t>(v));
    }

    void writeVector2(const math::Vector2& v)
    {
        writeFloatLE(v.x);
        writeFloatLE(v.y);
    }

    void writeVector3(const math::Vector3& v)
    {
        writeFloatLE(v.x);
        writeFloatLE(v.y);
        writeFloatLE(v.z);
    }

    void writeBuffer(const uint8_t* data, const size_t len)
    {
        buffer.insert(buffer.end(), data, data + len);
    }

    void writeVarUInt(uint32_t v)
    {
        while (true)
        {
            const uint8_t b = v & 0x7F;
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

    static size_t getVarUIntSize(uint32_t v)
    {
        size_t len = 0;
        do
        {
            len++;
            v >>= 7;
        } while (v != 0);
        return len;
    }

    void writeVarInt(const int32_t v)
    {
        writeVarUInt(static_cast<uint32_t>(v << 1 ^ (v < 0 ? ~0 : 0)));
    }

    static size_t getVarIntSize(int32_t v)
    {
        return getVarUIntSize(static_cast<uint32_t>(v << 1 ^ (v < 0 ? ~0 : 0)));
    }

    void writeVarULong(uint64_t v)
    {
        while (true)
        {
            const uint8_t b = v & 0x7F;
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

    static size_t getVarULongSize(uint64_t v)
    {
        size_t len = 0;
        do
        {
            len++;
            v >>= 7;
        } while (v != 0);
        return len;
    }

    void writeVarLong(const int64_t v)
    {
        writeVarULong(static_cast<uint64_t>(v << 1 ^ (v < 0 ? ~0 : 0)));
    }

    static size_t getVarLongSize(const int64_t v)
    {
        return getVarULongSize(static_cast<uint64_t>(v << 1 ^ (v < 0 ? ~0 : 0)));
    }

    void writeString(const std::string& s)
    {
        writeVarUInt(static_cast<uint32_t>(s.size()));
        buffer.insert(buffer.end(), s.begin(), s.end());
    }

    [[nodiscard]] const uint8_t* data() const
    {
        return buffer.data();
    }

    [[nodiscard]] size_t length() const
    {
        return buffer.size();
    }
};

} // namespace cyrex::network::io
