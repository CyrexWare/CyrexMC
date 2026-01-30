#include "mcbe_packet_router.hpp"

#include "log/console_logger.hpp"
#include "log/message_type.hpp"
#include "network/session/network_session.hpp"
#include "text/format/builder.hpp"
#include "text/format/color.hpp"

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

    //std::cout << renderConsole(bedrock(Color::GREEN) + "[MCBE]", true)
    //          << renderConsole(bedrock(Color::DARK_GRAY) + " raw payload ", false) << "(" << len << " bytes) : ";

    //for (size_t i = 0; i < len; i++)
    //    std::printf("%02X ", data[i]);
    //std::cout << std::endl;

    std::vector<uint8_t> payload;

    if (!session->compressionEnabled)
    {
        cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_DEBUG,
                                       cyrex::text::format::Builder()
                                           .color(text::format::Color::DARK_GRAY)
                                           .text("compression inactive")
                                           .build());
        payload.assign(data, data + len);
    }
    else
    {
        const uint8_t compressionMethod = data[0];

        cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_DEBUG,
                                       cyrex::text::format::Builder()
                                           .color(text::format::Color::DARK_GRAY)
                                           .text(std::format("compression method = 0x{:02X}", compressionMethod))
                                           .build());

        const uint8_t* body = data + 1;
        const size_t bodyLen = len - 1;

        if (compressionMethod == 0xFF)
        {
            cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_DEBUG,
                                           cyrex::text::format::Builder()
                                               .color(text::format::Color::DARK_GRAY)
                                               .text("compression inactive")
                                               .build());
            payload.assign(body, body + bodyLen);
        }
        else if (compressionMethod == 0x00)
        {
            cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_DEBUG,
                                           cyrex::text::format::Builder()
                                               .color(text::format::Color::DARK_GRAY)
                                               .text("ZLIB decompressing...")
                                               .build());
            if (!session->compressor().decompress(body, bodyLen, payload))
            {
                cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_ERROR,
                                               cyrex::text::format::Builder()
                                                   .color(text::format::Color::DARK_GRAY)
                                                   .text("failed to decompress!")
                                                   .build());
                return;
            }

            cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_DEBUG,
                                           cyrex::text::format::Builder()
                                               .color(text::format::Color::DARK_GRAY)
                                               .text("decompressed size = " + std::to_string(payload.size()))
                                               .build());
        }
        else
        {
            cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_ERROR,
                                           cyrex::text::format::Builder()
                                               .color(text::format::Color::DARK_GRAY)
                                               .text(&"unknwon compression method = "[(int)compressionMethod])
                                               .build());
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
