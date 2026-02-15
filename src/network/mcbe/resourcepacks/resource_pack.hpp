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
    mutable util::UUID id{};

    virtual bool verifyManifest() const
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

    util::UUID getPackId() const override
    {
        if (!id.is_nil())
            return id;

        const std::string uuidStr = manifest["header"]["uuid"].get<std::string>();

        auto parsed = uuids::uuid::from_string(uuidStr);
        if (!parsed.has_value())
            throw std::invalid_argument("Invalid UUID format");

        id = parsed.value();
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
        const util::UUID uuid = getPackId();
        const auto bytes = reinterpret_cast<const uint8_t*>(&uuid);

        std::size_t h = 0;
        for (size_t i = 0; i < 16; ++i)
            h = h * 31 + bytes[i];

        return h;
    }
};

} // namespace cyrex::nw::resourcepacks
