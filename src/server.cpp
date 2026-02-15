#include "server.hpp"

#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/resourcepacks/loader/resource_pack_loader_def.hpp"
#include "network/mcbe/resourcepacks/loader/zipped_resource_pack_loader.hpp"
#include "network/mcbe/resourcepacks/resource_pack_factory.hpp"
#include "network/raknet/handler/raknet_handler.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <unordered_set>
#include <utility>

cyrex::Server::Config cyrex::Server::Config::fromProperties(const cyrex::util::ServerProperties& p)
{
    return {p.port, p.portIpv6, p.maxPlayers, p.serverName, p.motd, p.defaultGameMode, p.forceResources};
}

namespace
{
std::uint64_t generateServerId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<std::uint64_t> dist;
    return dist(gen);
}
} // namespace

cyrex::Server::Server(Config config) :
    m_config(std::move(config)),
    m_serverUniqueId(generateServerId()),
    m_running(true)
{
    m_raknet = std::make_unique<cyrex::nw::raknet::RaknetHandler>(*this);

    using namespace cyrex::nw::resourcepacks;

    std::unordered_set<std::shared_ptr<ResourcePackLoaderDef>> loaders;

    // use registerLoader instead?
    loaders.insert(
        std::make_shared<cyrex::nw::resourcepacks::ZippedResourcePackLoader>(std::filesystem::path("resource_packs")));

    m_resourcePackFactory = std::make_unique<ResourcePackFactory>(std::move(loaders));

    m_commands = std::make_unique<cyrex::command::CommandManager>(*this);
    m_commands->registerDefaults();
}

cyrex::Server::~Server()
{
    // more better cleanup, and we need cleanup function for sessions, etc
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

cyrex::nw::protocol::GameMode cyrex::Server::getDefaultGameMode() const
{
    return m_config.defaultGameMode;
}

void cyrex::Server::setDefaultGameMode(cyrex::nw::protocol::GameMode mode)
{
    m_config.defaultGameMode = mode;
}

void cyrex::Server::setDefaultGameModeFromString(std::string_view mode)
{
    m_config.defaultGameMode = cyrex::nw::protocol::fromString(mode);
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

cyrex::nw::resourcepacks::ResourcePackFactory& cyrex::Server::getResourcePackFactory()
{
    return *m_resourcePackFactory;
}

const cyrex::nw::resourcepacks::ResourcePackFactory& cyrex::Server::getResourcePackFactory() const
{
    return *m_resourcePackFactory;
}

bool cyrex::Server::shouldForceResources() const
{
    return m_config.forceResources;
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
