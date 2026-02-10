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
        io::UUID uuid{};
        std::string version;

        Entry() = default;
        Entry(const io::UUID& id, const std::string& ver) : uuid(id), version(ver)
        {
        }
    };

    ResourcePackClientResponseStatus responseStatus{};
    std::vector<Entry> packEntries;

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        out.writeU8(static_cast<uint8_t>(responseStatus));
        out.writeU16LE(static_cast<uint16_t>(packEntries.size()));
        for (const auto& entry : packEntries)
        {
            std::string s = uuidToString(entry.uuid) + "_" + entry.version;
            out.writeString(s);
        }
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
                io::UUID id = stringToUUID(s.substr(0, pos));
                std::string ver = s.substr(pos + 1);
                packEntries[i] = Entry{id, ver};
            }
        }
        return true;
    }

     bool handle(cyrex::nw::session::NetworkSession& session) override;

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

    static io::UUID stringToUUID(const std::string& str)
    {
        io::UUID u{};
        if (str.size() != 32 && str.size() != 36)
            return u;
        std::string cleanStr;
        for (char c : str)
            if (c != '-')
                cleanStr += c;
        for (size_t i = 0; i < 16; ++i)
        {
            u[i] = static_cast<uint8_t>(std::stoi(cleanStr.substr(i * 2, 2), nullptr, 16));
        }
        return u;
    }
};

} // namespace cyrex::nw::protocol
