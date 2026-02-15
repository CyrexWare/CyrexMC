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

    util::UUID getPackId() const override
    {
        const auto* bytes = reinterpret_cast<const uint8_t*>(&id);
        bool isEmpty = std::all_of(bytes, bytes + 16, [](uint8_t b) { return b == 0; });

        if (isEmpty)
        {
            std::string idStr = manifest["header"]["uuid"].get<std::string>();
            std::string cleanStr;
            cleanStr.reserve(32);

            for (char c : idStr)
                if (c != '-')
                    cleanStr += c;

            std::array<uint8_t, 16> tmp{};
            for (size_t i = 0; i < 16; ++i)
                tmp[i] = static_cast<uint8_t>(std::stoul(cleanStr.substr(i * 2, 2), nullptr, 16));

            id = std::bit_cast<util::UUID>(tmp);
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
        util::UUID uuid = getPackId();
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&uuid);

        std::size_t h = 0;
        for (size_t i = 0; i < 16; ++i)
            h = h * 31 + bytes[i];

        return h;
    }
};

} // namespace cyrex::nw::resourcepacks
