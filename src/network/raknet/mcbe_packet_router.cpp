#include "mcbe_packet_router.hpp"

#include "network/session/network_session.hpp"
#include "util/textformat.hpp"

#include <iostream>
using namespace cyrex::network::session;
using namespace cyrex::util;

void cyrex::network::raknet::McbePacketRouter::route(RakNet::Packet* p, cyrex::network::raknet::RaknetConnections& connections)
{
    cyrex::network::session::NetworkSession* session = connections.get(p->guid);
    if (!session)
        return;

    const uint8_t* data = p->data + 1;
    size_t const len = p->length - 1;

    if (len == 0)
        return;

    //std::cout << renderConsole(bedrock(Color::GREEN) + "[MCBE]", true)
    //          << renderConsole(bedrock(Color::DARK_GRAY) + " raw payload ", false) << "(" << len << " bytes) : ";

    //for (size_t i = 0; i < len; i++)
    //    std::printf("%02X ", data[i]);
    //std::cout << std::endl;

    std::vector<uint8_t> payload;

    if (!session->compressionEnabled)
    {
        std::cout << renderConsole(bedrock(Color::GREEN) + "[MCBE]", true)
                  << renderConsole(bedrock(Color::GRAY) + " compression DISABLED", false) << std::endl;

        payload.assign(data, data + len);
    }
    else
    {
        uint8_t const compressionMethod = data[0];

        std::cout << renderConsole(bedrock(Color::GREEN) + "[MCBE][DEBUG]", true)
                  << renderConsole(bedrock(Color::DARK_GRAY) + " compression method = 0x", false) << std::hex
                  << (int)compressionMethod << std::dec << std::endl;

        const uint8_t* body = data + 1;
        size_t const bodyLen = len - 1;

        if (compressionMethod == 0xFF)
        {
            std::cout << renderConsole(bedrock(Color::GREEN) + "[MCBE][DEBUG]", true)
                      << renderConsole(bedrock(Color::GRAY) + " no compression", false) << std::endl;

            payload.assign(body, body + bodyLen);
        }
        else if (compressionMethod == 0x00)
        {
            std::cout << renderConsole(bedrock(Color::GREEN) + "[MCBE][DEBUG]", true)
                      << renderConsole(bedrock(Color::GRAY) + " ZLIB decompressing...", false) << std::endl;

            if (!session->compressor().decompress(body, bodyLen, payload))
            {
                std::cout << renderConsole(bedrock(Color::RED) + "[MCBE][ERROR]", true)
                          << renderConsole(bedrock(Color::DARK_GRAY) + " decompression failed", false) << std::endl;
                return;
            }

            std::cout << renderConsole(bedrock(Color::GREEN) + "[MCBE][DEBUG]", true)
                      << renderConsole(bedrock(Color::GRAY) + " decompressed size = ", false) << payload.size()
                      << std::endl;
        }
        else
        {
            std::cout << renderConsole(bedrock(Color::RED) + "[MCBE][ERROR]", true)
                      << renderConsole(bedrock(Color::DARK_GRAY) + " unknown compression method = ", false)
                      << (int)compressionMethod << std::endl;
            return;
        }
    }

    //std::cout << renderConsole(bedrock(Color::GREEN) + "[MCBE][DEBUG]", true)
    //          << renderConsole(bedrock(Color::GRAY) + " final payload: ", false);

    /*   for (uint8_t b : payload)
        std::printf("%02X ", b);
    std::cout << std::endl;*/

    session->onRaw(*p, payload.data(), payload.size());
}
