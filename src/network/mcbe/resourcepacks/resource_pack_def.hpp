#pragma once
#include "network/io/binary_reader.hpp"

#include <string>
#include <vector>

namespace cyrex::nw::resourcepacks
{

class ResourcePackDef
{
public:
    virtual ~ResourcePackDef() = default;

    virtual std::string getPackName() const = 0;
    virtual std::string getSubPackName() const
    {
        return "";
    }
    virtual io::UUID getPackId() const = 0;
    virtual std::string getPackVersion() const = 0;
    virtual int getPackSize() const = 0;
    virtual std::vector<uint8_t> getSha256() const = 0;
    virtual std::vector<uint8_t> getPackChunk(int offset, int len) = 0;

    virtual bool isAddonPack() const
    {
        return false;
    }
    virtual std::string cdnUrl() const
    {
        return "";
    }
    virtual bool isRaytracingCapable() const
    {
        return false;
    }
    virtual bool usesScript() const
    {
        return false;
    }
    virtual std::string getEncryptionKey() const
    {
        return "";
    }

    virtual bool operator==(const ResourcePackDef& other) const
    {
        io::UUID a = getPackId();
        io::UUID b = other.getPackId();
        return a == b; 
    }
};

} // namespace cyrex::nw::resourcepacks
