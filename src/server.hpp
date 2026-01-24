#pragma once
#include <stdexcept>
// RakNet
#include <RakNet/RakPeerInterface.h>

class Server
{
public:
    struct Config
    {
        std::uint16_t port{};
        std::uint32_t max_users{};
        std::uint16_t max_incoming_connections{};

        // Default configuration for a typical minecraft bedrock server
        static constexpr Config make_default_mc_be()
        {
            return {.port = 19132, .max_users = 20, .max_incoming_connections = 5};
            return {.port = 19132, .max_users = 20, .max_incoming_connections = 5};
        }
    };

    struct InitFailedError : std::runtime_error
    {
        explicit InitFailedError(const std::string& message);
    };

public:
    // Initializes the server to a usable state
    // Throws: InitFailedError
    explicit Server(const Config& config);
    ~Server();

private:
    // Disable copying
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

public:
    // Moves
    Server(Server&& other) noexcept;
    Server& operator=(Server&& other) noexcept;

public:
    void run();

private:
    void stop();

private:
    void receive_packets();
    void on_packet_received(const RakNet::Packet* packet);

private:
    RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
};