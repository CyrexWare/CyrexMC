#include "resource_pack_factory.hpp"

#include <algorithm>
#include <uuid.h>
#include <stdexcept>

namespace cyrex::nw::resourcepacks
{

ResourcePackFactory::ResourcePackFactory(const std::unordered_set<std::shared_ptr<ResourcePackLoaderDef>>& loadersSet) :
    loaders(loadersSet)
{
    reloadPacks();
}

void ResourcePackFactory::reloadPacks()
{
    packs.clear();
    packsById.clear();

    for (auto& loader : loaders)
    {
        for (auto& pack : loader->loadPacks())
        {
            packs.insert(pack);
            packsById[pack->getPackId()] = pack;
        }
    }
}

std::vector<std::shared_ptr<ResourcePackDef>> ResourcePackFactory::getResourceStack() const
{
    return {packs.begin(), packs.end()};
}

std::shared_ptr<ResourcePackDef> ResourcePackFactory::getPackById(const io::UUID& id) const
{
    auto it = packsById.find(id);
    return it != packsById.end() ? it->second : nullptr;
}

int ResourcePackFactory::getMaxChunkSize() const
{
    return maxChunkSize;
}

void ResourcePackFactory::setMaxChunkSize(int size)
{
    if (size <= 0)
        throw std::invalid_argument("Max chunk size must be positive");
    maxChunkSize = size;
}

void ResourcePackFactory::registerPackLoader(std::shared_ptr<ResourcePackLoaderDef> loader)
{
    loaders.insert(loader);
}

} // namespace cyrex::nw::resourcepacks
