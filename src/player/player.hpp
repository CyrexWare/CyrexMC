#pragma once

#include "actor/actor.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/protocol/resource_pack_chunk_request.hpp"
#include "network/mcbe/protocol/resource_pack_client_response.hpp"
#include "network/mcbe/protocol/types/SubClientId.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackClientResponseStatus.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackMeta.hpp"
#include "network/session/network_session.hpp"
#include "util/uuid.hpp"

#include <deque>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace cyrex::nw::protocol
{
class ResourcePackChunkRequestPacket;
class ResourcePackClientResponsePacket;
class Packet;
} // namespace cyrex::nw::protocol

namespace cyrex
{
class Server;
}

namespace cyrex::player
{

class Player final : public actor::Actor
{
private:
    nw::protocol::SubClientId m_subClientId;
    nw::session::NetworkSession* m_session;
    cyrex::Server& m_server;
    bool m_markedForDisconnect{false};

    std::map<uuid::UUID, std::unique_ptr<nw::protocol::ResourcePackMeta>> m_loadedPacks;
    std::deque<uuid::UUID> m_packQueue;
    std::deque<std::pair<uuid::UUID, int>> m_pendingChunks;

    uuid::UUID m_currentPack{};
    bool m_queueProcessing = false;

public:
    using SubClientId = nw::protocol::SubClientId;

    Player(SubClientId id, nw::session::NetworkSession* session, cyrex::Server& server) noexcept;

    SubClientId getSubClientId() const noexcept;

    void sendPacket(std::unique_ptr<nw::protocol::Packet> packet, bool immediately = false);

    template <class... Packets>
    void sendPacketBatch(bool immediately, Packets&&... packets);

    void markForDisconnect() noexcept;
    bool isMarkedForDisconnect() const noexcept;

    void doLoginSuccess();
    void processChunkQueue();
    bool handleResourcePackChunkRequest(const nw::protocol::ResourcePackChunkRequestPacket& request);
    bool handleResourcePackClientResponse(const nw::protocol::ResourcePackClientResponsePacket& pk);
    void nextPack();

    void disconnect(const std::string& message);
};
} // namespace cyrex::player