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
    //SKIP 0xFE
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
    if (session->encryptionEnabled)
    {
        //TODO: uh hmm
    }

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

        if (compressionMethod == static_cast<uint8_t>(mcpe::protocol::types::CompressionAlgorithm::NONE))
        {
            cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_DEBUG,
                                           cyrex::text::format::Builder()
                                               .color(text::format::Color::DARK_GRAY)
                                               .text("compression inactive")
                                               .build());
            payload.assign(body, body + bodyLen);
        }
        else if (
            compressionMethod == static_cast<uint8_t>(mcpe::protocol::types::CompressionAlgorithm::ZLIB)
            || compressionMethod == static_cast<uint8_t>(mcpe::protocol::types::CompressionAlgorithm::SNAPPY))
        {
            if (compressionMethod != static_cast<uint8_t>(session->compressor().networkId()))
            {
                cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_ERROR,
                                               cyrex::text::format::Builder()
                                                   .color(text::format::Color::DARK_GRAY)
                                                   .text("wrong decompression alg!")
                                                   .build());
                return;
            }
            cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_DEBUG,
                                           cyrex::text::format::Builder()
                                               .color(text::format::Color::DARK_GRAY)
                                               .text("decompressing...")
                                               .build());
            mcbe::compression::CompressionStatus const status = session->compressor().decompress(body, bodyLen, payload);
            if (status == mcbe::compression::CompressionStatus::FAILED)
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

    session->onRaw(*p, payload.data(), payload.size());
}
