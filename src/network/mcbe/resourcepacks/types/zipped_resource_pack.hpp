#pragma once
#include "network/io/binary_reader.hpp"
#include "network/mcbe/resourcepacks/resource_pack.hpp"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace cyrex::nw::resourcepacks
{

class ZippedResourcePack : public ResourcePack
{
public:
    explicit ZippedResourcePack(const std::string& path);

    std::string getPackName() const override;
    io::UUID getPackId() const override;
    std::string getPackVersion() const override;
    int getPackSize() const override;
    std::vector<uint8_t> getSha256() const override;
    std::vector<uint8_t> getPackChunk(int off, int len) override;

private:
    std::string filePath;
    std::vector<uint8_t> buffer;
    std::vector<uint8_t> sha256Hash;
    nlohmann::json manifest;
    mutable io::UUID id{};

    void loadZip(const std::string& path);
    bool verifyManifest() const;
};

} // namespace cyrex::nw::resourcepacks
