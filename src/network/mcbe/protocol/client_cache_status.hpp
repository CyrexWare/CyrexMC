#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"

namespace cyrex::network::protocol
{

class ClientCacheStatusPacket final :
    public PacketImpl<ClientCacheStatusPacket, std::to_underlying(PacketId::ClientCacheStatus), PacketDirection::Serverbound, false>
{
public:
    bool enabled = false;

    bool decodePayload(io::BinaryReader& in) override
    {
        enabled = in.readBool();
        return true;
    }

    bool encodePayload(io::BinaryWriter& out) const override
    {
        out.writeBool(enabled);
        return true;
    }

    bool handle(session::NetworkSession& session) override
    {
        return true;
    }
};

} // namespace cyrex::network::protocol
