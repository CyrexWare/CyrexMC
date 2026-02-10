#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"
#include "network/mcbe/resourcepacks/resource_pack_def.hpp"
#include "log/logging.hpp"

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
            setPackId(stringToUUID(idPart));
        } catch (...)
        {
            setPackId(io::UUID{}); 
        }

        setPackVersionStr(versionPart);
    }

    void encodePackInfo(cyrex::nw::io::BinaryWriter& out) const
    {
        auto packId = getPackId();
        auto packVersion = getPackVersionStr();

        std::string packInfo = !isUUIDEmpty(packId) ? uuidToString(packId) : uuidToString(io::UUID{});
        if (!packVersion.empty())
        {
            packInfo += "_" + packVersion;
        }
       
        logging::info("Encoding pack info: id={}, version={}, combined={}", uuidToString(packId), packVersion, packInfo);
        out.writeString(packInfo);
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

    static io::UUID stringToUUID(const std::string& str)
    {
        io::UUID u{};
        if (str.size() != 32 && str.size() != 36)
        {
            return u;
        }
        std::string cleanStr;
        for (char c : str)
        {
            if (c != '-')
                cleanStr += c;
        }
        for (size_t i = 0; i < 16; ++i)
        {
            u[i] = static_cast<uint8_t>(std::stoi(cleanStr.substr(i * 2, 2), nullptr, 16));
        }
        return u;
    }

    static bool isUUIDEmpty(const io::UUID& u)
    {
        for (auto b : u)
        {
            if (b != 0)
                return false;
        }
        return true;
    }
};

} // namespace cyrex::nw::protocol
