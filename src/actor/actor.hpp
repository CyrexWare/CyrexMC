#pragma once

#include <atomic>

#include <cstdint>

namespace cyrex::actor
{

class Actor
{
public:
    using ActorId = std::uint64_t;

    Actor();
    virtual ~Actor() = default;

    ActorId getId() const noexcept;

private:
    ActorId m_id;
    static std::atomic<ActorId> s_nextId;
};

} // namespace cyrex::actor
