#include "util/server_properties.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace cyrex::util
{

static ServerProperties defaults()
{
    return {19134, 19135, 100, "CyrexMC", "Cyrex MCBE Software", mcpe::protocol::types::GameMode::SURVIVAL};
}

static void writeDefaults(const std::string& path, const ServerProperties& cfg)
{
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open())
        return;

    out << "server-port=" << cfg.port << "\n";
    out << "server-portv6=" << cfg.portIpv6 << "\n";
    out << "max-players=" << cfg.maxPlayers << "\n";
    out << "server-name=" << cfg.serverName << "\n";
    out << "motd=" << cfg.motd << "\n";
    out << "gamemode=" << mcpe::protocol::types::toString(cfg.defaultGameMode) << "\n";
}

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
            cfg.defaultGameMode = mcpe::protocol::types::fromString(val);
    }

    return cfg;
}

} // namespace cyrex::util
