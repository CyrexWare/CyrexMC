#include "zipped_resource_pack.hpp"

#include "libdeflate.h"
#include "log/logging.hpp"
#include "util/zip.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <vector>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/sha256.h>

#include <cstdint>
#include <cstring>

namespace cyrex::network::resourcepacks
{

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
    std::pair<std::vector<uint8_t>, ::util::zip::ZipEntry> result = ::util::zip::extractFile(path, "manifest.json");
    std::vector<uint8_t>& compressed = result.first;
    ::util::zip::ZipEntry& entry = result.second;

    std::vector<uint8_t> out;
    if (entry.comp_method == 0)
    {
        out = std::move(compressed);
    }
    else if (entry.comp_method == 8)
    {
        struct libdeflate_decompressor* dec = libdeflate_alloc_decompressor();
        if (!dec)
            throw std::runtime_error("libdeflate_alloc_decompressor failed");
        out.resize(entry.usize);
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

} // namespace cyrex::network::resourcepacks
