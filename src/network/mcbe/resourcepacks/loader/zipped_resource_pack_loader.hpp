#pragma once
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

    std::vector<std::unique_ptr<ResourcePackDef>> loadPacks() override;
};

} // namespace cyrex::nw::resourcepacks
