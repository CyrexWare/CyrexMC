#pragma once
#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/mcbe/resourcepacks/resource_pack_def.hpp"
#include "network/session/network_session.hpp"

#include <array>
#include <sstream>
#include <string>

namespace cyrex::nw::protocol
{

class ResourcePackDataPacket
{
public:
    virtual ~ResourcePackDataPacket() = default;

    virtual std::string getPackVersionStr() const = 0;
    virtual void setPackVersionStr(const std::string& version) = 0;

    virtual io::UUID getPackId() const = 0;
    virtual void setPackId(const io::UUID& id) = 0;

protected:
    void decodePackInfo(cyrex::nw::io::BinaryReader& in)
    {
        std::string packInfo = in.readString();
        auto underscorePos = packInfo.find('_');

        std::string idPart = (underscorePos != std::string::npos) ? packInfo.substr(0, underscorePos) : packInfo;
        std::string versionPart = (underscorePos != std::string::npos) ? packInfo.substr(underscorePos + 1) : "";

        try
        {
            setPackId(io::stringToUUID(idPart));
        } catch (...)
        {
            setPackId(io::UUID{});
        }

        setPackVersionStr(versionPart);
    }

    void encodePackInfo(cyrex::nw::io::BinaryWriter& out) const
    {
        auto packId = getPackId();

        out.writeString(io::uuidToString(packId));
    }
};

} // namespace cyrex::nw::protocol
