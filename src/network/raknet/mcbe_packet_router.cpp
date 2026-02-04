#include "mcbe_packet_router.hpp"

#include "log/logging.hpp"
#include "network/session/network_session.hpp"

#include <iostream>
using namespace cyrex::network::session;

void cyrex::network::raknet::McbePacketRouter::route(RakNet::Packet* p, cyrex::network::raknet::RaknetConnections& connections)
{
    cyrex::network::session::NetworkSession* session = connections.get(p->guid);
    if (!session)
        return;
    //SKIP 0xFE
    const std::uint8_t* data = p->data + 1;
    const std::size_t len = p->length - 1;

    if (len == 0)
        return;

    std::vector<std::uint8_t> payload;
    if (session->encryptionEnabled)
    {
        if (mcbe::encryption::decrypt(session->cipherBlock(), data, len, payload))
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
        const auto compressionMethod = static_cast<mcpe::protocol::types::CompressionAlgorithm>(payload[0]);
        const std::vector old(payload);
        cyrex::logging::info(LOG_MCBE, "compression method = 0x{:02X}", std::to_underlying(compressionMethod));
        switch (compressionMethod)
        {
            case mcpe::protocol::types::CompressionAlgorithm::NONE:
            {
                cyrex::logging::info(LOG_MCBE, "compression inactive.");
                break;
            }
            case mcpe::protocol::types::CompressionAlgorithm::ZLIB:
            case mcpe::protocol::types::CompressionAlgorithm::SNAPPY:
            {
                if (compressionMethod != session->compressor().networkId())
                {
                    cyrex::logging::error(LOG_MCBE, "invalid decompression alg...");
                    return;
                }
                cyrex::logging::info(LOG_MCBE, "decompressing...");

                const mcbe::compression::CompressionStatus status = session->compressor().decompress(old.data() + 1, old.size() - 1, payload);
                if (status == mcbe::compression::CompressionStatus::FAILED)
                {
                    cyrex::logging::error(LOG_MCBE, "failed to decompress!");
                    return;
                }
                cyrex::logging::info(LOG_MCBE, "decompressed size = {}", payload.size());
                break;
            }
            default:
                cyrex::logging::error(LOG_MCBE, "unknown compression method = {}", payload[0]);
                return;
        }
    }

    session->onRaw(*p, payload.data(), payload.size());
}
