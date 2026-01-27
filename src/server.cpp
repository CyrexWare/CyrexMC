#include "server.hpp"

#include <iostream>
#include <magic_enum/magic_enum.hpp>

// Raknet
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/RakSleep.h>

///  Exceptions
cyrex::Server::InitFailedError::InitFailedError(const std::string& message) : std::runtime_error(message)
{
}

cyrex::Server::NullPacketException::NullPacketException(const std::string& message) : std::runtime_error(message)
{
}
// --

cyrex::Server::Server(INetworkPeer* const peer, const Config& config)
{
    if (peer == nullptr)
    {
        throw InitFailedError("peer is nullptr");
    }

    this->m_peer = peer;

    const RakNet::StartupResult startupResult = m_peer->startup({.maxConnections = config.maxUsers, .port = config.port});

    // All clear
    if (startupResult == RakNet::RAKNET_STARTED)
    {
        m_peer->setMaximumIncomingConnections(config.maxIncomingConnections);
        return;
    }

    throw InitFailedError(std::string(magic_enum::enum_name(startupResult)));
}

cyrex::Server::~Server()
{
    stop();
}

cyrex::Server::Server(Server&& other) noexcept : m_peer{other.m_peer}
{
    other.m_peer = nullptr;
}

cyrex::Server& cyrex::Server::operator=(Server&& other) noexcept
{
    stop();
    m_peer = other.m_peer;
    other.m_peer = nullptr;
    return *this;
}

void cyrex::Server::run()
{
    while (true)
    {
        receivePackets();
        RakSleep(15);
    }
}

void cyrex::Server::stop()
{
    if (m_peer && m_peer->isActive())
    {
        m_peer->shutdown({.blockDuration = 50});
    }
}

void cyrex::Server::receivePackets()
{
    for (RakNet::Packet* packet{}; (packet = m_peer->receive()) != nullptr; m_peer->deallocatePacket(packet))
    {
        onPacketReceived(packet);
    }
}

void cyrex::Server::onPacketReceived(RakNet::Packet* const packet)
{
    if (packet == nullptr)
    {
        throw NullPacketException("null packet in onPacketReceived");
    }

    switch (packet->data[0])
    {
        case ID_NEW_INCOMING_CONNECTION:
            std::cerr << "A user is connecting...\n";
            break;
    }
}
