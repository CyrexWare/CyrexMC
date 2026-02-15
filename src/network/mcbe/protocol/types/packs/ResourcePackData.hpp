#pragma once

#include "network/mcbe/resourcepacks/resource_pack_def.hpp"
#include "util/uuid.hpp"

#include <bitset>
#include <memory>
#include <unordered_map>
#include <uuid.h>

#include <cmath>
#include <cstdint>

namespace cyrex::nw::protocol
{

class ResourcePackData
{
public:
    util::UUID packId;
    std::shared_ptr<resourcepacks::ResourcePackDef> pack;
    int maxChunkSize;
    int chunkCount;
    std::vector<bool> want;
    std::vector<bool> sent;
    int nextToSend = 0;

    ResourcePackData(const util::UUID& id, const std::shared_ptr<resourcepacks::ResourcePackDef>& p, int maxChunk, int chunks) :
        packId(id),
        pack(p),
        maxChunkSize(maxChunk),
        chunkCount(chunks),
        want(static_cast<size_t>(chunks), false),
        sent(static_cast<size_t>(chunks), false)
    {
    }

    bool finished() const
    {
        return nextToSend >= chunkCount;
    }
};
} // namespace cyrex::nw::protocol