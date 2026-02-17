#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"

#include <string>
#include <vector>

namespace cyrex::nw::proto
{

struct ResourcePackInfoEntry
{
    uuid::UUID packId{};
    std::string packVersion;
    uint64_t packSize = 0;
    std::string encryptionKey;
    std::string subPackName;
    std::string contentIdentity;
    bool scripting = false;
    bool addonPack = false;
    bool raytracingCapable = false;
    std::string cdnUrl;

    void encode(io::BinaryWriter& out) const
    {
        out.writeUUID(packId);
        out.writeString(packVersion);
        out.writeU64LE(packSize);
        out.writeString(encryptionKey);
        out.writeString(subPackName);
        out.writeString(contentIdentity);
        out.writeBool(scripting);
        out.writeBool(addonPack);
        out.writeBool(raytracingCapable);
        out.writeString(cdnUrl);
    }

    void decode(io::BinaryReader& in)
    {
        packId = in.readUUID();
        packVersion = in.readString();
        packSize = in.readU64LE();
        encryptionKey = in.readString();
        subPackName = in.readString();
        contentIdentity = in.readString();
        scripting = in.readBool();
        addonPack = in.readBool();
        raytracingCapable = in.readBool();
        cdnUrl = in.readString();
    }
};

} // namespace cyrex::nw::proto
