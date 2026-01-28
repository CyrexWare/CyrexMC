// Glalie: redo
//#include <gtest/gtest.h>
//
//#include <server.hpp>
//
//struct cyrex::Server::Testing
//{
//    static void onNullPacketReceived(Server& server)
//    {
//        server.onPacketReceived(nullptr);
//    }
//};
//
//class MockPeer : public cyrex::INetworkPeer
//{
//public:
//    [[nodiscard]] cyrex::StartupResult startup(const StartupInfo startupInfo) override
//    {
//        (void)startupInfo;
//        return cyrex::StartupResult::RAKNET_ALREADY_STARTED;
//    }
//
//    void shutdown(const ShutdownInfo shutdownInfo) override
//    {
//        (void)shutdownInfo;
//    }
//
//    [[nodiscard]] cyrex::Packet* receive() override
//    {
//        return nullptr;
//    }
//
//    [[nodiscard]] std::uint32_t send(const char* data, const std::uint32_t length, const SendInfo sendInfo) override
//    {
//        (void)data;
//        (void)length;
//        (void)sendInfo;
//        return 0;
//    }
//
//    void setMaximumIncomingConnections(const std::uint16_t maxIncomingConnections) override
//    {
//        (void)maxIncomingConnections;
//    }
//
//    void deallocatePacket(cyrex::Packet* packet) override
//    {
//        (void)packet;
//    }
//
//    [[nodiscard]] bool isActive() const override
//    {
//        return false;
//    }
//};
//
//// --- TEST SUITE 1 ---
//TEST(ServerTest, ConstructorThrowsOnNullPeer)
//{
//    ASSERT_THROW(cyrex::Server(nullptr, {}), cyrex::Server::InitFailedError);
//}
//
//TEST(ServerTest, ConstructorThrowsOnInvalidConfig)
//{
//    const cyrex::Server::Config badMaxUsers{
//        .port = 1234,
//        .maxUsers = 0,
//        .maxIncomingConnections = 10,
//    };
//
//    const cyrex::Server::Config badIncomingConnections{
//        .port = 1234,
//        .maxUsers = 20,
//        .maxIncomingConnections = 0,
//    };
//
//    auto mockPeerA = MockPeer{};
//    auto mockPeerB = MockPeer{};
//
//    ASSERT_THROW(cyrex::Server(&mockPeerA, badMaxUsers), cyrex::Server::InitFailedError);
//    ASSERT_THROW(cyrex::Server(&mockPeerB, badIncomingConnections), cyrex::Server::InitFailedError);
//}
//
//TEST(ServerTest, OnPacketReceivedThrowsNullPacketException)
//{
//    const cyrex::Server::Config cfg{
//        .port = 1234,
//        .maxUsers = 10,
//        .maxIncomingConnections = 10,
//    };
//
//    auto peer = MockPeer{};
//    cyrex::Server server(&peer, cfg);
//    ASSERT_THROW(cyrex::Server::Testing::onNullPacketReceived(server), cyrex::Server::NullPacketException);
//}
