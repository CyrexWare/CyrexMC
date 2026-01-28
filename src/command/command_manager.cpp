#include "command_manager.hpp"

#include "command/types/performance_command.hpp"
#include "command/types/stop_command.hpp"
#include "command_context.hpp"
#include "server.hpp"
#include "util/textformat.hpp"

#include <iostream>
#include <sstream>

cyrex::command::CommandManager::CommandManager(cyrex::Server& server) : m_server(server)
{
}

void cyrex::command::CommandManager::registerDefaults()
{
    registerCommand(std::make_unique<cyrex::command::types::StopCommand>());
    registerCommand(std::make_unique<cyrex::command::types::PerformanceCommand>());
}

void cyrex::command::CommandManager::registerCommand(std::unique_ptr<CommandBase> command)
{
    m_registry.registerCommand(std::move(command));
}

bool cyrex::command::CommandManager::hasPermission(PermissionLevel source, const Permission& required) const
{
    return static_cast<int>(source) >= static_cast<int>(required.level);
}

void cyrex::command::CommandManager::executeConsole(const std::string& line)
{
    std::istringstream ss(line);
    std::string name;
    ss >> name;

    if (name.empty())
        return;

    std::vector<std::string> args;
    std::string arg;
    while (ss >> arg)
        args.push_back(arg);

    auto* cmd = m_registry.find(name);
    if (!cmd)
    {
        std::cout << cyrex::util::renderConsole(cyrex::util::bedrock(cyrex::util::Color::RED) +
                                                    cyrex::util::bedrock(cyrex::util::Style::BOLD) +
                                                    "[Cyrex] The command you have entered does not exist." +
                                                    cyrex::util::bedrock(cyrex::util::Style::RESET),
                                                false)
                  << std::endl;
        return;
    }

    PermissionLevel sourceLevel = PermissionLevel::CONSOLE;

    if (!hasPermission(sourceLevel, cmd->permission()))
    {
        std::cout << cyrex::util::renderConsole(cyrex::util::bedrock(cyrex::util::Color::GOLD) +
                                                    cyrex::util::bedrock(cyrex::util::Style::BOLD) +
                                                    "[Cyrex] You do not have permission to use this command." +
                                                    cyrex::util::bedrock(cyrex::util::Style::RESET),
                                                false)
                  << std::endl;
        return;
    }

    CommandContext ctx{m_server, line, args, sourceLevel};
    cmd->execute(ctx);
}
