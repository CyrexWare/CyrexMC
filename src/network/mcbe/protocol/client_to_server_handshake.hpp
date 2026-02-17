#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"

namespace cyrex::nw::protocol
{

class ClientToServerHandshakePacket final :
    public PacketImpl<ClientToServerHandshakePacket, std::to_underlying(PacketId::ClientToServerHandshake), PacketDirection::Serverbound, true>
{
public:
    bool decodePayload(io::BinaryReader&) override
    {
        // NOOP
        return true;
    }

    bool encodePayload(io::BinaryWriter&) const override
    {
        // NOOP
        return true;
    }

    bool handle(session::NetworkSession& session) override
    {
        // NOOP
        return true;
    }
};

} // namespace cyrex::nw::protocol
