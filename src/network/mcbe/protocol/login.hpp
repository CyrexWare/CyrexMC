#pragma once

#include "network/session/network_session.hpp"

namespace cyrex::network::mcbe::protocol
{
class LoginPacket final : public PacketImpl<LoginPacket, ProtocolInfo::loginPacket, PacketDirection::Serverbound, false>
{
public:
    std::uint32_t protocol = 0;
    std::string authInfoJson;
    std::string clientDataJwt;

    bool encodePayload(io::BinaryWriter& out) const override
    {
        out.writeU32BE(protocol);
        out.writeString(tryEncodeRequestForConnection());
        return true;
    }

    bool decodePayload(io::BinaryReader& in) override
    {
        protocol = in.readU32BE();
        tryDecodeRequestForConnection(in.readString());
        return true;
    }

    bool handle(session::NetworkSession& session) override
    {
        return session.handleLogin(protocol, authInfoJson, clientDataJwt);
    }

private:
    void tryDecodeRequestForConnection(const std::string& binary)
    {
        io::BinaryReader cr(reinterpret_cast<const uint8_t*>(binary.data()), binary.size());

        const uint32_t authLen = cr.readU32LE();
        authInfoJson = cr.readBytes(authLen);

        const uint32_t clientLen = cr.readU32LE();
        clientDataJwt = cr.readBytes(clientLen);
    }

    [[nodiscard]] std::string tryEncodeRequestForConnection() const
    {
        io::BinaryWriter cr{};

        cr.writeU32LE(static_cast<uint32_t>(authInfoJson.size()));
        cr.writeBuffer(reinterpret_cast<const uint8_t*>(authInfoJson.data()), authInfoJson.size());

        cr.writeU32LE(static_cast<uint32_t>(clientDataJwt.size()));
        cr.writeBuffer(reinterpret_cast<const uint8_t*>(clientDataJwt.data()), clientDataJwt.size());

        return {reinterpret_cast<const char*>(cr.data()), cr.length()};
    }
};

} // namespace cyrex::network::mcbe::protocol
