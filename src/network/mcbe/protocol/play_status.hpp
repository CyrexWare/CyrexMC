#pragma once

#include "network/session/network_session.hpp"

namespace cyrex::nw::protocol
{
namespace io = cyrex::nw::io;
namespace ses = cyrex::nw::session;

enum class PlayStatus : uint32_t
{
    LoginSuccess = 0,
    LoginFailedClient = 1,
    LoginFailedServer = 2,
    PlayerSpawn = 3,
    LoginFailedInvalidTenant = 4,
    LoginFailedVanillaEdu = 5,
    LoginFailedEduVanilla = 6,
    LoginFailedServerFull = 7,
    LoginFailedEditorVanilla = 8,
    LoginFailedVanillaEditor = 9
};

class PlayStatusPacket final :
    public PacketImpl<PlayStatusPacket, ProtocolInfo::playStatusPacket, PacketDirection::Clientbound, true>
{
public:
    PlayStatus status = PlayStatus::LoginSuccess;

    bool decodePayload(io::BinaryReader& in) override
    {
        status = static_cast<PlayStatus>(in.readU32BE());
        return true;
    }

    bool encodePayload(io::BinaryWriter& out) const override
    {
        out.writeU32BE(static_cast<uint32_t>(status));
        return true;
    }

    bool handle(ses::NetworkSession& /*session*/) override
    {
        return true;
    }
};

} // namespace cyrex::nw::protocol