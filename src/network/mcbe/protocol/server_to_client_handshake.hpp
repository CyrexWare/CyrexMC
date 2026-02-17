#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/packetids.hpp"

#include <string>

namespace cyrex::network::protocol
{

class ServerToClientHandshakePacket final :
    public PacketImpl<ServerToClientHandshakePacket, std::to_underlying(PacketId::ServerToClientHandshake), PacketDirection::Clientbound, true>
{
public:
    std::string jwt;

    bool decodePayload(io::BinaryReader& in) override
    {
        jwt = in.readString();
        return true;
    }

    bool encodePayload(io::BinaryWriter& out) const override
    {
        out.writeString(jwt);
        return true;
    }

    bool handle(session::NetworkSession& session) override
    {
        return true;
    }
};

} // namespace cyrex::network::protocol
