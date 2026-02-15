#pragma once

#include "command/command_manager.hpp"
#include "network/mcbe/protocol/types/GameMode.hpp"
#include "network/mcbe/resourcepacks/loader/resource_pack_loader_def.hpp"
#include "network/mcbe/resourcepacks/resource_pack_factory.hpp"
#include "util/server_properties.hpp"

#include <RakNet/RakNetTypes.h>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <cstdint>

namespace cyrex::nw::raknet
{
class RaknetHandler;
}

namespace cyrex
{

class Server
{
public:
    struct Config
    {
        std::uint16_t port;
        std::uint16_t portIpv6;
        std::uint32_t maxPlayers;
        std::string serverName;
        std::string motd;
        cyrex::nw::protocol::GameMode defaultGameMode;
        bool forceResources;

        static Config fromProperties(const cyrex::util::ServerProperties& props);
    };

    explicit Server(Config config);
    ~Server();

    [[nodiscard]] std::uint16_t getPort() const;
    [[nodiscard]] std::uint16_t getPortIpv6() const;
    [[nodiscard]] std::uint32_t getMaxPlayers() const;
    [[nodiscard]] std::uint64_t getServerUniqueId() const;
    [[nodiscard]] const std::string& getServerName() const;
    [[nodiscard]] const std::string& getMotd() const;

    [[nodiscard]] cyrex::nw::protocol::GameMode getDefaultGameMode() const;
    void setDefaultGameMode(cyrex::nw::protocol::GameMode mode);
    void setDefaultGameModeFromString(std::string_view mode);

    void addPlayer(const RakNet::RakNetGUID& guid);
    void removePlayer(const RakNet::RakNetGUID& guid);
    [[nodiscard]] bool hasPlayer(const RakNet::RakNetGUID& guid) const;
    [[nodiscard]] std::size_t getPlayerCount() const;
    [[nodiscard]] const std::vector<RakNet::RakNetGUID>& getAllPlayers() const;

    void stop();
    void run();

    [[nodiscard]] cyrex::nw::resourcepacks::ResourcePackFactory& getResourcePackFactory();
    [[nodiscard]] const cyrex::nw::resourcepacks::ResourcePackFactory& getResourcePackFactory() const;
    [[nodiscard]] bool shouldForceResources() const;

private:
    void commandLoop();

    Config m_config;
    std::unique_ptr<cyrex::nw::raknet::RaknetHandler> m_raknet;
    std::unique_ptr<cyrex::nw::resourcepacks::ResourcePackFactory> m_resourcePackFactory;
    std::vector<RakNet::RakNetGUID> m_players;
    std::uint64_t m_serverUniqueId;
    std::atomic<bool> m_running;
    std::unique_ptr<cyrex::command::CommandManager> m_commands;
};
} // namespace cyrex
