#include "zipped_resource_pack.hpp"

#include "log/logging.hpp"
#include "miniz.h"

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/sha256.h>

#ifdef min
#undef min
#endif

namespace cyrex::nw::resourcepacks
{

ZippedResourcePack::ZippedResourcePack(const std::string& path) : filePath(path)
{
    if (!std::filesystem::exists(path))
        throw std::invalid_argument("File not found: " + path);

    loadZip(path);

    if (!verifyManifest())
        throw std::runtime_error("Invalid resource pack manifest");

    fileStream.open(path, std::ios::binary);
    if (!fileStream)
        throw std::runtime_error("Failed to open file stream for chunk reading");
}

ZippedResourcePack::~ZippedResourcePack()
{
    if (fileStream.is_open())
        fileStream.close();
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
        std::streamsize bytesRead = file.gcount();
        if (bytesRead > 0)
        {
            if (wc_Sha256Update(&ctx, buffer.data(), static_cast<uint32_t>(bytesRead)) != 0)
                throw std::runtime_error("SHA256 update failed");
        }
    }

    if (wc_Sha256Final(&ctx, hash.data()) != 0)
        throw std::runtime_error("SHA256 final failed");

    return hash;
}

std::vector<uint8_t> ZippedResourcePack::getPackChunk(uint64_t off, uint64_t len)
{
    if (len == 0)
        throw std::invalid_argument("Chunk length must be positive");

    if (!fileStream.is_open())
        throw std::runtime_error("File stream is not open");

    uint64_t fileSize = getPackSize();

    if (off >= fileSize)
        throw std::out_of_range("Offset is out of file bounds");

    uint64_t chunkLen = std::min(len, fileSize - off);

    std::vector<uint8_t> chunk(static_cast<size_t>(chunkLen));

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

cyrex::util::UUID ZippedResourcePack::getPackId() const
{
    const auto* bytes = reinterpret_cast<const uint8_t*>(&id);
    bool isEmpty = std::all_of(bytes, bytes + 16, [](uint8_t b) { return b == 0; });

    if (isEmpty)
    {
        std::string uuidStr = manifest["header"]["uuid"].get<std::string>();
        std::string cleanStr;
        cleanStr.reserve(32);

        for (char c : uuidStr)
            if (c != '-')
                cleanStr += c;

        std::array<uint8_t, 16> tmp{};
        for (size_t i = 0; i < 16; ++i)
            tmp[i] = static_cast<uint8_t>(std::stoi(cleanStr.substr(i * 2, 2), nullptr, 16));

        id = std::bit_cast<cyrex::util::UUID>(tmp);
    }

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
