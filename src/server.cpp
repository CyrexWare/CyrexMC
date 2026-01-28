#include "server.hpp"

#include "network/raknet/raknet_handler.hpp"
#include "util/textformat.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

cyrex::Server* cyrex::Server::s_instance = nullptr;

cyrex::Server::Config cyrex::Server::Config::fromProperties(const cyrex::util::ServerProperties& p)
{
    return {p.port, p.portIpv6, p.maxPlayers, p.serverName, p.motd, p.defaultGameMode};
}

static std::uint64_t generateServerId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<std::uint64_t> dist;
    return dist(gen);
}

cyrex::Server::Server(const Config& config) : m_config(config), m_serverUniqueId(generateServerId()), m_running(true)
{
    if (s_instance)
        throw std::runtime_error("Server already exists");

    s_instance = this;

    m_raknet = std::make_unique<cyrex::network::raknet::RaknetHandler>();
    m_commands = std::make_unique<cyrex::command::CommandManager>(*this);
    m_commands->registerDefaults();
}

cyrex::Server::~Server()
{
    // more better cleanup, and we need cleanup function for sessions, etc
    m_running = false;
    m_players.clear();
    s_instance = nullptr;
}

cyrex::Server& cyrex::Server::getInstance()
{
    if (!s_instance)
        throw std::runtime_error("Server not initialized");

    return *s_instance;
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

cyrex::mcpe::protocol::types::GameMode cyrex::Server::getDefaultGameMode() const
{
    return m_config.defaultGameMode;
}

void cyrex::Server::setDefaultGameMode(cyrex::mcpe::protocol::types::GameMode mode)
{
    m_config.defaultGameMode = mode;
}

void cyrex::Server::setDefaultGameModeFromString(std::string_view mode)
{
    m_config.defaultGameMode = cyrex::mcpe::protocol::types::fromString(mode);
}

void cyrex::Server::addPlayer(const RakNet::RakNetGUID& guid)
{
    if (!hasPlayer(guid))
        m_players.push_back(guid);
}

void cyrex::Server::removePlayer(const RakNet::RakNetGUID& guid)
{
    auto it = std::remove(m_players.begin(), m_players.end(), guid);
    m_players.erase(it, m_players.end());
}

bool cyrex::Server::hasPlayer(const RakNet::RakNetGUID& guid) const
{
    return std::find(m_players.begin(), m_players.end(), guid) != m_players.end();
}

std::size_t cyrex::Server::getPlayerCount() const
{
    return m_players.size();
}

const std::vector<RakNet::RakNetGUID>& cyrex::Server::getAllPlayers() const
{
    return m_players;
}

void cyrex::Server::stop()
{
    cyrex::Server::~Server();
}

void cyrex::Server::commandLoop()
{
    while (m_running)
    {
        std::string line;
        if (!std::getline(std::cin, line))
            break;

        m_commands->executeConsole(line);
    }
}

void cyrex::Server::run()
{
    std::thread commandThread(&cyrex::Server::commandLoop, this);

    while (m_running)
    {
        m_raknet->poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (commandThread.joinable())
        commandThread.join();
}
