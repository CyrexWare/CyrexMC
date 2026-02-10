#pragma once
#include "resource_pack_def.hpp"

#include <nlohmann/json.hpp>
#include <stdexcept>

namespace cyrex::nw::resourcepacks
{

class ResourcePack : public ResourcePackDef
{
protected:
    nlohmann::json manifest;
    mutable io::UUID id{};

    bool verifyManifest() const
    {
        if (!manifest.contains("format_version") || !manifest.contains("header") || !manifest.contains("modules"))
        {
            return false;
        }
        const auto& header = manifest["header"];
        return header.contains("description") && header.contains("name") && header.contains("uuid") &&
               header.contains("version") && header["version"].size() == 3;
    }

public:
    std::string getPackName() const override
    {
        return manifest["header"]["name"].get<std::string>();
    }

    io::UUID getPackId() const override
    {
        if (std::all_of(id.begin(), id.end(), [](auto b) { return b == 0; }))
        {
            std::string idStr = manifest["header"]["uuid"].get<std::string>();
            for (int i = 0; i < 16; ++i)
            {
                id[i] = static_cast<uint8_t>(std::stoul(idStr.substr(i * 2, 2), nullptr, 16));
            }
        }
        return id;
    }

    std::string getPackVersion() const override
    {
        const auto& v = manifest["header"]["version"];
        return std::to_string(v[0].get<int>()) + "." + std::to_string(v[1].get<int>()) + "." +
               std::to_string(v[2].get<int>());
    }

    std::size_t hashCode() const
    {
        std::size_t h = 0;
        for (auto b : getPackId())
            h = h * 31 + b;
        return h;
    }
};

} // namespace cyrex::nw::resourcepacks
