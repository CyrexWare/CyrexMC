#pragma once
#include "network/mcbe/resourcepacks/resource_pack_def.hpp"

#include <memory>
#include <vector>

namespace cyrex::nw::resourcepacks
{

class ResourcePackLoaderDef
{
public:
    virtual ~ResourcePackLoaderDef() = default;

    virtual std::vector<std::shared_ptr<ResourcePackDef>> loadPacks() = 0;
};

} // namespace cyrex::nw::resourcepacks
