#pragma once

#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"

#include <string>

#include <cstdint>

namespace cyrex::nw::protocol
{

class LoginPacket final :
    public cyrex::nw::protocol::
        PacketImpl<LoginPacket, std::to_underlying(PacketId::Login), cyrex::nw::protocol::PacketDirection::Serverbound, false>
{
public:
    std::uint32_t protocol = 0;
    std::string authInfoJson;
    std::string clientDataJwt;

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        out.writeU32BE(protocol);
        out.writeString(tryEncodeRequestForConnection());
        return true;
    }

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        protocol = in.readU32BE();
        tryDecodeRequestForConnection(in.readString());
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return session.handleLogin(protocol, authInfoJson, clientDataJwt);
    }

private:
    void tryDecodeRequestForConnection(const std::string& binary)
    {
        cyrex::nw::io::BinaryReader cr(reinterpret_cast<const uint8_t*>(binary.data()), binary.size());

        authInfoJson = cr.readStringU32LE();
        clientDataJwt = cr.readStringU32LE();
    }

    [[nodiscard]] std::string tryEncodeRequestForConnection() const
    {
        cyrex::nw::io::BinaryWriter cr{};

        cr.writeStringU32LE(authInfoJson);
        cr.writeStringU32LE(clientDataJwt);

        return {reinterpret_cast<const char*>(cr.data()), cr.size()};
    }
};

} // namespace cyrex::nw::protocol
