#include "mcbe_packet_router.hpp"

#include "log/logging.hpp"
#include "network/mcbe/compression/compressors.hpp"
#include "network/session/network_session.hpp"

#include <iostream>
#include <utility>

using namespace cyrex::nw::session;

void cyrex::nw::raknet::McbePacketRouter::route(RakNet::Packet* p, raknet::RaknetConnections& connections)
{
    session::NetworkSession* session = connections.get(p->guid);
    if (!session)
        return;
    //SKIP 0xFE
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
            logging::error(LOG_MCBE, "failed to decrypt!");
            return;
        }
    }
    else
    {
        payload.assign(data, data + len);
    }

    if (!session->compressionEnabled)
    {
        logging::info(LOG_MCBE, "compression inactive.");
    }
    else
    {
        const auto compressionMethod = static_cast<proto::CompressionAlgorithm>(payload.front());
        ;
        logging::info(LOG_MCBE, "compression method = 0x{:02X}", std::to_underlying(compressionMethod));
        if (const auto* compressor = proto::getCompressor(compressionMethod))
        {
            const std::vector old(payload);
            logging::info(LOG_MCBE, "decompressing...");
            payload = *compressor->decompress({old.data() + 1, old.size() - 1});
            logging::info(LOG_MCBE, "decompressed size = {}", payload.size());
        }
        else
        {
            session->onRaw(*p, payload.data() + 1, payload.size() - 1);
            logging::info(LOG_MCBE, "compression inactive.");
            return;
        }
    }

    session->onRaw(*p, payload.data(), payload.size());
}
