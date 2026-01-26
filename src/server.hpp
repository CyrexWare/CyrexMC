#pragma once

#include <stdexcept>

#include <cstdint>
#include <unordered_map>

// RakNet
#include <RakNet/RakPeerInterface.h>

#include "client.hpp"

class Server
{
public:
    struct Config
    {
        std::uint16_t port{};
        std::uint32_t maxUsers{};
        std::uint16_t maxIncomingConnections{};

        // Default configuration for a typical minecraft bedrock server
        static constexpr Config makeDefault()
        {
            return {.port = 19132, .maxUsers = 20, .maxIncomingConnections = 5};
        }
    };

    struct InitFailedError : std::runtime_error
    {
        explicit InitFailedError(const std::string& message);
    };

    // Initializes the server to a usable state
    // Throws: InitFailedError
    explicit Server(const Config& config);
    ~Server();
    Server(Server&& other) noexcept;
    Server& operator=(Server&& other) noexcept;
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void run();

private:
    void stop();
    void receivePackets();
    void onPacketReceived(RakNet::Packet* packet);

    RakNet::RakPeerInterface* m_peer = RakNet::RakPeerInterface::GetInstance();

    std::unordered_map<RakNet::RakNetGUID, Client> clients;
};