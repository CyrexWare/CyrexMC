#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackInfoEntry.hpp"
#include "network/session/network_session.hpp"

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

    util::UUID worldTemplateId{};
    std::string worldTemplateVersion;

    std::vector<ResourcePackInfoEntry> resourcePackEntries;

    bool encodePayload(io::BinaryWriter& out) const override
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

    bool decodePayload(io::BinaryReader& in) override
    {
        return false;
    }

    bool handle(session::NetworkSession& session) override
    {
        return true;
    }

private:
    void encodePacks(io::BinaryWriter& out, const std::vector<ResourcePackInfoEntry>& packs) const
    {
        out.writeU16LE(static_cast<uint16_t>(packs.size()));

        for (const auto& entry : packs)
        {
            entry.encode(out);
        }
    }
};

} // namespace cyrex::nw::protocol
