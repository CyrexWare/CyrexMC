#pragma once
#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"

namespace cyrex::nw::protocol
{

class ResourcePackChunkRequestPacket final :
    public PacketImpl<ResourcePackChunkRequestPacket, std::to_underlying(PacketId::ResourcePackChunkRequest), PacketDirection::Serverbound, true>
{
public:
    uuid::UUID packId{};
    std::string packVersion;
    int chunkIndex = 0;

    bool encodePayload(io::BinaryWriter& out) const override
    {
        //NOOP
        return true;
    }

    bool decodePayload(io::BinaryReader& in) override
    {
        std::string packInfo = in.readString();
        // depending on certain settings this could be set, but with our current setup this isnt needed, mabye in the future.
        packVersion = "";
        packId = uuid::fromString(packInfo);
        chunkIndex = in.readI16LE();
        return true;
    }

    bool handle(session::NetworkSession& session);
};

} // namespace cyrex::nw::protocol
