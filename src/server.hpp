#pragma once

#include "command/command_manager.hpp"
#include "network/mcbe/encryption/encryption.hpp"
#include "network/mcbe/protocol/types/GameMode.hpp"
#include "network/mcbe/protocol/types/SubClientId.hpp"
#include "network/mcbe/resourcepacks/loader/resource_pack_loader_def.hpp"
#include "network/mcbe/resourcepacks/resource_pack_factory.hpp"
#include "network/session/network_session.hpp"
#include "util/server_properties.hpp"

#include <RakNet/RakNetTypes.h>
#include <atomic>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <cstdint>

namespace cyrex::nw::raknet
{
class RaknetHandler;
}

namespace cyrex::player
{
class Player;
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
        bool enableEncryption;

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

    [[nodiscard]] std::size_t getPlayerCount() const;
    cyrex::player::Player& createPlayer(nw::protocol::SubClientId id, nw::session::NetworkSession* session);
    void removePlayer(player::Player& player);
    const std::vector<std::unique_ptr<cyrex::player::Player>>& getPlayers() const noexcept;
    void broadcastPacketToAll(const nw::protocol::Packet& packet, player::Player* exclude = nullptr);

    void stop();
    void run();

    [[nodiscard]] cyrex::nw::resourcepacks::ResourcePackFactory& getResourcePackFactory();
    [[nodiscard]] const cyrex::nw::resourcepacks::ResourcePackFactory& getResourcePackFactory() const;
    cyrex::nw::protocol::AesEncryptor::EccKey* getServerPrivateKey() const;
    [[nodiscard]] bool shouldForceResources() const;
    [[nodiscard]] static bool isEncryptionEnabled();
    [[nodiscard]] static bool isOnlineMode();

private:
    void commandLoop() const;

    Config m_config;
    std::unique_ptr<nw::raknet::RaknetHandler> m_raknet;
    std::unique_ptr<nw::resourcepacks::ResourcePackFactory> m_resourcePackFactory;
    nw::protocol::AesEncryptor::EccKeyPtr m_serverPrivateKey;
    std::uint64_t m_serverUniqueId;
    std::atomic<bool> m_running;
    std::unique_ptr<command::CommandManager> m_commands;
    std::unordered_set<std::unique_ptr<nw::resourcepacks::ResourcePackLoaderDef>> m_loaders;
    std::vector<std::unique_ptr<player::Player>> m_players;
};
} // namespace cyrex
