#include "raknet_motd.hpp"

#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/protocol/types/GameMode.hpp"

std::string cyrex::network::raknet::buildMotd(const Server& server)
{
    auto gm = server.getDefaultGameMode();

    int gmNumeric = 1;
    if (gm == protocol::GameMode::CREATIVE)
        gmNumeric = 0;
    if (gm == protocol::GameMode::ADVENTURE)
        gmNumeric = 2;

    std::string motd;
    motd += "MCPE;";
    motd += server.getMotd();
    motd += ";";
    motd += std::to_string(protocol::ProtocolInfo::currentProtocol);
    motd += ";";
    motd += protocol::ProtocolInfo::minecraftVersionNetwork;
    motd += ";";
    motd += std::to_string(server.getPlayerCount());
    motd += ";";
    motd += std::to_string(server.getMaxPlayers());
    motd += ";";
    motd += std::to_string(server.getServerUniqueId());
    motd += ";";
    motd += server.getServerName();
    motd += ";";
    motd += protocol::toGameModeString(gm);
    motd += ";";
    motd += std::to_string(gmNumeric);
    motd += ";";
    motd += std::to_string(server.getPort());
    motd += ";";
    motd += std::to_string(server.getPortIpv6());
    motd += ";";

    return motd;
}
