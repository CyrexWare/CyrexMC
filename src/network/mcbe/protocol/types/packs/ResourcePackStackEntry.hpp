#pragma once
#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"

#include <string>

namespace cyrex::nw::protocol
{

struct ResourcePackStackEntry
{
    std::string packId;
    std::string packVersion;
    std::string subPackName;

    void encode(io::BinaryWriter& out) const
    {
        out.writeString(packId);
        logging::info("packId (StackEntry): {}", packId);
        out.writeString(packVersion);
        out.writeString(subPackName);
    }

    void decode(io::BinaryReader& in)
    {
        packId = in.readString();
        packVersion = in.readString();
        subPackName = in.readString();
    }
};

} // namespace cyrex::nw::protocol
