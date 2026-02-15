#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"

namespace cyrex::nw::protocol
{

class ClientCacheStatusPacket final :
    public PacketImpl<ClientCacheStatusPacket, static_cast<uint32_t>(PacketId::ClientCacheStatus), PacketDirection::Serverbound, false>
{
public:
    bool enabled = false;

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        enabled = in.readBool();
        return true;
    }

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        out.writeBool(enabled);
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return true;
    }
};

} // namespace cyrex::nw::protocol
