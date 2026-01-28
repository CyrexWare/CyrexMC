#pragma once

#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/session/network_session.hpp"
#include "util/textformat.hpp"

#include <iostream>

using namespace cyrex::network::io;
using namespace cyrex::network::session;

namespace cyrex::network::mcbe::protocol
{
class LoginPacket final : public cyrex::network::mcbe::Packet
{
public:
    uint32_t protocol = 0;
    std::string authInfoJson;
    std::string clientDataJwt;

    uint32_t networkId() const override
    {
        return ProtocolInfo::LOGIN_PACKET;
    }

    cyrex::network::mcbe::PacketDirection direction() const override
    {
        return cyrex::network::mcbe::PacketDirection::Serverbound;
    }

    bool allowBeforeLogin() const override
    {
        return false;
    }

protected:
    void decodePayload(BinaryReader& in) override
    {
        protocol = in.readU32BE();
        std::cout << (cyrex::util::renderConsole(cyrex::util::bedrock(cyrex::util::Color::GREEN) + "[MCBE] ", true) +
                      cyrex::util::renderConsole(cyrex::util::bedrock(cyrex::util::Color::EMERALD) + "Received "
                                                                                                     "Protocol Version "
                                                                                                     "(LoginPacket): ",
                                                 false))
                  << protocol << std::endl;
        std::string connectionRequest = in.readString();
        tryDecodeRequestForConnection(connectionRequest);
    }

    void encodePayload(BinaryWriter& out) const override
    {
        out.writeU32BE(protocol);
        out.writeString(tryEncodeRequestForConnection());
    }

public:
    bool handle(NetworkSession& session) override
    {
        return false;
    }

private:
    void tryDecodeRequestForConnection(const std::string& binary)
    {
        BinaryReader cr(reinterpret_cast<const uint8_t*>(binary.data()), binary.size());

        uint32_t authLen = cr.readU32LE();
        authInfoJson = cr.readBytes(authLen);

        uint32_t clientLen = cr.readU32LE();
        clientDataJwt = cr.readBytes(clientLen);
    }

    std::string tryEncodeRequestForConnection() const
    {
        BinaryWriter cr;

        cr.writeU32LE(static_cast<uint32_t>(authInfoJson.size()));
        cr.writeBuffer(reinterpret_cast<const uint8_t*>(authInfoJson.data()), authInfoJson.size());

        cr.writeU32LE(static_cast<uint32_t>(clientDataJwt.size()));
        cr.writeBuffer(reinterpret_cast<const uint8_t*>(clientDataJwt.data()), clientDataJwt.size());

        return std::string(reinterpret_cast<const char*>(cr.data()), cr.length());
    }
};
} // namespace cyrex::network::mcbe::protocol