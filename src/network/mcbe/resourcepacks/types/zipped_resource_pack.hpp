#pragma once
#include "network/io/binary_reader.hpp"
#include "network/mcbe/resourcepacks/resource_pack.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace cyrex::nw::resourcepacks
{

class ZippedResourcePack : public ResourcePack
{
public:
    explicit ZippedResourcePack(const std::string& path);
    ~ZippedResourcePack() override;

    std::string getPackName() const override;
    util::UUID getPackId() const override;
    std::string getPackVersion() const override;
    uint64_t getPackSize() const override;
    std::vector<uint8_t> getSha256() const override;
    std::vector<uint8_t> getPackChunk(uint64_t off, uint64_t len) override;
    std::string getPackChunkString(std::streamoff start, std::size_t length) const override;

private:
    std::string filePath;
    mutable std::vector<uint8_t> sha256Hash;
    nlohmann::json manifest;
    mutable util::UUID id{};
    mutable std::ifstream fileStream;

    void loadZip(const std::string& path);
    bool verifyManifest() const;
    std::vector<uint8_t> computeSha256FromFile() const;
};

} // namespace cyrex::nw::resourcepacks
