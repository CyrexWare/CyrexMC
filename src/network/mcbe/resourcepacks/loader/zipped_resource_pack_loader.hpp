#pragma once
#include "network/mcbe/resourcepacks/types/zipped_resource_pack.hpp"
#include "resource_pack_loader_def.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace cyrex::nw::resourcepacks
{

class ZippedResourcePackLoader : public ResourcePackLoaderDef
{
protected:
    std::filesystem::path folderPath;

public:
    explicit ZippedResourcePackLoader(const std::filesystem::path& path) : folderPath(path)
    {
    }

    std::vector<std::shared_ptr<ResourcePackDef>> loadPacks() override;
};

} // namespace cyrex::nw::resourcepacks
