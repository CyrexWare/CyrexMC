#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackStackEntry.hpp"
#include "network/session/network_session.hpp"
#include "types/Experiments.hpp"

#include <string>
#include <vector>

namespace cyrex::nw::protocol
{

class ResourcePackStackPacket final :
    public PacketImpl<ResourcePackStackPacket, static_cast<uint32_t>(PacketId::ResourcePackStack), PacketDirection::Clientbound, true>
{
public:
    bool mustAccept = false;
    std::vector<ResourcePackStackEntry> resourcePackStack;
    std::string baseGameVersion;
    Experiments experiments;
    bool includeEditorPacks = false;

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        out.writeBool(mustAccept);
        out.writeVarUInt(static_cast<uint32_t>(resourcePackStack.size()));
        for (const auto& entry : resourcePackStack)
            entry.encode(out);
        out.writeString(baseGameVersion);
        experiments.encode(out);
        out.writeBool(includeEditorPacks);
        return true;
    }

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        // NOOP
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return true;
    }
};

} // namespace cyrex::nw::protocol
