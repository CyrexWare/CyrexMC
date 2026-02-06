#include "raknet_motd.hpp"

#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/protocol/types/GameMode.hpp"

std::string cyrex::network::raknet::buildMotd(const cyrex::Server& server)
{
    auto gm = server.getDefaultGameMode();

    int gmNumeric = 1;
    if (gm == cyrex::mcpe::protocol::types::GameMode::CREATIVE)
        gmNumeric = 0;
    if (gm == cyrex::mcpe::protocol::types::GameMode::ADVENTURE)
        gmNumeric = 2;

    std::string motd;
    motd += "MCPE;";
    motd += server.getMotd();
    motd += ";";
    motd += std::to_string(cyrex::network::mcbe::protocol::ProtocolInfo::currentProtocol);
    motd += ";";
    motd += cyrex::network::mcbe::protocol::ProtocolInfo::minecraftVersionNetwork;
    motd += ";";
    motd += std::to_string(server.getPlayerCount());
    motd += ";";
    motd += std::to_string(server.getMaxPlayers());
    motd += ";";
    motd += std::to_string(server.getServerUniqueId());
    motd += ";";
    motd += server.getServerName();
    motd += ";";
    motd += mcpe::protocol::types::toString(gm);
    motd += ";";
    motd += std::to_string(gmNumeric);
    motd += ";";
    motd += std::to_string(server.getPort());
    motd += ";";
    motd += std::to_string(server.getPortIpv6());
    motd += ";";

    return motd;
}
