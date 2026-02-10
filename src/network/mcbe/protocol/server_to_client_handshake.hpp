#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/session/network_session.hpp"

#include <string>

namespace cyrex::nw::protocol
{

class ServerToClientHandshakePacket final :
    public PacketImpl<ServerToClientHandshakePacket, static_cast<uint32_t>(PacketId::ServerToClientHandshake), PacketDirection::Clientbound, true>
{
public:
    std::string jwt;

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        jwt = in.readString();
        return true;
    }

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        out.writeString(jwt);
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return true;
    }
};

} // namespace cyrex::nw::protocol
