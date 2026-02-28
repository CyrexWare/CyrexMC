#pragma once
#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"

namespace cyrex::network::protocol
{

class ResourcePackChunkRequestPacket final :
    public PacketImpl<ResourcePackChunkRequestPacket, std::to_underlying(PacketId::ResourcePackChunkRequest), PacketDirection::Serverbound, true>
{
public:
    uuid::UUID packId{};
    int chunkIndex = 0;

    bool encodePayload(io::BinaryWriter& out) const override
    {
        //NOOP
        return true;
    }

    bool decodePayload(io::BinaryReader& in) override
    {
        packId = uuid::fromString(in.readString());
        chunkIndex = in.readI16LE();
        return true;
    }

    bool handle(session::NetworkSession& session) override;
};

} // namespace cyrex::network::protocol
