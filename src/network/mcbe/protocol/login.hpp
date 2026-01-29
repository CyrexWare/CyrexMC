#pragma once

#include "log/console_logger.hpp"
#include "log/message_type.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/session/network_session.hpp"
#include "text/format/builder.hpp"
#include "text/format/color.hpp"

#include <iostream>

namespace cyrex::network::mcbe::protocol
{
class LoginPacket final : public cyrex::network::mcbe::Packet
{
public:
    uint32_t protocol = 0;
    std::string authInfoJson;
    std::string clientDataJwt;

    [[nodiscard]] uint32_t networkId() const override
    {
        return ProtocolInfo::loginPacket;
    }

    [[nodiscard]] cyrex::network::mcbe::PacketDirection direction() const override
    {
        return cyrex::network::mcbe::PacketDirection::Serverbound;
    }

    [[nodiscard]] bool allowBeforeLogin() const override
    {
        return false;
    }

protected:
    void decodePayload(cyrex::network::io::BinaryReader& in) override
    {
        protocol = in.readU32BE();
        cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_DEBUG,
                                       cyrex::text::format::Builder()
                                           .color(text::format::Color::DARK_GRAY)
                                           .text("[MCBE] Received Protocol Version (LoginPacket): " + protocol)
                                           .build());
        const std::string connectionRequest = in.readString();
        tryDecodeRequestForConnection(connectionRequest);
    }

    void encodePayload(cyrex::network::io::BinaryWriter& out) const override
    {
        out.writeU32BE(protocol);
        out.writeString(tryEncodeRequestForConnection());
    }

public:
    bool handle(cyrex::network::session::NetworkSession& /*session*/) override
    {
        return false;
    }

private:
    void tryDecodeRequestForConnection(const std::string& binary)
    {
        cyrex::network::io::BinaryReader cr(reinterpret_cast<const uint8_t*>(binary.data()), binary.size());

        const uint32_t authLen = cr.readU32LE();
        authInfoJson = cr.readBytes(authLen);

        const uint32_t clientLen = cr.readU32LE();
        clientDataJwt = cr.readBytes(clientLen);
    }

    [[nodiscard]] std::string tryEncodeRequestForConnection() const
    {
        cyrex::network::io::BinaryWriter cr{};

        cr.writeU32LE(static_cast<uint32_t>(authInfoJson.size()));
        cr.writeBuffer(reinterpret_cast<const uint8_t*>(authInfoJson.data()), authInfoJson.size());

        cr.writeU32LE(static_cast<uint32_t>(clientDataJwt.size()));
        cr.writeBuffer(reinterpret_cast<const uint8_t*>(clientDataJwt.data()), clientDataJwt.size());

        return {reinterpret_cast<const char*>(cr.data()), cr.length()};
    }
};
} // namespace cyrex::network::mcbe::protocol
