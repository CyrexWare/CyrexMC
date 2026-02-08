#include "server.hpp"

#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/raknet/handler/raknet_handler.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <utility>

using namespace cyrex::nw::protocol;
namespace util = cyrex::util;
namespace rak = cyrex::nw::raknet;
namespace cmd = cyrex::command;

namespace
{
std::uint64_t generateServerId()
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<std::uint64_t> dist;
    return dist(gen);
}
} // namespace

cyrex::Server::Config cyrex::Server::Config::fromProperties(const util::ServerProperties& p)
{
    return {p.port, p.portIpv6, p.maxPlayers, p.serverName, p.motd, p.defaultGameMode};
}

cyrex::Server::Server(Config config) :
    m_config(std::move(config)),
    m_serverUniqueId(generateServerId()),
    m_running(true)
{
    m_raknet = std::make_unique<rak::RaknetHandler>(*this);
    m_commands = std::make_unique<cmd::CommandManager>(*this);
    m_commands->registerDefaults();
}

cyrex::Server::~Server()
{
    m_running = false;
    m_players.clear();
}

std::uint16_t cyrex::Server::getPort() const
{
    return m_config.port;
}

std::uint16_t cyrex::Server::getPortIpv6() const
{
    return m_config.portIpv6;
}

std::uint32_t cyrex::Server::getMaxPlayers() const
{
    return m_config.maxPlayers;
}

std::uint64_t cyrex::Server::getServerUniqueId() const
{
    return m_serverUniqueId;
}

const std::string& cyrex::Server::getServerName() const
{
    return m_config.serverName;
}

const std::string& cyrex::Server::getMotd() const
{
    return m_config.motd;
}

GameMode cyrex::Server::getDefaultGameMode() const
{
    return m_config.defaultGameMode;
}

void cyrex::Server::setDefaultGameMode(GameMode mode)
{
    m_config.defaultGameMode = mode;
}

void cyrex::Server::setDefaultGameModeFromString(std::string_view mode)
{
    m_config.defaultGameMode = fromString(mode);
}

std::size_t cyrex::Server::getPlayerCount() const
{
    return m_players.size();
}

const std::vector<RakNet::RakNetGUID>& cyrex::Server::getAllPlayers() const
{
    return m_players;
}

void cyrex::Server::addPlayer(const RakNet::RakNetGUID& guid)
{
    if (!hasPlayer(guid))
        m_players.push_back(guid);
}

void cyrex::Server::removePlayer(const RakNet::RakNetGUID& guid)
{
    std::erase_if(m_players, [&](const auto& g) { return g == guid; });
}

bool cyrex::Server::hasPlayer(const RakNet::RakNetGUID& guid) const
{
    return std::find(m_players.begin(), m_players.end(), guid) != m_players.end();
}

void cyrex::Server::stop()
{
    m_running = false;
    m_players.clear();
}

void cyrex::Server::commandLoop()
{
    while (m_running)
    {
        std::string line;
        if (!std::getline(std::cin, line))
            break;

        if (line.empty())
            continue;

        m_commands->executeConsole(line);
    }
}

void cyrex::Server::run()
{
    [[maybe_unused]] std::thread commandThread(&Server::commandLoop, this);

    while (m_running)
    {
        m_raknet->poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (commandThread.joinable())
        commandThread.join();
}
