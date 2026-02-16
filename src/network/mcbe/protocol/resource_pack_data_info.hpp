#pragma once
#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackDataTypes.hpp"
#include "network/mcbe/resourcepacks/resource_pack.hpp"
#include "network/session/network_session.hpp"

#include <string>
#include <vector>

namespace cyrex::nw::protocol
{

class ResourcePackDataInfoPacket final :
    public PacketImpl<ResourcePackDataInfoPacket, std::to_underlying(PacketId::ResourcePackDataInfo), PacketDirection::Clientbound, true>
{
public:
    using PacketImpl::getDefStatic;

    uuid::UUID packId{};
    std::string packVersion;
    int maxChunkSize = 0;
    int chunkCount = 0;
    uint64_t compressedPackSize = 0;
    std::vector<uint8_t> sha256;
    bool premium = false;
    ResourcePackDataType type = ResourcePackDataType::Resource;

    bool encodePayload(io::BinaryWriter& out) const override
    {
        out.writeString(uuid::uuidToString(packId));
        out.writeI32LE(maxChunkSize);
        out.writeI32LE(chunkCount);
        out.writeU64LE(compressedPackSize);

        out.writeVarUInt(static_cast<uint32_t>(sha256.size()));
        out.writeBytes(sha256);

        out.writeBool(premium);
        out.writeU8(static_cast<uint8_t>(type));
        return true;
    }

    bool decodePayload(io::BinaryReader& in) override
    {
        // NOOP
        return true;
    }

    bool handle(session::NetworkSession& session) override
    {
        return true;
    }
};

} // namespace cyrex::nw::protocol
