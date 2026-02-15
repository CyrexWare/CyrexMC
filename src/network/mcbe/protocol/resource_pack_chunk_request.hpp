#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"
#include "resource_pack_packet_impl.hpp"

namespace cyrex::nw::protocol
{

class ResourcePackChunkRequestPacket final :
    public PacketImpl<ResourcePackChunkRequestPacket, static_cast<uint32_t>(PacketId::ResourcePackChunkRequest), PacketDirection::Serverbound, true>
{
public:
    using PacketImpl<ResourcePackChunkRequestPacket, static_cast<uint32_t>(PacketId::ResourcePackChunkRequest), PacketDirection::Serverbound, true>::getDefStatic;
    
    io::UUID packId{};
    std::string packVersion;
    int chunkIndex = 0;

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        //NOOP
        return true;
    }

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        std::string packInfo = in.readString();

        const auto sepPos = packInfo.find('_');
        const std::string uuidStr = packInfo.substr(0, sepPos);

        packId = io::stringToUUID(uuidStr);
        packVersion = packInfo.substr(sepPos + 1);

        chunkIndex = in.readI16LE();

        logging::log("Decoded ResourcePackChunkRequestPacket: packId={}, packVersion={}, chunkIndex={}",
                     io::uuidToString(packId),
                     packVersion,
                     chunkIndex);
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session);
};

} // namespace cyrex::nw::protocol
