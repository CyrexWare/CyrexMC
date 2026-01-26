#pragma once

#include <array>
#include <iostream>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace deflate
{

struct HuffmanCode
{
    uint16_t value{};
    uint8_t bits{};
};

struct HuffmanTable : public std::vector<HuffmanCode>
{
    uint8_t maxBits{};

    using std::vector<HuffmanCode>::vector;

    static HuffmanTable makeTable(std::span<const uint8_t> lengths)
    {
        const auto maxLength = std::ranges::max(lengths);

        std::vector<uint16_t> lengthCount(maxLength + 1);
        for (auto l : lengths)
        {
            lengthCount[l]++;
        }
        lengthCount[0] = 0;

        std::vector<uint16_t> nextCode(maxLength + 1);
        uint16_t code{};
        for (uint16_t bits = 1; bits <= maxLength; bits++)
        {
            code = (code + lengthCount[bits - 1]) << 1;
            nextCode[bits] = code;
        }

        HuffmanTable decodeTable(1 << maxLength);
        decodeTable.maxBits = maxLength;

        for (uint16_t x = 0; x < lengths.size(); x++)
        {
            const auto len = lengths[x];
            if (len == 0)
            {
                continue;
            }

            const auto code = nextCode[len]++;

            decodeTable[code << (maxLength - len)] = {x, len};
        }

        auto lastCode = decodeTable[0];
        for (auto& code : decodeTable)
        {
            if (code.bits == 0)
            {
                code = lastCode;
            }
            else
            {
                lastCode = code;
            }
        }

        return decodeTable;
    };
};

template <typename T>
T reverseBits(T v)
{
    T r = v;                           // r will be reversed bits of v; first get LSB of v
    auto s = sizeof(v) * CHAR_BIT - 1; // extra shift needed at end

    for (v >>= 1; v; v >>= 1)
    {
        r <<= 1;
        r |= v & 1;
        s--;
    }
    r <<= s; // shift when v's highest bits are zero

    return r;
}

template <typename T>
T reverseBits(T v, uint8_t maxBits)
{
    return reverseBits(v) >> ((sizeof(T) * 8) - maxBits);
}

template <typename T>
HuffmanCode decodeCode(const HuffmanTable& table, T bits)
{
    return table[bits & (T(-1) >> (sizeof(T) * 8 - table.maxBits))];
}

HuffmanTable invertTableBits(const HuffmanTable& table)
{
    HuffmanTable newTable = table;
    for (uint16_t x = 0; x < table.size(); x++)
    {
        newTable[reverseBits(x, table.maxBits)] = table[x];
    }
    return newTable;
}

template <typename T = std::uint8_t>
struct BitStream
{
    struct Offset
    {
        std::size_t byteOffset{};
        std::uint8_t bitOffset{};
    };

    std::span<std::uint8_t> data{};
    Offset offset{};

    void checkPosition() const
    {
        if (offset.byteOffset >= data.size())
        {
            throw std::runtime_error("out of range");
        }
    }

    void roundPosition()
    {
        if (offset.bitOffset > 0)
        {
            offset.bitOffset = 0;
            offset.byteOffset++;
        }
    }

    template <typename I = std::uint8_t>
    I readBits(std::uint8_t count)
    {
        if (count > sizeof(I) * 8)
        {
            throw std::runtime_error("invalid count for the size");
        }

        I out{};

        uint8_t shift{};

        if (offset.byteOffset >= data.size())
        {
            return out;
        }

        if (offset.bitOffset != 0)
        {
            const std::uint8_t byte = data[offset.byteOffset] >> offset.bitOffset;

            const std::uint8_t available = 8 - offset.bitOffset;
            const std::uint8_t toRead = std::min(available, count);

            out |= (byte & (0xFF >> (8 - toRead)));
            count -= toRead;

            offset.bitOffset += toRead;
            shift += toRead;

            if (offset.bitOffset > 8)
            {
                throw std::runtime_error("Logic error");
            }
            else if (offset.bitOffset == 8)
            {
                offset.bitOffset = 0;
                offset.byteOffset++;
            }
        }

        while (count >= 8)
        {
            if (offset.byteOffset >= data.size())
            {
                return out;
            }

            out |= (I(data[offset.byteOffset++]) << shift);
            shift += 8;
            count -= 8;
        }

        if (count > 0 && offset.byteOffset < data.size())
        {
            I byte = data[offset.byteOffset];

            out |= (byte & (0xFF >> (8 - count))) << shift;

            offset.bitOffset = count;
        }

        return out;
    }

    template <typename I = std::uint8_t>
    I readBitsReversed(std::uint8_t count)
    {
        if (count > sizeof(I) * 8)
        {
            throw std::runtime_error("invalid count for the size");
        }

        I out{};

        if (offset.bitOffset != 0)
        {
            const std::uint8_t byte = data[offset.byteOffset] << offset.bitOffset;

            const std::uint8_t available = 8 - offset.bitOffset;
            const std::uint8_t toRead = std::min(available, count);

            out |= (byte >> (8 - toRead));
            count -= toRead;

            offset.bitOffset += toRead;

            if (offset.bitOffset > 8)
            {
                throw std::runtime_error("Logic error");
            }
            else if (offset.bitOffset == 8)
            {
                offset.bitOffset = 0;
                offset.byteOffset++;
            }
        }

        while (count >= 8)
        {
            out <<= 8;
            out |= data[offset.byteOffset++];
            count -= 8;
        }

        if (count > 0)
        {
            auto byte = data[offset.byteOffset];

            out |= byte >> (8 - count);
            offset.bitOffset = count;
        }

        return out;
    }

    uint16_t readHuffmanCode(const HuffmanTable& table)
    {
        const auto originalOffset = offset;

        const auto bits = readBits<uint16_t>(table.maxBits);

        const auto code = decodeCode(table, bits);

        offset = originalOffset;
        offset.bitOffset += code.bits;

        while (offset.bitOffset >= 8)
        {
            offset.bitOffset -= 8;
            offset.byteOffset++;
        }

        return code.value;
    }
};

namespace Alphabet
{
struct Entry
{
    uint8_t extraBits;
    uint16_t baseLength;
};

static constexpr auto LengthOffest = 257;
static constexpr std::array<Entry, 29> Length{
    Entry{0, 3},   Entry{0, 4},   Entry{0, 5},   Entry{0, 6},   Entry{0, 7},   Entry{0, 8},
    Entry{0, 9},   Entry{0, 10},  Entry{1, 11},  Entry{1, 13},  Entry{1, 15},  Entry{1, 17},
    Entry{2, 19},  Entry{2, 23},  Entry{2, 27},  Entry{2, 31},  Entry{3, 35},  Entry{3, 43},
    Entry{3, 51},  Entry{3, 59},  Entry{4, 67},  Entry{4, 83},  Entry{4, 99},  Entry{4, 115},
    Entry{5, 131}, Entry{5, 163}, Entry{5, 195}, Entry{5, 227}, Entry{0, 258},
};

static constexpr std::array<Entry, 30> Distance{
    Entry{0, 1},     Entry{0, 2},     Entry{0, 3},     Entry{0, 4},      Entry{1, 5},      Entry{1, 7},
    Entry{2, 9},     Entry{2, 13},    Entry{3, 17},    Entry{3, 25},     Entry{4, 33},     Entry{4, 49},
    Entry{5, 65},    Entry{5, 97},    Entry{6, 129},   Entry{6, 193},    Entry{7, 257},    Entry{7, 385},
    Entry{8, 513},   Entry{8, 769},   Entry{9, 1025},  Entry{9, 1537},   Entry{10, 2049},  Entry{10, 3073},
    Entry{11, 4097}, Entry{11, 6145}, Entry{12, 8193}, Entry{12, 12289}, Entry{13, 16385}, Entry{13, 24577},
};
} // namespace Alphabet

static const HuffmanTable staticLengthTable = []()
{
    std::vector<std::uint8_t> lengths;
    lengths.insert(lengths.end(), 144, 8);
    lengths.insert(lengths.end(), 112, 9);
    lengths.insert(lengths.end(), 24, 7);
    lengths.insert(lengths.end(), 8, 8);
    return invertTableBits(HuffmanTable::makeTable(lengths));
}();

static const HuffmanTable staticDistanceTable = []()
{
    std::vector<std::uint8_t> lengths;
    lengths.insert(lengths.end(), 32, 5);
    return invertTableBits(HuffmanTable::makeTable(lengths));
}();

std::optional<std::vector<std::uint8_t>> inflate(std::span<std::uint8_t> input)
{
    BitStream<std::uint8_t> stream{input};

    const auto CM = stream.readBits(4);
    const auto CINFO = stream.readBits(4);

    const auto CMF = CINFO << 4 | CM;

    if (CM != 8)
    {
        std::cerr << "Unsupported compression method" << std::endl;
        return std::nullopt;
    }

    if (CINFO > 7)
    {
        std::cerr << "Invalid window size" << std::endl;
        return std::nullopt;
    }

    const auto FCHECK = stream.readBits(5);
    const auto FDICT = stream.readBits(1);
    const auto FLEVEL = stream.readBits(2);

    const auto FLG = (FLEVEL << 6) | (FDICT << 5) | FCHECK;

    if (FDICT)
    {
        std::cerr << "Dictionaries not supported" << std::endl;
        return std::nullopt;
    }

    const auto checkValue = ((std::uint16_t)CMF << 8) + FLG;

    if (checkValue % 31 != 0)
    {
        std::cerr << "FCHECK fail" << std::endl;
        return std::nullopt;
    }

    std::vector<uint8_t> outputData;

    // read blocks
    while (true)
    {
        const auto BFINAL = stream.readBits(1);
        const auto BTYPE = stream.readBits(2);

        // Raw data
        if (BTYPE == 0)
        {
            stream.roundPosition();
            const auto LEN = stream.readBits<uint16_t>(16);
            const auto NLEN = stream.readBits<uint16_t>(16);

            if (LEN != (uint16_t)~NLEN)
            {
                std::cerr << "invalid raw block length" << std::endl;
                return std::nullopt;
            }

            outputData.append_range(stream.data.subspan(stream.offset.byteOffset, LEN));
            stream.offset.byteOffset += LEN;
        }
        else
        {
            const HuffmanTable* lengthTable = &staticLengthTable;
            const HuffmanTable* distanceTable = &staticDistanceTable;

            HuffmanTable dynamicLengthTable;
            HuffmanTable dynamicDistanceTable;

            // Dynamic huffman
            if (BTYPE == 2)
            {
                const auto HLIT = stream.readBits(5) + 257u;
                const auto HDIST = stream.readBits(5) + 1u;
                const auto HCLEN = stream.readBits(4) + 4u;

                static constexpr std::array<uint8_t, 19>
                    permutations{16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

                std::vector<std::uint8_t> codeLenght(19);
                for (int x = 0; x < HCLEN; x++)
                {
                    codeLenght[permutations[x]] = stream.readBits(3);
                }

                auto codeTable = HuffmanTable::makeTable(codeLenght);
                codeTable = invertTableBits(codeTable);

                const auto codeDecode = [&](auto count)
                {
                    std::vector<std::uint8_t> vector;
                    vector.reserve(count);

                    while (vector.size() < count)
                    {
                        const auto code = stream.readHuffmanCode(codeTable);
                        if (code >= 0 && code <= 15)
                        {
                            vector.push_back(code);
                        }
                        else if (code == 16)
                        {
                            const auto repeatLength = stream.readBits<uint8_t>(2) + 3;
                            vector.insert(vector.end(), repeatLength, vector.back());
                        }
                        else if (code == 17)
                        {
                            const auto repeatLength = stream.readBits<uint8_t>(3) + 3;
                            vector.insert(vector.end(), repeatLength, 0);
                        }
                        else if (code == 18)
                        {
                            const auto repeatLength = stream.readBits<uint8_t>(7) + 11;
                            vector.insert(vector.end(), repeatLength, 0);
                        }
                    }

                    return vector;
                };

                const auto lengths = codeDecode(HLIT + HDIST);

                dynamicLengthTable = HuffmanTable::makeTable({lengths.begin(), HLIT});
                dynamicDistanceTable = HuffmanTable::makeTable({lengths.begin() + HLIT, HDIST});

                dynamicLengthTable = invertTableBits(dynamicLengthTable);
                dynamicDistanceTable = invertTableBits(dynamicDistanceTable);

                lengthTable = &dynamicLengthTable;
                distanceTable = &dynamicDistanceTable;
            }

            while (true)
            {
                const auto code = stream.readHuffmanCode(*lengthTable);
                if (code >= 0 && code <= 255)
                {
                    outputData.push_back(code);
                }
                else if (code == 256)
                {
                    break;
                }
                else
                {
                    const auto lengthEntry = Alphabet::Length[code - Alphabet::LengthOffest];
                    const auto length = lengthEntry.baseLength + stream.readBits<std::uint16_t>(lengthEntry.extraBits);

                    const auto distanceCode = stream.readHuffmanCode(*distanceTable);
                    const auto distanceEntry = Alphabet::Distance[distanceCode];
                    const auto distance = distanceEntry.baseLength + stream.readBits<std::uint16_t>(distanceEntry.extraBits);

                    outputData.resize(outputData.size() + length);

                    auto dst = outputData.data() + outputData.size() - length;
                    auto src = dst - distance;

                    for (int x = 0; x < length; x++)
                    {
                        *(dst++) = *(src++);
                    }
                }
            }
        }

        if (BFINAL)
        {
            break;
        }
    }

    return outputData;
}

} // namespace deflate