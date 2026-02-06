#pragma once

#include "network/session/network_session.hpp"

namespace cyrex::nw::protocol
{
class PlayStatusPacket final :
    public cyrex::nw::protocol::
        PacketImpl<PlayStatusPacket, ProtocolInfo::playStatusPacket, cyrex::nw::protocol::PacketDirection::Clientbound, true>
{
public:
    // Maybe make this separately?
    static constexpr uint32_t loginSuccess = 0;
    static constexpr uint32_t loginFailedClient = 1;
    static constexpr uint32_t loginFailedServer = 2;
    static constexpr uint32_t playerSpawn = 3;
    static constexpr uint32_t loginFailedInvalidTenant = 4;
    static constexpr uint32_t loginFailedVanillaEdu = 5;
    static constexpr uint32_t loginFailedEduVanilla = 6;
    static constexpr uint32_t loginFailedServerFull = 7;
    static constexpr uint32_t loginFailedEditorVanilla = 8;
    static constexpr uint32_t loginFailedVanillaEditor = 9;

    uint32_t status = 0;

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        status = in.readU32BE();
        return true;
    }

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        out.writeU32BE(status);
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& /*session*/) override
    {
        return true;
    }
};
} // namespace cyrex::nw::protocol
