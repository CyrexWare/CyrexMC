#pragma once

#include "assert.h"
#include "network/mcbe/packet_def.hpp"
#include "network/mcbe/packet_direction.hpp"

#include <functional>
#include <memory>
#include <unordered_map>

template <typename Derived, typename Base>
    requires std::is_convertible_v<Derived*, Base*> && std::is_polymorphic_v<Base>
std::unique_ptr<Derived> dynamic_pointer_cast_unique(std::unique_ptr<Base>&& ptr)
{
    Derived* derived_ptr = dynamic_cast<Derived*>(ptr.get());

    if (derived_ptr)
    {
        ptr.release();
        return std::unique_ptr<Derived>(derived_ptr);
    }

    return nullptr;
}

namespace cyrex::network::mcbe
{

class PacketFactory
{
public:
    template <typename T>
    void add()
    {
        auto def = std::make_unique<T>();
        assert(!m_entries.contains(def->networkId));
        m_entries.emplace(def->networkId, std::move(def));
    }

    [[nodiscard]] PacketDef* find(uint32_t id) const
    {
        auto it = m_entries.find(id);
        if (it == m_entries.end())
        {
            return nullptr;
        }

        return it->second.get();
    }

    [[nodiscard]] PacketDef& get(uint32_t id) const
    {
        auto ptr = find(id);
        assert(ptr);
        return *ptr;
    }

    std::unique_ptr<Packet> create(uint32_t id) const
    {
        auto def = find(id);
        if (!def)
        {
            return {};
        }

        return def->create();
    }

    template <typename T>
    auto create() const
    {
        T t;
        return dynamic_pointer_cast_unique<typename T::PacketType>(get(t.networkId).create());
    }


    void registerAll();

private:
    std::unordered_map<uint32_t, std::unique_ptr<PacketDef>> m_entries;
};
} // namespace cyrex::network::mcbe
