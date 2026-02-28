#include "server.hpp"

#include "info.hpp"
#include "network/mcbe/encryption/encryption.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/resourcepacks/loader/resource_pack_loader_def.hpp"
#include "network/mcbe/resourcepacks/loader/zipped_resource_pack_loader.hpp"
#include "network/mcbe/resourcepacks/resource_pack_factory.hpp"
#include "network/raknet/handler/raknet_handler.hpp"
#include "network/session/network_session.hpp"
#include "player/player.hpp"
#include "util/cpu.hpp"
#include "wolfcrypt/ecc.h"
#include "wolfcrypt/random.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


cyrex::Server::Config cyrex::Server::Config::fromProperties(const cyrex::util::ServerProperties& props)
{
    return {props.port, props.portIpv6, props.maxPlayers, props.serverName, props.motd, props.defaultGameMode, props.forceResources};
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
    m_raknet = std::make_unique<nw::raknet::RaknetHandler>(*this);

    using namespace cyrex::nw::resourcepacks;

    m_loaders.insert(std::make_unique<ZippedResourcePackLoader>(std::filesystem::path("resource_packs")));
    std::unordered_set<ResourcePackLoaderDef*> rawLoaders;
    for (auto& loader : m_loaders)
    {
        rawLoaders.insert(loader.get());
    }

    m_resourcePackFactory = std::make_unique<ResourcePackFactory>(rawLoaders);
    m_serverPrivateKey = nw::protocol::AesEncryptor::generateServerKeypair();
    // }

    m_commands = std::make_unique<cyrex::command::CommandManager>(*this);
    m_commands->registerDefaults();
}


cyrex::Server::~Server()
{
    // more better cleanup, and we need cleanup function for sessions, etc
    m_running = false;
    // m_players.clear();
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

void cyrex::Server::setDefaultGameModeFromString(const std::string_view mode)
{
    m_config.defaultGameMode = cyrex::nw::protocol::fromString(mode);
}

cyrex::player::Player& cyrex::Server::createPlayer(cyrex::player::Player::SubClientId id, nw::session::NetworkSession* session)
{
    m_players.push_back(std::make_unique<cyrex::player::Player>(id, session, *this));
    return *m_players.back();
}

void cyrex::Server::removePlayer(cyrex::player::Player& player)
{
    m_players.erase(std::remove_if(m_players.begin(),
                                   m_players.end(),
                                   [&player](const std::unique_ptr<cyrex::player::Player>& p)
                                   { return p.get() == &player; }),
                    m_players.end());
}

std::size_t cyrex::Server::getPlayerCount() const
{
    return m_players.size();
}

const std::vector<std::unique_ptr<cyrex::player::Player>>& cyrex::Server::getPlayers() const noexcept
{
    return m_players;
}

void cyrex::Server::broadcastPacketToAll(const nw::protocol::Packet& packet, cyrex::player::Player* exclude)
{
    for (const auto& player : m_players)
    {
        if (exclude && player.get() == exclude)
            continue;

        auto cloned = packet.getDef().create();
        player->sendPacket(std::move(cloned));
    }
}

cyrex::nw::resourcepacks::ResourcePackFactory& cyrex::Server::getResourcePackFactory()
{
    return *m_resourcePackFactory;
}

const cyrex::nw::resourcepacks::ResourcePackFactory& cyrex::Server::getResourcePackFactory() const
{
    return *m_resourcePackFactory;
}

cyrex::nw::protocol::AesEncryptor::EccKey* cyrex::Server::getServerPrivateKey() const
{
    return m_serverPrivateKey.get();
}

bool cyrex::Server::shouldForceResources() const
{
    return m_config.forceResources;
}

bool cyrex::Server::isEncryptionEnabled()
{
    return true;
}

bool cyrex::Server::isOnlineMode()
{
    return true;
}

void cyrex::Server::stop()
{
    m_running = false;
    // m_players.clear();
}

void cyrex::Server::commandLoop() const
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
    cyrex::util::CPU cpu;

    std::thread commandThread(&cyrex::Server::commandLoop, this);

    while (m_running)
    {
        m_raknet->poll();

        double load = cpu.getUsage();

        std::ostringstream title;
        title << "CyrexMC v" << Info::version().toString() << " | Load: " << std::fixed << std::setprecision(1) << load
              << "%";

#ifdef _WIN32
        SetConsoleTitleA(title.str().c_str());
#else
        std::cout << "\033]0;" << title.str() << "\007";
#endif

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (commandThread.joinable())
        commandThread.join();
}