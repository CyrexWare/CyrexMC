#include "zipped_resource_pack.hpp"

#include "miniz.h"

#include <fstream>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstdint>

#ifdef min
#undef min
#endif

namespace
{
std::vector<uint8_t> computeSha256(const std::vector<uint8_t>& data)
{
    wc_Sha256 ctx;
    std::vector<uint8_t> hash(SHA256_DIGEST_SIZE);

    if (wc_InitSha256(&ctx) != 0)
        throw std::runtime_error("Failed to initialize SHA256 context");

    if (wc_Sha256Update(&ctx, data.data(), static_cast<uint32_t>(data.size())) != 0)
        throw std::runtime_error("SHA256 update failed");

    if (wc_Sha256Final(&ctx, hash.data()) != 0)
        throw std::runtime_error("SHA256 final failed");

    return hash;
}
} // namespace

namespace cyrex::nw::resourcepacks
{

ZippedResourcePack::ZippedResourcePack(const std::string& path) : filePath(path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::invalid_argument("File not found: " + path);

    buffer = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), {});
    sha256Hash = computeSha256(buffer);

    loadZip(path);

    if (!verifyManifest())
        throw std::runtime_error("Invalid resource pack manifest");
}

void ZippedResourcePack::loadZip(const std::string& path)
{
    mz_zip_archive zip{};
    if (!mz_zip_reader_init_file(&zip, path.c_str(), 0))
        throw std::runtime_error("Failed to open zip file: " + path);

    int fileIndex = mz_zip_reader_locate_file(&zip, "manifest.json", nullptr, 0);
    if (fileIndex < 0)
    {
        mz_zip_reader_end(&zip);
        throw std::runtime_error("manifest.json not found in zip");
    }

    size_t size = 0;
    void* data = mz_zip_reader_extract_file_to_heap(&zip, "manifest.json", &size, 0);
    if (!data)
    {
        mz_zip_reader_end(&zip);
        throw std::runtime_error("Failed to read manifest.json from zip");
    }

    manifest = nlohmann::json::parse(static_cast<char*>(data), static_cast<char*>(data) + size);
    mz_free(data);
    mz_zip_reader_end(&zip);
}

std::vector<uint8_t> ZippedResourcePack::getPackChunk(int off, int len)
{
    if (off < 0 || off >= buffer.size())
        return {};
    int chunkLen = std::min(len, static_cast<int>(buffer.size()) - off);
    return std::vector<uint8_t>(buffer.begin() + off, buffer.begin() + off + chunkLen);
}

std::string ZippedResourcePack::getPackName() const
{
    return manifest["header"]["name"].get<std::string>();
}

cyrex::nw::io::UUID ZippedResourcePack::getPackId() const
{
    if (id == io::UUID{})
    {
        std::string uuidStr = manifest["header"]["uuid"];
        for (int i = 0; i < 16; ++i)
            id[i] = static_cast<uint8_t>(std::stoi(uuidStr.substr(i * 2, 2), nullptr, 16));
    }
    return id;
}

std::string ZippedResourcePack::getPackVersion() const
{
    auto versionArray = manifest["header"]["version"];
    return std::to_string(versionArray[0].get<int>()) + "." + std::to_string(versionArray[1].get<int>()) + "." +
           std::to_string(versionArray[2].get<int>());
}

int ZippedResourcePack::getPackSize() const
{
    return static_cast<int>(buffer.size());
}

std::vector<uint8_t> ZippedResourcePack::getSha256() const
{
    return sha256Hash;
}

bool ZippedResourcePack::verifyManifest() const
{
    return manifest.contains("format_version") && manifest.contains("header") && manifest.contains("modules");
}

} // namespace cyrex::nw::resourcepacks
