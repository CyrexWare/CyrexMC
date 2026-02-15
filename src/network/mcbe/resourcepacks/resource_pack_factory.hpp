#pragma once
#include "loader/resource_pack_loader_def.hpp"
#include "resource_pack_def.hpp"
#include "util/uuid.hpp"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cyrex::nw::resourcepacks
{

class ResourcePackFactory
{
public:
    explicit ResourcePackFactory(const std::unordered_set<std::shared_ptr<ResourcePackLoaderDef>>& loadersSet);

    void reloadPacks();
    std::vector<std::shared_ptr<ResourcePackDef>> getResourceStack() const;
    std::shared_ptr<ResourcePackDef> getPackById(const util::UUID& id) const;
    int getMaxChunkSize() const;
    void setMaxChunkSize(int size);
    void registerPackLoader(std::shared_ptr<ResourcePackLoaderDef> loader);

private:
    int maxChunkSize = 1024 * 256;
    std::unordered_map<util::UUID, std::shared_ptr<ResourcePackDef>, std::hash<util::UUID>> packsById;
    std::unordered_set<std::shared_ptr<ResourcePackDef>> packs;
    std::unordered_set<std::shared_ptr<ResourcePackLoaderDef>> loaders;
};

} // namespace cyrex::nw::resourcepacks
