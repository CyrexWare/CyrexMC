#include "performance_command.hpp"

#include "command/permission.hpp"
#include "util/performance.hpp"
#include "util/textformat.hpp"

#include <iostream>

std::string cyrex::command::types::PerformanceCommand::name() const
{
    return "performance";
}

std::string cyrex::command::types::PerformanceCommand::description() const
{
    return "Shows server performance stats";
}

cyrex::command::Permission cyrex::command::types::PerformanceCommand::permission() const
{
    return {"server.performance", PermissionLevel::OP};
}

void cyrex::command::types::PerformanceCommand::execute(CommandContext&)
{
    std::cout << cyrex::util::renderConsole(cyrex::util::getReport(), false) << std::endl;
}
