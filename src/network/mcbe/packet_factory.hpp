#pragma once

#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_def.hpp"

#include <functional>
#include <memory>
#include <unordered_map>

#include <cassert>

template <typename Derived, typename Base>
std::unique_ptr<Derived> dynamicPointerCastUnique(std::unique_ptr<Base>&& ptr) noexcept
{
    if (!ptr)
        return nullptr;

    if (auto derived = dynamic_cast<Derived*>(ptr.get()))
    {
        ptr.release();
        return std::unique_ptr<Derived>(derived);
    }

    return nullptr;
}

namespace cyrex::nw::protocol
{

class PacketFactory
{
public:
    template <typename T>
    void add()
    {
        const auto& def = T::getDefStatic();
        if (m_entries.contains(def.networkId))
        {
            m_entries.erase(def.networkId);
        }
        m_entries.emplace(def.networkId, &def);
    }

    [[nodiscard]] const PacketDef* find(const uint32_t id) const
    {
        const auto it = m_entries.find(id);
        if (it == m_entries.end())
        {
            return nullptr;
        }

        return it->second;
    }

    [[nodiscard]] const PacketDef& get(const uint32_t id) const
    {
        const auto ptr = find(id);
        assert(ptr);
        return *ptr;
    }

    void remove(const uint32_t id)
    {
        if (m_entries.contains(id))
        {
            m_entries.erase(id);
        }
    }

    [[nodiscard]] std::unique_ptr<Packet> create(const uint32_t id) const
    {
        const auto def = find(id);
        if (!def)
        {
            return {};
        }

        return def->create();
    }

    void registerAll();

private:
    std::unordered_map<uint32_t, const PacketDef*> m_entries;
};
} // namespace cyrex::nw::protocol
