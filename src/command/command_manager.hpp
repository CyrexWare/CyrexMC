#pragma once

#include "command_base.hpp"
#include "command_registry.hpp"
#include "permission.hpp"

#include <memory>
#include <string>

namespace cyrex
{
class Server;
}

namespace cyrex::command
{

class CommandManager
{
public:
    explicit CommandManager(Server& server);

    void registerDefaults();
    void registerCommand(std::unique_ptr<CommandBase> command);
    void executeConsole(const std::string& line);

private:
    [[nodiscard]] bool hasPermission(PermissionLevel source, const Permission& required) const;

    Server& m_server;
    CommandRegistry m_registry;
};
} // namespace cyrex::command
