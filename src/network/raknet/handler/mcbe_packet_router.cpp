#include "mcbe_packet_router.hpp"

#include "log/logging.hpp"
#include "network/mcbe/compression/compressors.hpp"
#include "network/session/network_session.hpp"

#include <utility>
#include <vector>

namespace cyrex::nw::raknet
{
namespace ses = cyrex::nw::session;
namespace proto = cyrex::nw::protocol;

void McbePacketRouter::route(RakNet::Packet* p, RaknetConnections& connections)
{
    ses::NetworkSession* session = connections.get(p->guid);
    if (!session)
        return;

    // Skip 0xFE
    const std::uint8_t* data = p->data + 1;
    const std::size_t len = p->length - 1;

    if (len <= 1)
        return;

    std::vector<std::uint8_t> payload;

    if (session->encryptionEnabled)
    {
        if (auto payloadOpt = session->getEncryptor().decrypt({data, len}))
        {
            payload = std::move(*payloadOpt);
        }
        else
        {
            cyrex::logging::error(LOG_MCBE, "failed to decrypt!");
            return;
        }
    }
    else
    {
        payload.assign(data, data + len);
    }

    if (!session->compressionEnabled)
    {
        cyrex::logging::info(LOG_MCBE, "compression inactive.");
    }
    else
    {
        const auto compressionMethod = static_cast<proto::CompressionAlgorithm>(payload.front());

        cyrex::logging::info(LOG_MCBE, "compression method = 0x{:02X}", std::to_underlying(compressionMethod));

        if (const auto* compressor = proto::getCompressor(compressionMethod))
        {
            const std::vector old(payload);

            cyrex::logging::info(LOG_MCBE, "decompressing...");
            payload = *compressor->decompress({old.data() + 1, old.size() - 1});

            cyrex::logging::info(LOG_MCBE, "decompressed size = {}", payload.size());
        }
        else
        {
            session->onRaw(*p, payload.data() + 1, payload.size() - 1);
            cyrex::logging::info(LOG_MCBE, "compression inactive.");
            return;
        }
    }

    session->onRaw(*p, payload.data(), payload.size());
}

} // namespace cyrex::nw::raknet
