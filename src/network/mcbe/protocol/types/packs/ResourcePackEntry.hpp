#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/resourcepacks/resource_pack_def.hpp"

#include <string>

namespace cyrex::nw::protocol
{

struct ResourcePackEntry
{
    io::UUID packId{};
    std::string packVersion;
    int64_t packSize = 0;
    std::string encryptionKey;
    std::string subPackName;
    std::string contentIdentity;
    bool scripting = false;
    bool addonPack = false;
    bool raytracingCapable = false;
    std::string cdnUrl;

    ResourcePackEntry() = default;

    explicit ResourcePackEntry(const resourcepacks::ResourcePackDef& pack)
    {
        packId = pack.getPackId();
        packVersion = pack.getPackVersion();
        packSize = pack.getPackSize();
        encryptionKey = pack.getEncryptionKey();
        subPackName = pack.getSubPackName();
        contentIdentity = !encryptionKey.empty() ? uuidToString(packId) : "";
        scripting = pack.usesScript();
        addonPack = pack.isAddonPack();
        raytracingCapable = pack.isRaytracingCapable();
        cdnUrl = pack.cdnUrl();
    }

    void encode(cyrex::nw::io::BinaryWriter& out) const
    {
        out.writeUUID(packId);
        out.writeString(packVersion);
        out.writeI16LE(packSize);
        out.writeString(encryptionKey);
        out.writeString(subPackName);
        out.writeString(contentIdentity);
        out.writeBool(scripting);
        out.writeBool(addonPack);
        out.writeBool(raytracingCapable);
        out.writeString(cdnUrl);
    }

    void decode(cyrex::nw::io::BinaryReader& in)
    {
        packId = in.readUUID();
        packVersion = in.readString();
        packSize = in.readI16LE();
        encryptionKey = in.readString();
        subPackName = in.readString();
        contentIdentity = in.readString();
        scripting = in.readBool();
        addonPack = in.readBool();
        raytracingCapable = in.readBool();
        cdnUrl = in.readString();
    }

private:
    static std::string uuidToString(const io::UUID& u)
    {
        std::string str;
        char buf[3]{};
        for (auto b : u)
        {
            std::snprintf(buf, sizeof(buf), "%02x", b);
            str += buf;
        }
        return str;
    }
};

} // namespace cyrex::nw::protocol
