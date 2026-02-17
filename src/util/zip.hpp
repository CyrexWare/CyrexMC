#pragma once

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <cstdint>

namespace util
{
namespace zip
{

inline uint16_t le16(const uint8_t* p)
{
    return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}

inline uint32_t le32(const uint8_t* p)
{
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

struct ZipEntry
{
    uint32_t crc32;
    uint32_t csize;
    uint32_t usize;
    uint32_t local_header_offset;
    uint16_t comp_method;
    uint16_t gp_flags;
    std::string name;
};

inline std::pair<std::vector<uint8_t>, ZipEntry> extractFile(const std::string& zipPath, const std::string& filename)
{
    std::ifstream ifs(zipPath, std::ios::binary);
    if (!ifs)
        throw std::runtime_error("Failed to open zip file: " + zipPath);

    ifs.seekg(0, std::ios::end);
    std::streamoff fileSize = ifs.tellg();
    if (fileSize < 22)
        throw std::runtime_error("Not a valid zip (file too small)");

    size_t searchSize = static_cast<size_t>(std::min<std::streamoff>(fileSize, 65557));
    ifs.seekg(fileSize - static_cast<std::streamoff>(searchSize), std::ios::beg);
    std::vector<uint8_t> tail(searchSize);
    ifs.read(reinterpret_cast<char*>(tail.data()), static_cast<std::streamsize>(searchSize));
    if (ifs.gcount() != static_cast<std::streamsize>(searchSize))
        throw std::runtime_error("Failed reading zip tail");

    ptrdiff_t eocd_pos = -1;
    for (ptrdiff_t i = static_cast<ptrdiff_t>(searchSize) - 22; i >= 0; --i)
    {
        if (tail[i] == 0x50 && tail[i + 1] == 0x4b && tail[i + 2] == 0x05 && tail[i + 3] == 0x06)
        {
            eocd_pos = i;
            break;
        }
    }
    if (eocd_pos < 0)
        throw std::runtime_error("End of central directory not found");

    const uint8_t* eocd = tail.data() + eocd_pos;
    uint32_t central_dir_size = le32(eocd + 12);
    uint32_t central_dir_offset = le32(eocd + 16);

    if (central_dir_offset + central_dir_size > static_cast<uint32_t>(fileSize))
        throw std::runtime_error("Central directory extends past end of file");

    ifs.seekg(static_cast<std::streamoff>(central_dir_offset), std::ios::beg);
    std::vector<uint8_t> central(central_dir_size);
    ifs.read(reinterpret_cast<char*>(central.data()), static_cast<std::streamsize>(central_dir_size));
    if (ifs.gcount() != static_cast<std::streamsize>(central_dir_size))
        throw std::runtime_error("Failed reading central directory");

    ZipEntry entry{};
    bool found = false;
    size_t cd_idx = 0;

    while (cd_idx + 4 <= central_dir_size)
    {
        const uint8_t* p = central.data() + cd_idx;
        uint32_t sig = le32(p);
        if (sig != 0x02014b50)
            break;

        uint16_t name_len = le16(p + 28);
        uint16_t extra_len = le16(p + 30);
        uint16_t comment_len = le16(p + 32);
        uint16_t comp_method = le16(p + 10);
        uint16_t gp_flags = le16(p + 8);
        uint32_t crc32_val = le32(p + 16);
        uint32_t csize = le32(p + 20);
        uint32_t usize = le32(p + 24);
        uint32_t local_header_offset = le32(p + 42);

        const uint8_t* name_ptr = p + 46;
        if (name_ptr + name_len > central.data() + central_dir_size)
            break;

        std::string name(reinterpret_cast<const char*>(name_ptr), name_len);
        if (name == filename)
        {
            found = true;
            entry.crc32 = crc32_val;
            entry.csize = csize;
            entry.usize = usize;
            entry.local_header_offset = local_header_offset;
            entry.comp_method = comp_method;
            entry.gp_flags = gp_flags;
            entry.name = name;
            break;
        }
        cd_idx += 46 + name_len + extra_len + comment_len;
    }

    if (!found)
        throw std::runtime_error(filename + " not found in zip");

    ifs.seekg(static_cast<std::streamoff>(entry.local_header_offset), std::ios::beg);
    std::vector<uint8_t> local_header(30);
    ifs.read(reinterpret_cast<char*>(local_header.data()), 30);
    if (ifs.gcount() != 30)
        throw std::runtime_error("Failed reading local header");

    uint32_t lh_sig = le32(local_header.data());
    if (lh_sig != 0x04034b50)
        throw std::runtime_error("Invalid local header signature");

    uint16_t lh_name_len = le16(local_header.data() + 26);
    uint16_t lh_extra_len = le16(local_header.data() + 28);

    uint64_t data_offset = static_cast<uint64_t>(entry.local_header_offset) + 30 + lh_name_len + lh_extra_len;
    if (data_offset + entry.csize > static_cast<uint64_t>(fileSize))
        throw std::runtime_error("Compressed data extends past end of file");

    ifs.seekg(static_cast<std::streamoff>(data_offset), std::ios::beg);
    std::vector<uint8_t> compressed(entry.csize);
    ifs.read(reinterpret_cast<char*>(compressed.data()), static_cast<std::streamsize>(entry.csize));
    if (ifs.gcount() != static_cast<std::streamsize>(entry.csize))
        throw std::runtime_error("Failed reading compressed data");

    return std::make_pair(std::move(compressed), entry);
}

} // namespace zip
} // namespace util
