#pragma once
#include <stdexcept>
#include <cstdint>

// RakNet
#include "network_peer.hpp"

namespace cyrex
{

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
            return {.port = 19132, .maxUsers = 20, .maxIncomingConnections = 5};
        }
    };

#pragma region Exceptions and Errors
    struct InitFailedError : std::runtime_error
    {
        explicit InitFailedError(const std::string& message);
    };

    struct NullPacketException : std::runtime_error
    {
        explicit NullPacketException(const std::string& message);
    };
#pragma endregion

#pragma region Testing
    struct Testing
    {
        static void onNullPacketReceived(Server& server)
        {
            server.onPacketReceived(nullptr);
        }
    };
#pragma region

    // Initializes the server to a usable state
    // Throws: InitFailedError
    explicit Server(INetworkPeer* const peer, const Config& config);
    ~Server();
    Server(Server&& other) noexcept;
    Server& operator=(Server&& other) noexcept;
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void run();

private:
    void stop();
    void receivePackets();

    // Throws: NullPacketException if the packet is nullptr
    void onPacketReceived(const RakNet::Packet* packet);

private:
    INetworkPeer* m_peer;
};

} // namespace cyrex