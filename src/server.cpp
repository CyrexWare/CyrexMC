#include "server.hpp"

#include <iostream>
#include <magic_enum/magic_enum.hpp>

// Raknet
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/RakSleep.h>

Server::InitFailedError::InitFailedError(const std::string& message) : std::runtime_error(message)
{
}

Server::Server(const Config& config)
{
    RakNet::SocketDescriptor socketDescriptor(config.port, nullptr);
    const RakNet::StartupResult startupResult = m_peer->Startup(config.maxUsers, &socketDescriptor, 1);

    // All clear
    if (startupResult == RakNet::RAKNET_STARTED)
    {
        m_peer->SetMaximumIncomingConnections(config.maxIncomingConnections);
        return;
    }

    throw InitFailedError(std::string(magic_enum::enum_name(startupResult)));
}

Server::~Server()
{
    stop();
}

Server::Server(Server&& other) noexcept : m_peer{other.m_peer}
{
    other.m_peer = nullptr;
}

Server& Server::operator=(Server&& other) noexcept
{
    stop();
    m_peer = other.m_peer;
    other.m_peer = nullptr;
    return *this;
}

void Server::run()
{
    while (true)
    {
        receivePackets();
        RakSleep(15);
    }
}

void Server::stop()
{
    if (m_peer && m_peer->IsActive())
    {
        m_peer->Shutdown(50);
    }
}

void Server::receivePackets()
{
    for (RakNet::Packet* packet{}; (packet = m_peer->Receive()) != nullptr; m_peer->DeallocatePacket(packet))
    {
        onPacketReceived(packet);
    }
}

void Server::onPacketReceived(const RakNet::Packet* packet)
{
    switch (packet->data[0])
    {
        case ID_NEW_INCOMING_CONNECTION:
            std::cerr << "A user is connecting...\n";
            break;
    }
}
