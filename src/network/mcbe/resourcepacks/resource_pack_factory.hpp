#pragma once
#include "network/mcbe/resourcepacks/loader/resource_pack_loader_def.hpp"
#include "resource_pack_def.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cyrex::nw::resourcepacks
{

class ResourcePackFactory
{
protected:
    int maxChunkSize = 1024 * 256;
    std::unordered_map<std::string, std::shared_ptr<ResourcePackDef>> packsById;
    std::unordered_set<std::shared_ptr<ResourcePackDef>> packs;
    std::unordered_set<std::shared_ptr<ResourcePackLoaderDef>> loaders;

    static std::string uuidToString(const io::UUID& u)
    {
        std::string str;
        for (auto b : u)
        {
            char buf[3];
            std::snprintf(buf, sizeof(buf), "%02x", b);
            str += buf;
        }
        return str;
    }

public:
    explicit ResourcePackFactory(const std::unordered_set<std::shared_ptr<ResourcePackLoaderDef>>& loadersSet) :
        loaders(loadersSet)
    {
        reloadPacks();
    }

    void reloadPacks()
    {
        packs.clear();
        packsById.clear();
        for (auto& loader : loaders)
        {
            for (auto& pack : loader->loadPacks())
            {
                packs.insert(pack);
                packsById[uuidToString(pack->getPackId())] = pack;
            }
        }
    }

    std::vector<std::shared_ptr<ResourcePackDef>> getResourceStack()
    {
        return {packs.begin(), packs.end()};
    }

    std::shared_ptr<ResourcePackDef> getPackById(const io::UUID& id)
    {
        auto it = packsById.find(uuidToString(id));
        return it != packsById.end() ? it->second : nullptr;
    }

    int getMaxChunkSize() const
    {
        return maxChunkSize;
    }

    void registerPackLoader(std::shared_ptr<ResourcePackLoaderDef> loader)
    {
        loaders.insert(loader);
    }
};

} // namespace cyrex::nw::resourcepacks
