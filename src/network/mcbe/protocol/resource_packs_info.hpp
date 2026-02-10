#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"
#include "network/mcbe/resourcepacks/resource_pack_def.hpp"
#include "network/mcbe/resourcepacks/resource_pack.hpp"

#include <memory>
#include <string>
#include <vector>

namespace cyrex::nw::protocol
{

class ResourcePacksInfoPacket final :
    public PacketImpl<ResourcePacksInfoPacket, static_cast<uint32_t>(PacketId::ResourcePacksInfo), PacketDirection::Clientbound, true>
{
public:
    bool mustAccept = false;
    bool hasAddonPacks = false;
    bool scripting = false;

    bool disableVibrantVisuals = false;

    io::UUID worldTemplateId{};
    std::string worldTemplateVersion;

    std::vector<std::shared_ptr<cyrex::nw::resourcepacks::ResourcePackDef>> resourcePackEntries;

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        out.writeBool(mustAccept);
        out.writeBool(hasAddonPacks);
        out.writeBool(scripting);
        out.writeBool(disableVibrantVisuals);

        out.writeUUID(worldTemplateId);
        out.writeString(worldTemplateVersion);

        encodePacks(out, resourcePackEntries);
        return true;
    }

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        // NOOP
        return false;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return true;
        //return session.handleResourcePacksInfo(*this);
    }

private:
    void encodePacks(cyrex::nw::io::BinaryWriter& out,
                     const std::vector<std::shared_ptr<cyrex::nw::resourcepacks::ResourcePackDef>>& packs) const
    {
        out.writeU16LE(static_cast<uint16_t>(packs.size()));

        for (const auto& entry : packs)
        {
            out.writeUUID(entry->getPackId());
            //{
            //    const auto& uuid = entry->getPackId();
            //    std::ostringstream oss;
            //    oss << "packId raw =";
            //    for (auto b : uuid) // raw 16 bytes
            //        oss << " " << static_cast<int>(b);
            //    logging::log(oss.str());
            //}
            out.writeString(entry->getPackVersion());
            logging::log("packVersion raw =", entry->getPackVersion());
            out.writeU64LE(static_cast<uint64_t>(entry->getPackSize()));
            logging::log("packSize raw =", entry->getPackSize());
            out.writeString(entry->getEncryptionKey());
            logging::log("encryptionKey raw =", entry->getEncryptionKey());

            out.writeString(entry->getSubPackName());
            logging::log("subPackName raw =", entry->getSubPackName());

            //// Write and log encryptedPackId (UUID only if encryption key exists)
            //auto encryptedId = !entry->getEncryptionKey().empty() ? entry->getPackId() : UUID{};
            out.writeString(!entry->getEncryptionKey().empty() ? uuidToString(entry->getPackId()) : "");
            //{
            //    const auto& uuid = encryptedId;
            //    std::ostringstream oss;
            //    oss << "encryptedPackId raw =";
            //    for (auto b : uuid.bytes)
            //        oss << " " << static_cast<int>(b);
            //    logging::log(oss.str());
            //}
            out.writeBool(entry->usesScript());
            logging::log("usesScript raw =", entry->usesScript() ? 1 : 0);

            out.writeBool(entry->isAddonPack());
            logging::log("isAddonPack raw =", entry->isAddonPack() ? 1 : 0);

            out.writeBool(entry->isRaytracingCapable());
            logging::log("isRaytracingCapable raw =", entry->isRaytracingCapable() ? 1 : 0);
            out.writeString(entry->cdnUrl());
            logging::log("cdnUrl raw =", entry->cdnUrl());
        }
    }

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
