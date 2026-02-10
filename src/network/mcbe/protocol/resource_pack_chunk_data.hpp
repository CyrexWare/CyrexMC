#pragma once
#include "resource_pack_packet_impl.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"

#include <string>
#include <vector>

namespace cyrex::nw::protocol
{

class ResourcePackChunkDataPacket final :
    public PacketImpl<ResourcePackChunkDataPacket, static_cast<uint32_t>(PacketId::ResourcePackChunkData), PacketDirection::Clientbound, true>,
    public ResourcePackDataPacket
{
public:
    using PacketImpl<ResourcePackChunkDataPacket, static_cast<uint32_t>(PacketId::ResourcePackChunkData), PacketDirection::Clientbound, true>::getDefStatic;

    io::UUID packId{};
    std::string packVersion; 
    int chunkIndex = 0;
    uint64_t progress = 0;
    std::vector<uint8_t> data;

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        encodePackInfo(out);
        // check this out
        out.writeI16LE(chunkIndex);
        out.writeU64LE(progress);
        out.writeBytes(data);
        return true;
    }

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        decodePackInfo(in);
        chunkIndex = in.readI16LE();
        progress = in.readU64LE();
        data = in.readBytesVector(in.remaining()); 
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return true;
        //return session.handleResourcePackChunkData(*this);
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
