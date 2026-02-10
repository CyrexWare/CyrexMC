#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackDataTypes.hpp"
#include "network/mcbe/resourcepacks/resource_pack.hpp"
#include "network/mcbe/resourcepacks/resource_pack_def.hpp"
#include "network/session/network_session.hpp"
#include "resource_pack_packet_impl.hpp"
#include "log/logging.hpp"

#include <string>
#include <vector>

namespace cyrex::nw::protocol
{

class ResourcePackDataInfoPacket final :
    public PacketImpl<ResourcePackDataInfoPacket, static_cast<uint32_t>(PacketId::ResourcePackDataInfo), PacketDirection::Clientbound, true>
{
public:
    using PacketImplType = PacketImpl<ResourcePackDataInfoPacket,
                                      static_cast<uint32_t>(PacketId::ResourcePackDataInfo),
                                      PacketDirection::Clientbound,
                                      true>;
    using PacketImplType::getDefStatic;

    io::UUID packId{};
    std::string packVersion;
    int maxChunkSize = 0;
    int chunkCount = 0;
    uint64_t compressedPackSize = 0;
    std::vector<uint8_t> sha256;
    bool premium = false;
    ResourcePackDataType type = ResourcePackDataType::Resource;

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        encodePackInfo(out);
        out.writeI32LE(maxChunkSize);
        out.writeI32LE(chunkCount);
        out.writeU64LE(compressedPackSize);
        out.writeBytes(sha256);
        out.writeBool(premium);
        out.writeU8(static_cast<uint8_t>(type));
        logging::info("maxChunkSize={},chunkCount={},compressedPackSize={},sha256={},premium={}", maxChunkSize, chunkCount, compressedPackSize, sha256, premium);
        return true;
    }

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        decodePackInfo(in);
        maxChunkSize = in.readI16LE();
        chunkCount = in.readI16LE();
        compressedPackSize = in.readU64LE();
        sha256 = in.readBytesVector(32);
        premium = in.readBool();
        type = static_cast<ResourcePackDataType>(in.readU8());
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return true; // placeholder
    }

    // Inline "ResourcePackDataPacket interface"
    std::string getPackVersionStr() const
    {
        return packVersion;
    }
    void setPackVersionStr(const std::string& version)
    {
        packVersion = version;
    }

    io::UUID getPackId() const
    {
        return packId;
    }
    void setPackId(const io::UUID& id)
    {
        packId = id;
    }

private:
    // Pack info helpers
    void encodePackInfo(cyrex::nw::io::BinaryWriter& out) const
    {
        out.writeUUID(packId);
        out.writeString(packVersion);

    }

    void decodePackInfo(cyrex::nw::io::BinaryReader& in)
    {
        packId = in.readUUID();
        packVersion = in.readString();
    }
};

} // namespace cyrex::nw::protocol
