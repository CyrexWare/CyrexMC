#pragma once
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"

#include <string>
#include <vector>

namespace cyrex::nw::protocol
{

struct ExperimentEntry
{
    std::string name;
    bool enabled = false;

    void encode(io::BinaryWriter& out) const
    {
        out.writeString(name);
        out.writeBool(enabled);
    }

    void decode(io::BinaryReader& in)
    {
        name = in.readString();
        enabled = in.readBool();
    }
};

class Experiments
{
public:
    std::vector<ExperimentEntry> activeToggles;
    std::vector<ExperimentEntry> alwaysOnToggles;
    bool wereAnyExperimentsEverToggled = false;

    void encode(cyrex::nw::io::BinaryWriter& out) const
    {
        const uint32_t total = static_cast<uint32_t>(activeToggles.size() + alwaysOnToggles.size());
        out.writeU32LE(total);

        for (const auto& e : activeToggles)
            e.encode(out);

        for (const auto& e : alwaysOnToggles)
            e.encode(out);

        out.writeBool(wereAnyExperimentsEverToggled);
    }

    void decode(cyrex::nw::io::BinaryReader& in)
    {
        const uint32_t total = in.readU32LE();

        activeToggles.clear();
        alwaysOnToggles.clear();
        activeToggles.reserve(total);

        for (uint32_t i = 0; i < total; ++i)
        {
            ExperimentEntry e;
            e.decode(in);
            activeToggles.emplace_back(std::move(e));
        }

        wereAnyExperimentsEverToggled = in.readBool();
    }
};

} // namespace cyrex::nw::protocol
