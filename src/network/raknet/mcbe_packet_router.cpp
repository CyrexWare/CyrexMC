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

    const uint8_t* data = p->data + 1;
    const size_t len = p->length - 1;

    if (len == 0)
        return;

    std::vector<uint8_t> payload;

    if (!session->compressionEnabled)
    {
        cyrex::logging::info(LOG_MCBE, "compression inactive.");
        payload.assign(data, data + len);
    }
    else
    {
        const uint8_t compressionMethod = data[0];

        cyrex::logging::info(LOG_MCBE, "compression method = 0x{:02X}", compressionMethod);

        const uint8_t* body = data + 1;
        const size_t bodyLen = len - 1;

        if (compressionMethod == 0xFF)
        {
            cyrex::logging::info(LOG_MCBE, "compression inactive.");
            payload.assign(body, body + bodyLen);
        }
        else if (compressionMethod == 0x00)
        {
            cyrex::logging::info(LOG_MCBE, "ZLIB decompressing...");
            if (!session->compressor().decompress(body, bodyLen, payload))
            {
                cyrex::logging::error(LOG_MCBE, "failed to decompress!");
                return;
            }

            cyrex::logging::info(LOG_MCBE, "decompressed size = {}", payload.size());
        }
        else
        {
            cyrex::logging::error(LOG_MCBE, "unknwon compression method = {}", compressionMethod);
            return;
        }
    }

    session->onRaw(*p, payload.data(), payload.size());
}
