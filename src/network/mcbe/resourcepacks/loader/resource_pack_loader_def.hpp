#pragma once
#include "network/mcbe/resourcepacks/resource_pack_def.hpp"

#include <memory>
#include <vector>

namespace cyrex::network::resourcepacks
{

class ResourcePackLoaderDef
{
public:
    virtual ~ResourcePackLoaderDef() = default;

    virtual std::vector<std::unique_ptr<ResourcePackDef>> loadPacks() = 0;
};

} // namespace cyrex::network::resourcepacks
