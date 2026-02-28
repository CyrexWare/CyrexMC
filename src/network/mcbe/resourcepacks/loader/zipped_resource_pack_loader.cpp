#include "zipped_resource_pack_loader.hpp"

#include "log/logging.hpp"
#include "network/mcbe/resourcepacks/types/zipped_resource_pack.hpp"

#include <algorithm>
#include <filesystem>

namespace cyrex::network::resourcepacks
{

std::vector<std::unique_ptr<ResourcePackDef>> ZippedResourcePackLoader::loadPacks()
{
    std::vector<std::unique_ptr<ResourcePackDef>> loadedPacks;

    namespace fs = std::filesystem;

    if (!fs::exists(folderPath))
    {
        logging::info("Resource pack folder '{}' does not exist. Creating...", folderPath.string());
        fs::create_directories(folderPath);
        logging::info("No resource packs found yet. Folder created.");
        return loadedPacks;
    }

    if (!fs::is_directory(folderPath))
    {
        throw std::invalid_argument("Invalid resource pack path: " + folderPath.string());
    }

    bool anyPackFound = false;

    for (const auto& entry : fs::directory_iterator(folderPath))
    {
        if (!entry.is_regular_file())
            continue;

        auto path = entry.path();
        auto ext = path.extension().string();
        std::ranges::transform(ext, ext.begin(), ::tolower);

        if (ext == ".key")
            continue;

        try
        {
            std::unique_ptr<ResourcePackDef> pack;

            if (ext == ".zip" || ext == ".mcpack")
            {
                pack = std::make_unique<ZippedResourcePack>(path.string());
            }
            else
            {
                logging::warn("Skipping unsupported file type for resource pack '{}'", path.filename().string());
                continue;
            }

            loadedPacks.push_back(std::move(pack));
            anyPackFound = true;
            logging::info("Loaded resource pack '{}'", path.filename().string());

        } catch (const std::exception& e)
        {
            logging::error("Failed to load resource pack '{}': {}", path.filename().string(), e.what());
        }
    }

    if (!anyPackFound)
    {
        logging::info("No valid resource packs found in '{}'.", folderPath.string());
    }

    return loadedPacks;
}

} // namespace cyrex::network::resourcepacks
