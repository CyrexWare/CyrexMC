#include "zipped_resource_pack.hpp"

#include "libdeflate.h"
#include "log/logging.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/sha256.h>

#include <cstdint>
#include <cstring>

namespace cyrex::nw::resourcepacks
{

static uint16_t le16(const uint8_t* p)
{
    return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}
static uint32_t le32(const uint8_t* p)
{
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

ZippedResourcePack::ZippedResourcePack(const std::string& path) : filePath(path)
{
    if (!std::filesystem::exists(path))
        throw std::invalid_argument("File not found: " + path);

    loadZip(path);

    if (!ZippedResourcePack::verifyManifest())
        throw std::runtime_error("Invalid resource pack manifest");

    fileStream.open(path, std::ios::binary);
    if (!fileStream)
        throw std::runtime_error("Failed to open file stream for chunk reading");
}

ZippedResourcePack::~ZippedResourcePack()
{
    fileStream.close();
}

void ZippedResourcePack::loadZip(const std::string& path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
        throw std::runtime_error("Failed to open zip file: " + path);

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

    ssize_t eocd_pos = -1;
    for (ssize_t i = static_cast<ssize_t>(searchSize) - 22; i >= 0; --i)
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
    uint16_t disk_no = le16(eocd + 4);
    (void)disk_no;
    uint16_t start_disk = le16(eocd + 6);
    (void)start_disk;
    uint16_t entries_on_disk = le16(eocd + 8);
    uint16_t total_entries = le16(eocd + 10);
    uint32_t central_dir_size = le32(eocd + 12);
    uint32_t central_dir_offset = le32(eocd + 16);

    if (central_dir_offset + central_dir_size > static_cast<uint32_t>(fileSize))
        throw std::runtime_error("Central directory extends past end of file");

    ifs.seekg(static_cast<std::streamoff>(central_dir_offset), std::ios::beg);
    std::vector<uint8_t> central(central_dir_size);
    ifs.read(reinterpret_cast<char*>(central.data()), static_cast<std::streamsize>(central_dir_size));
    if (ifs.gcount() != static_cast<std::streamsize>(central_dir_size))
        throw std::runtime_error("Failed reading central directory");

    bool found = false;
    uint32_t cd_idx = 0;
    uint32_t found_crc = 0;
    uint32_t found_csize = 0;
    uint32_t found_usize = 0;
    uint32_t found_lh_offset = 0;
    uint16_t found_filename_len = 0;
    std::vector<uint8_t> file_name_buf;

    while (cd_idx + 4 <= central_dir_size)
    {
        const uint8_t* p = central.data() + cd_idx;
        uint32_t sig = le32(p);
        if (sig != 0x02014b50)
            break;
        uint16_t name_len = le16(p + 28);
        uint16_t extra_len = le16(p + 30);
        uint16_t comment_len = le16(p + 32);
        uint32_t crc32_val = le32(p + 16);
        uint32_t csize = le32(p + 20);
        uint32_t usize = le32(p + 24);
        uint32_t local_header_offset = le32(p + 42);
        const uint8_t* name_ptr = p + 46;
        if (name_len == 0 || (name_ptr + name_len) > (central.data() + central_dir_size))
            break;
        std::string name(reinterpret_cast<const char*>(name_ptr), name_len);
        if (name == "manifest.json")
        {
            found = true;
            found_crc = crc32_val;
            found_csize = csize;
            found_usize = usize;
            found_lh_offset = local_header_offset;
            found_filename_len = name_len;
            file_name_buf.assign(name_ptr, name_ptr + name_len);
            break;
        }
        cd_idx += 46 + name_len + extra_len + comment_len;
    }

    if (!found)
        throw std::runtime_error("manifest.json not found in zip");

    if (found_lh_offset + 30 > static_cast<uint32_t>(fileSize))
        throw std::runtime_error("Local header extends past end of file");

    ifs.seekg(static_cast<std::streamoff>(found_lh_offset), std::ios::beg);
    std::vector<uint8_t> local_header(30);
    ifs.read(reinterpret_cast<char*>(local_header.data()), 30);
    if (ifs.gcount() != 30)
        throw std::runtime_error("Failed reading local header");

    uint32_t lh_sig = le32(local_header.data());
    if (lh_sig != 0x04034b50)
        throw std::runtime_error("Invalid local header signature");

    uint16_t lh_name_len = le16(local_header.data() + 26);
    uint16_t lh_extra_len = le16(local_header.data() + 28);
    uint64_t data_offset = static_cast<uint64_t>(found_lh_offset) + 30 + lh_name_len + lh_extra_len;
    if (data_offset + found_csize > static_cast<uint64_t>(fileSize))
        throw std::runtime_error("Compressed data extends past end of file");

    ifs.seekg(static_cast<std::streamoff>(data_offset), std::ios::beg);
    std::vector<uint8_t> compressed(found_csize);
    ifs.read(reinterpret_cast<char*>(compressed.data()), static_cast<std::streamsize>(found_csize));
    if (ifs.gcount() != static_cast<std::streamsize>(found_csize))
        throw std::runtime_error("Failed reading compressed manifest data");

    uint16_t gp_flags = le16(local_header.data() + 6);
    (void)gp_flags;
    uint16_t comp_method = le16(local_header.data() + 8);

    std::vector<uint8_t> out;
    if (comp_method == 0)
    {
        out = std::move(compressed);
    }
    else if (comp_method == 8)
    {
        struct libdeflate_decompressor* dec = libdeflate_alloc_decompressor();
        if (!dec)
            throw std::runtime_error("libdeflate_alloc_decompressor failed");
        out.resize(found_usize);
        size_t actual_out = 0;
        enum libdeflate_result
            r = libdeflate_deflate_decompress(dec, compressed.data(), compressed.size(), out.data(), out.size(), &actual_out);
        libdeflate_free_decompressor(dec);
        if (r != LIBDEFLATE_SUCCESS)
            throw std::runtime_error("libdeflate decompression failed");
        if (actual_out != out.size())
            out.resize(actual_out);
    }
    else
    {
        throw std::runtime_error("Unsupported compression method in zip");
    }

    manifest = nlohmann::json::parse(reinterpret_cast<const char*>(out.data()),
                                     reinterpret_cast<const char*>(out.data() + out.size()));
}

std::vector<uint8_t> ZippedResourcePack::computeSha256FromFile() const
{
    wc_Sha256 ctx;
    std::vector<uint8_t> hash(SHA256_DIGEST_SIZE);

    if (wc_InitSha256(&ctx) != 0)
        throw std::runtime_error("Failed to initialize SHA256 context");

    std::ifstream file(filePath, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open file for SHA256 computation");

    constexpr size_t bufferSize = 64 * 1024;
    std::vector<uint8_t> buffer(bufferSize);

    while (file)
    {
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        if (std::streamsize bytesRead = file.gcount(); bytesRead > 0)
        {
            if (wc_Sha256Update(&ctx, buffer.data(), static_cast<uint32_t>(bytesRead)) != 0)
                throw std::runtime_error("SHA256 update failed");
        }
    }

    if (wc_Sha256Final(&ctx, hash.data()) != 0)
        throw std::runtime_error("SHA256 final failed");

    return hash;
}

std::vector<uint8_t> ZippedResourcePack::getPackChunk(const uint64_t off, const uint64_t len)
{
    if (len == 0)
        throw std::invalid_argument("Chunk length must be positive");

    if (!fileStream.is_open())
        throw std::runtime_error("File stream is not open");

    const uint64_t fileSize = getPackSize();

    if (off >= fileSize)
        throw std::out_of_range("Offset is out of file bounds");

    const uint64_t chunkLen = std::min(len, fileSize - off);

    std::vector<uint8_t> chunk(chunkLen);

    fileStream.seekg(static_cast<std::streamoff>(off), std::ios::beg);
    fileStream.read(reinterpret_cast<char*>(chunk.data()), static_cast<std::streamsize>(chunkLen));

    if (static_cast<uint64_t>(fileStream.gcount()) != chunkLen)
        throw std::runtime_error("Failed to read full chunk from file");

    return chunk;
}

std::string ZippedResourcePack::getPackChunkString(std::streamoff start, std::size_t length) const
{
    if (length < 1)
    {
        throw std::invalid_argument("Pack length must be positive");
    }

    fileStream.seekg(start, std::ios::beg);
    if (!fileStream)
    {
        throw std::invalid_argument("Requested a resource pack chunk with invalid start offset");
    }

    std::string buffer(length, '\0');
    fileStream.read(buffer.data(), length);

    if (fileStream.gcount() != static_cast<std::streamsize>(length))
    {
        buffer.resize(fileStream.gcount());
    }

    return buffer;
}

std::string ZippedResourcePack::getPackName() const
{
    return manifest["header"]["name"].get<std::string>();
}

uuid::UUID ZippedResourcePack::getPackId() const
{
    if (!id.is_nil())
        return id;

    const std::string uuidStr = manifest["header"]["uuid"].get<std::string>();

    const auto parsed = uuids::uuid::from_string(uuidStr);
    if (!parsed.has_value())
        throw std::invalid_argument("Invalid UUID format");
    id = parsed.value();
    return id;
}

std::string ZippedResourcePack::getPackVersion() const
{
    auto versionArray = manifest["header"]["version"];
    return std::to_string(versionArray[0].get<int>()) + "." + std::to_string(versionArray[1].get<int>()) + "." +
           std::to_string(versionArray[2].get<int>());
}

uint64_t ZippedResourcePack::getPackSize() const
{
    return static_cast<uint64_t>(std::filesystem::file_size(filePath));
}

std::vector<uint8_t> ZippedResourcePack::getSha256() const
{
    if (sha256Hash.empty())
        sha256Hash = computeSha256FromFile();
    return sha256Hash;
}

bool ZippedResourcePack::verifyManifest() const
{
    return manifest.contains("format_version") && manifest.contains("header") && manifest.contains("modules");
}

} // namespace cyrex::nw::resourcepacks
