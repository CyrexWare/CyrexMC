#pragma once

#include "network/mcbe/resourcepacks/resource_pack_def.hpp"
#include "util/uuid.hpp"

#include <memory>
#include <unordered_map>
#include <uuid.h>
#include <vector>

#include <cmath>
#include <cstdint>

namespace cyrex::network::protocol
{

// mabye just remove this, i dont see the point.
struct Chunk
{
    int index;
};

struct ResourcePackChunkState
{
    Chunk chunk;
    bool want;
    bool sent;
};

class ResourcePackMeta
{
public:
    uuid::UUID packId;
    resourcepacks::ResourcePackDef* pack;
    int maxChunkSize;
    int chunkCount;
    std::vector<ResourcePackChunkState> chunks;
    int nextToSend = 0;

    ResourcePackMeta(const uuid::UUID& id, resourcepacks::ResourcePackDef* p, const int maxChunk, const int chunksCount) :
        packId(id),
        pack(p),
        maxChunkSize(maxChunk),
        chunkCount(chunksCount)
    {
        chunks.reserve(static_cast<size_t>(chunkCount));
        for (int i = 0; i < chunkCount; ++i)
        {
            chunks.push_back(ResourcePackChunkState{Chunk{i}, false, false});
        }
    }

    bool finished() const
    {
        return nextToSend >= chunkCount;
    }
};

} // namespace cyrex::network::protocol
