#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet_def.hpp"
#include "network/mcbe/packetids.hpp"

#include <string>

namespace cyrex::nw::session
{
class NetworkSession;
}

namespace cyrex::nw::protocol
{

class ResourcePackChunkDataPacket final :
    public PacketImpl<ResourcePackChunkDataPacket, std::to_underlying(PacketId::ResourcePackChunkData), PacketDirection::Clientbound, true>
{
public:
    uuid::UUID packId{};
    std::string packVersion;
    int chunkIndex = 0;
    uint64_t progress = 0;
    std::string data;

    bool encodePayload(io::BinaryWriter& out) const override
    {
        out.writeString(uuid::uuidToString(packId));
        out.writeU32LE(chunkIndex);
        out.writeU64LE(progress);
        out.writeString(data);
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
