#include "resource_pack_factory.hpp"

#include <algorithm>
#include <stdexcept>

namespace cyrex::nw::resourcepacks
{

ResourcePackFactory::ResourcePackFactory(const std::unordered_set<ResourcePackLoaderDef*>& loadersSet) :
    loaders(loadersSet)
{
    reloadPacks();
}

void ResourcePackFactory::reloadPacks()
{
    packs.clear();
    packsById.clear();

    for (auto* loader : loaders)
    {
        auto loaded = loader->loadPacks();

        for (auto& packPtr : loaded)
        {
            packs.push_back(std::move(packPtr));
            packsById[packs.back()->getPackId()] = packs.back().get();
        }
    }
}

std::vector<ResourcePackDef*> ResourcePackFactory::getResourceStack() const
{
    std::vector<ResourcePackDef*> out;
    out.reserve(packs.size());

    for (auto& p : packs)
        out.push_back(p.get());

    return out;
}

ResourcePackDef* ResourcePackFactory::getPackById(const uuid::UUID& id) const
{
    auto it = packsById.find(id);
    return it != packsById.end() ? it->second : nullptr;
}

int ResourcePackFactory::getMaxChunkSize() const
{
    return maxChunkSize;
}

void ResourcePackFactory::setMaxChunkSize(const int size)
{
    if (size <= 0)
        throw std::invalid_argument("Max chunk size must be positive");
    maxChunkSize = size;
}

void ResourcePackFactory::registerPackLoader(ResourcePackLoaderDef* loader)
{
    loaders.insert(loader);
}

} // namespace cyrex::nw::resourcepacks
