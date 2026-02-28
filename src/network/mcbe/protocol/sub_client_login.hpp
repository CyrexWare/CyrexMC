#pragma once

#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"

#include <string>

#include <cstdint>

namespace cyrex::network::protocol
{

class SubClientLoginPacket final :
    public PacketImpl<SubClientLoginPacket, std::to_underlying(PacketId::SubClientLogin), PacketDirection::Serverbound, false>
{
public:
    std::string authInfoJson;
    std::string clientDataJwt;

    bool encodePayload(io::BinaryWriter& out) const override
    {
        // NOOP
        return false;
    }

    bool decodePayload(io::BinaryReader& in) override
    {
        tryDecodeRequestForConnection(in.readString());
        return true;
    }

    bool handle(session::NetworkSession& session) override
    {
        if (subClientId == SubClientId::PrimaryClient)
        {
            return false;
        }
        return session.handleSubClientLogin(authInfoJson, clientDataJwt);
    }

private:
    void tryDecodeRequestForConnection(const std::string& binary)
    {
        io::BinaryReader cr(reinterpret_cast<const uint8_t*>(binary.data()), binary.size());

        authInfoJson = cr.readStringU32LE();
        clientDataJwt = cr.readStringU32LE();
    }
};

} // namespace cyrex::network::protocol
