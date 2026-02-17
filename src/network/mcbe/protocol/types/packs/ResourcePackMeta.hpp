#pragma once

#include "network/mcbe/resourcepacks/resource_pack_def.hpp"
#include "util/uuid.hpp"

#include <bitset>
#include <memory>
#include <unordered_map>
#include <uuid.h>

#include <cmath>
#include <cstdint>

namespace cyrex::network::protocol
{

class ResourcePackMeta
{
public:
    uuid::UUID packId;
    resourcepacks::ResourcePackDef* pack;
    int maxChunkSize;
    int chunkCount;
    std::vector<bool> want;
    std::vector<bool> sent;
    int nextToSend = 0;

    ResourcePackMeta(const uuid::UUID& id, resourcepacks::ResourcePackDef* p, const int maxChunk, const int chunks) :
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
} // namespace cyrex::network::protocol