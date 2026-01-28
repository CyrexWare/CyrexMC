#pragma once

#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/packetbase.hpp"

#include <functional>
#include <memory>
#include <unordered_map>

namespace cyrex::network::mcbe
{

class PacketPool
{
public:
    using Factory = std::function<std::unique_ptr<PacketBase>()>;

    struct Entry
    {
        Factory factory{};
        PacketDirection direction{PacketDirection::Bidirectional};
    };


    static PacketPool& instance()
    {
        static PacketPool p;
        return p;
    }

    void bind(uint32_t id, Factory f, PacketDirection dir)
    {
        entries[id] = {std::move(f), dir};
    }

    std::unique_ptr<PacketBase> create(uint32_t id) const
    {
        auto it = entries.find(id);
        if (it == entries.end())
            return nullptr;
        return it->second.factory();
    }

    PacketDirection direction(uint32_t id) const
    {
        auto it = entries.find(id);
        if (it == entries.end())
            return PacketDirection::Bidirectional;
        return it->second.direction;
    }

    void registerAll();

private:
    std::unordered_map<uint32_t, Entry> entries;
};
} // namespace cyrex::network::mcbe
