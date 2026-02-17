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
    explicit ResourcePackFactory(const std::unordered_set<ResourcePackLoaderDef*>& loadersSet);

    void reloadPacks();
    std::vector<ResourcePackDef*> getResourceStack() const;
    ResourcePackDef* getPackById(const uuid::UUID& id) const;
    int getMaxChunkSize() const;
    void setMaxChunkSize(int size);
    void registerPackLoader(ResourcePackLoaderDef* loader);

private:
    int maxChunkSize = 1024 * 256;
    std::vector<std::unique_ptr<ResourcePackDef>> packs;
    std::unordered_map<uuid::UUID, ResourcePackDef*> packsById;
    std::unordered_set<ResourcePackLoaderDef*> loaders;
};

} // namespace cyrex::nw::resourcepacks
