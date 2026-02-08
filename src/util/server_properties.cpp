#include "util/server_properties.hpp"

#include "network/mcbe/protocol/types/GameMode.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace cyrex::util
{
namespace proto = cyrex::nw::protocol;

namespace
{

ServerProperties defaults()
{
    return {19132, 19133, 100, "CyrexMC", "Cyrex MCBE Software", proto::GameMode::SURVIVAL};
}

void writeDefaults(const std::string& path, const ServerProperties& cfg)
{
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open())
        return;

    out << "server-port=" << cfg.port << "\n"
        << "server-portv6=" << cfg.portIpv6 << "\n"
        << "max-players=" << cfg.maxPlayers << "\n"
        << "server-name=" << cfg.serverName << "\n"
        << "motd=" << cfg.motd << "\n"
        << "gamemode=" << proto::toString(cfg.defaultGameMode) << "\n";
}

} // anonymous namespace

ServerProperties ServerProperties::load(const std::string& path)
{
    if (!std::filesystem::exists(path))
    {
        auto def = defaults();
        writeDefaults(path, def);
        return def;
    }

    auto cfg = defaults();
    std::ifstream in(path);
    if (!in.is_open())
        return cfg;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        auto pos = line.find('=');
        if (pos == std::string::npos)
            continue;

        const std::string key = line.substr(0, pos);
        const std::string val = line.substr(pos + 1);

        if (key == "server-port")
            cfg.port = static_cast<std::uint16_t>(std::stoi(val));
        else if (key == "server-portv6")
            cfg.portIpv6 = static_cast<std::uint16_t>(std::stoi(val));
        else if (key == "max-players")
            cfg.maxPlayers = static_cast<std::uint32_t>(std::stoul(val));
        else if (key == "server-name")
            cfg.serverName = val;
        else if (key == "motd")
            cfg.motd = val;
        else if (key == "gamemode")
            cfg.defaultGameMode = proto::fromString(val); // TODO: maybe rename fromString to something clearer
    }

    return cfg;
}

} // namespace cyrex::util
