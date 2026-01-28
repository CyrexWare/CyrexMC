#pragma once

#include "network/mcbe/packetbase.hpp"
#include "network/mcbe/protocol/request_network_settings.hpp"
#include "network/session/network_session.hpp"

#include <iostream>

namespace cyrex::network::mcbe::handler
{

// TODO: make an handler base class.
class SessionBeginHandler
{
public:
    void handle(cyrex::network::session::NetworkSession& session, cyrex::network::mcbe::PacketBase& packet)
    {
        // theres no way this is actually good, glalie review this, if this is lwk valid, idc yk
        auto& pk = static_cast<cyrex::network::mcbe::protocol::RequestNetworkSettingsPacket&>(packet);
        uint32_t version = pk.protocolVersion;

        // no guarentee that a protocol is accepted at the moment
        // we mabye will support multiple versions or make it easy for plugins to.
        if (!isProtocolMabyeAccepted(version))
        {
            session.disconnectUserForIncompatiableProtocol(version);
            return;
        }

        session.setProtocolId(version);

        // this packet needs to be properly handled and we should call session's compressor networkId, right now this is just hardcoded
        cyrex::network::mcbe::protocol::NetworkSettingsPacket pak{};
        pak.compressionThreshold = cyrex::network::mcbe::protocol::NetworkSettingsPacket::COMPRESS_EVERYTHING;
        pak.compressionAlgorithm = 1;
        pak.padding = 00;
        pak.enableClientThrottling = false;
        pak.clientThrottleThreshold = 0;
        pak.clientThrottleScalar = 0.0f;
        pak.trailingZero = 0;
        session.send(pak);
        // mark compression as ready to go lol!
        session.compressionEnabled = true;
    }

    // no way im planning multiprotocol??
    constexpr bool isProtocolMabyeAccepted(std::uint32_t protocol)
    {
        for (auto p : cyrex::network::mcbe::protocol::ProtocolInfo::ACCEPTED_PROTOCOLS)
            if (p == protocol)
                return true;
        return false;
    }
};
} // namespace cyrex::network::mcbe::handler
