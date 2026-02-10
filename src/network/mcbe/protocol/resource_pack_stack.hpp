#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/session/network_session.hpp"
#include "resource_pack_packet_impl.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackEntry.hpp"

#include <vector>

namespace cyrex::nw::protocol
{

class ResourcePackStackPacket final :
    public PacketImpl<ResourcePackStackPacket, static_cast<uint32_t>(PacketId::ResourcePackStack), PacketDirection::Clientbound, true>
{
public:
    using PacketImpl<ResourcePackStackPacket, static_cast<uint32_t>(PacketId::ResourcePackStack), PacketDirection::Clientbound, true>::getDefStatic;
    std::vector<ResourcePackEntry> resourcePackStack;
    bool mustAccept = false;

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        out.writeBool(mustAccept);
        out.writeU16LE(static_cast<uint16_t>(resourcePackStack.size()));

        for (const auto& entry : resourcePackStack)
        {
            entry.encode(out);
        }

        return true;
    }

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        mustAccept = in.readBool();
        uint16_t len = in.readU16LE();
        resourcePackStack.resize(len);

        for (auto& entry : resourcePackStack)
        {
            entry.decode(in);
        }

        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return true;
        //return session.handleResourcePackStack(*this);
    }
};

} // namespace cyrex::nw::protocol
