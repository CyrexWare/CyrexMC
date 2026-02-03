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
    const uint8_t* data = p->data + 1;
    const size_t len = p->length - 1;

    if (len == 0)
        return;

    std::vector<uint8_t> payload;
    if (session->encryptionEnabled)
    {
        //TODO: uh hmm
    }

    if (!session->compressionEnabled)
    {
        cyrex::logging::info(LOG_MCBE, "compression inactive.");
        payload.assign(data, data + len);
    }
    else
    {
        const auto compressionMethod = static_cast<mcpe::protocol::types::CompressionAlgorithm>(data[0]);

        cyrex::logging::info(LOG_MCBE, "compression method = 0x{:02X}", std::to_underlying(compressionMethod));

        const uint8_t* body = data + 1;
        const size_t bodyLen = len - 1;
        switch (compressionMethod)
        {
            case mcpe::protocol::types::CompressionAlgorithm::NONE:
            {
                cyrex::logging::info(LOG_MCBE, "compression inactive.");
                payload.assign(body, body + bodyLen);
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
                const mcbe::compression::CompressionStatus status = session->compressor().decompress(body, bodyLen, payload);
                if (status == mcbe::compression::CompressionStatus::FAILED)
                {
                    cyrex::logging::error(LOG_MCBE, "failed to decompress!");
                    return;
                }
                cyrex::logging::info(LOG_MCBE, "decompressed size = {}", payload.size());
                break;
            }
            default:
                cyrex::logging::error(LOG_MCBE, "unknwon compression method = {}", data[0]);
                return;
        }
    }

    session->onRaw(*p, payload.data(), payload.size());
}
