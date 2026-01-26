#include <gtest/gtest.h>

#include <cyrexmc/server.hpp>

class MockPeer : public cyrex::INetworkPeer
{
public:
    StartupResult startup(const StartupInfo startupInfo) override
    {
        return StartupResult::RAKNET_STARTED;
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
};

// --- TEST SUITE 1 ---
TEST(ExampleTest, IDK)
{
    MockPeer peer;

    //Server obj
    cyrex::Server server;
}
