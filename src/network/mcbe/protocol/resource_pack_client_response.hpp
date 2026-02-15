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
    public PacketImpl<ResourcePackClientResponsePacket, static_cast<uint32_t>(PacketId::ResourcePackClientResponse), PacketDirection::Serverbound, false>
{
public:
    using PacketImpl<ResourcePackClientResponsePacket,
                     static_cast<uint32_t>(PacketId::ResourcePackClientResponse),
                     PacketDirection::Serverbound,
                     false>::getDefStatic;

    struct Entry
    {
        util::UUID uuid{};
        std::string version;

        Entry() = default;
        Entry(const util::UUID& id, const std::string& ver) : uuid(id), version(ver)
        {
        }
    };

    ResourcePackClientResponseStatus responseStatus{};
    std::vector<Entry> packEntries;

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        // NOOP
        return true;
    }

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        responseStatus = static_cast<ResourcePackClientResponseStatus>(in.readU8());
        uint16_t len = in.readU16LE();
        packEntries.resize(len);

        for (uint16_t i = 0; i < len; ++i)
        {
            std::string s = in.readString();
            auto pos = s.find('_');
            if (pos != std::string::npos)
            {
                util::UUID id = stringToUUID(s.substr(0, pos));
                std::string ver = s.substr(pos + 1);
                packEntries[i] = Entry{id, ver};
            }
        }
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override;

private:
    static util::UUID stringToUUID(const std::string& str)
    {
        if (str.size() != 32 && str.size() != 36)
            return {};

        std::string cleanStr;
        cleanStr.reserve(32);
        for (char c : str)
            if (c != '-')
                cleanStr += c;

        std::array<uint8_t, 16> bytes{};
        for (size_t i = 0; i < 16; ++i)
            bytes[i] = static_cast<uint8_t>(std::stoi(cleanStr.substr(i * 2, 2), nullptr, 16));

        return util::UUID(bytes);
    }
};
} // namespace cyrex::nw::protocol