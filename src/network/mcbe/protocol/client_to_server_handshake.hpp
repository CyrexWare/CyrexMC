#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/session/network_session.hpp"

namespace cyrex::nw::protocol
{

class ClientToServerHandshakePacket final :
    public PacketImpl<ClientToServerHandshakePacket, static_cast<uint32_t>(PacketId::ClientToServerHandshake), PacketDirection::Serverbound, true>
{
public:
    bool decodePayload(cyrex::nw::io::BinaryReader&) override
    {
        // NOOP
        return true;
    }

    bool encodePayload(cyrex::nw::io::BinaryWriter&) const override
    {
        // NOOP
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        // NOOP
        return true;
    }
};

} // namespace cyrex::nw::protocol
