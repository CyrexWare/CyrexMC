#include "actor.hpp"

namespace cyrex::actor
{

std::atomic<Actor::ActorId> Actor::s_nextId{0};

Actor::Actor() : m_id(s_nextId.fetch_add(1, std::memory_order_relaxed))
{
}

Actor::ActorId Actor::getId() const noexcept
{
    return m_id;
}

} // namespace cyrex::actor
