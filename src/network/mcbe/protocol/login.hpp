#pragma once

#include "network/session/network_session.hpp"

#include <iostream>

namespace cyrex::nw::protocol
{
class LoginPacket final :
    public cyrex::nw::protocol::PacketImpl<LoginPacket, ProtocolInfo::loginPacket, cyrex::nw::protocol::PacketDirection::Serverbound, false>
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

        const uint32_t authLen = cr.readU32LE();
        authInfoJson = cr.readBytes(authLen);

        const uint32_t clientLen = cr.readU32LE();
        clientDataJwt = cr.readBytes(clientLen);
    }

    [[nodiscard]] std::string tryEncodeRequestForConnection() const
    {
        cyrex::nw::io::BinaryWriter cr{};

        cr.writeU32LE(static_cast<uint32_t>(authInfoJson.size()));
        cr.writeBuffer(reinterpret_cast<const uint8_t*>(authInfoJson.data()), authInfoJson.size());

        cr.writeU32LE(static_cast<uint32_t>(clientDataJwt.size()));
        cr.writeBuffer(reinterpret_cast<const uint8_t*>(clientDataJwt.data()), clientDataJwt.size());

        return {reinterpret_cast<const char*>(cr.data()), cr.length()};
    }
};

} // namespace cyrex::nw::protocol
