#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_def.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackClientResponseStatus.hpp"

#include <string>
#include <vector>

namespace cyrex::nw::session
{
class NetworkSession;
}

namespace cyrex::nw::protocol
{

class ResourcePackClientResponsePacket final :
    public PacketImpl<ResourcePackClientResponsePacket, std::to_underlying(PacketId::ResourcePackClientResponse), PacketDirection::Serverbound, false>
{
public:
    struct Entry
    {
        uuid::UUID uuid{};
        std::string version;

        Entry() = default;
        Entry(const uuid::UUID& id, const std::string& ver) : uuid(id), version(ver)
        {
        }
    };

    ResourcePackClientResponseStatus responseStatus{};
    std::vector<Entry> packEntries;

    bool encodePayload(io::BinaryWriter& out) const override
    {
        // NOOP
        return true;
    }

    bool decodePayload(io::BinaryReader& in) override
    {
        responseStatus = static_cast<ResourcePackClientResponseStatus>(in.readU8());
        const uint16_t len = in.readU16LE();
        packEntries.resize(len);

        for (uint16_t i = 0; i < len; ++i)
        {
            std::string s = in.readString();
            if (const auto pos = s.find('_'); pos != std::string::npos)
            {
                uuid::UUID id = uuid::fromString(s.substr(0, pos));
                std::string ver = s.substr(pos + 1);
                packEntries[i] = Entry{id, ver};
            }
        }
        return true;
    }

    bool handle(session::NetworkSession& session) override;
};
} // namespace cyrex::nw::protocol