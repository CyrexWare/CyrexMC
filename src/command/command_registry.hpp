#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cyrex::command
{

class CommandBase;

class CommandRegistry
{
public:
    void registerCommand(std::unique_ptr<CommandBase> command);
    CommandBase* find(const std::string& name) const;

private:
    std::unordered_map<std::string, CommandBase*> m_lookup;
    std::vector<std::unique_ptr<CommandBase>> m_storage;
};
} // namespace cyrex::command
