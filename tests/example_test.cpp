#include <gtest/gtest.h>

#include <cyrexmc/server.hpp>

class MockPeer : public cyrex::INetworkPeer
{
public:
    StartupResult startup(const StartupInfo startupInfo) override
    {
        return StartupResult::RAKNET_ALREADY_STARTED;
    }

    void shutdown(const ShutdownInfo shutdownInfo) override
    {
    }

    Packet* receive() override
    {
        return nullptr;
    }

    std::uint32_t send(const char* data, const int length, const SendInfo sendInfo) override
    {
        return 0;
    }

    void setMaximumIncomingConnections(const std::uint16_t maxIncomingConnections) override
    {
    }

    void deallocatePacket(Packet* packet) override
    {
    }

    bool isActive() const override
    {
        return false;
    }
};

// --- TEST SUITE 1 ---
TEST(ServerTest, ConstructorThrowsOnNullPeer)
{
    ASSERT_THROW(cyrex::Server(nullptr, {}), cyrex::Server::InitFailedError);
}

TEST(ServerTest, ConstructorThrowsOnInvalidConfig)
{
    const cyrex::Server::Config badMaxUsers{
        .port = 1234,
        .maxUsers = 0,
        .maxIncomingConnections = 10,
    };

    const cyrex::Server::Config badIncomingConnections{
        .port = 1234,
        .maxUsers = 20,
        .maxIncomingConnections = 0,
    };

    auto mockPeerA = MockPeer{};
    auto mockPeerB = MockPeer{};
    
    ASSERT_THROW(cyrex::Server(&mockPeerA, badMaxUsers), cyrex::Server::InitFailedError);
    ASSERT_THROW(cyrex::Server(&mockPeerB, badIncomingConnections), cyrex::Server::InitFailedError);
}
