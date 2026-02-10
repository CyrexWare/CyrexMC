#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"
#include "resource_pack_packet_impl.hpp"

namespace cyrex::nw::protocol
{

class ResourcePackChunkRequestPacket final :
    public PacketImpl<ResourcePackChunkRequestPacket, static_cast<uint32_t>(PacketId::ResourcePackChunkRequest), PacketDirection::Clientbound, true>,
    public ResourcePackDataPacket
{
public:
    using PacketImpl<ResourcePackChunkRequestPacket, static_cast<uint32_t>(PacketId::ResourcePackChunkRequest), PacketDirection::Clientbound, true>::getDefStatic;
    
    io::UUID packId{};
    std::string packVersion;
    int chunkIndex = 0;

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        encodePackInfo(out);
        out.writeI16LE(chunkIndex);
        return true;
    }

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        decodePackInfo(in);
        chunkIndex = in.readI16LE();
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return true;
        //return session.handleResourcePackChunkRequest(*this);
    }

    std::string getPackVersionStr() const override
    {
        return packVersion;
    }
    void setPackVersionStr(const std::string& version) override
    {
        packVersion = version;
    }

    io::UUID getPackId() const override
    {
        return packId;
    }
    void setPackId(const io::UUID& id) override
    {
        packId = id;
    }
};

} // namespace cyrex::nw::protocol
