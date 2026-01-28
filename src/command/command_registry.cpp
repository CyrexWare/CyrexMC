#include "command_registry.hpp"

#include "command_base.hpp"

void cyrex::command::CommandRegistry::registerCommand(std::unique_ptr<CommandBase> command)
{
    CommandBase* ptr = command.get();

    m_lookup[ptr->name()] = ptr;
    for (const auto& alias : ptr->aliases())
        m_lookup[alias] = ptr;

    m_storage.push_back(std::move(command));
}

cyrex::command::CommandBase* cyrex::command::CommandRegistry::find(const std::string& name) const
{
    auto it = m_lookup.find(name);
    return it == m_lookup.end() ? nullptr : it->second;
}
