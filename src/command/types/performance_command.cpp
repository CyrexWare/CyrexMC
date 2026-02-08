#include "performance_command.hpp"

#include "command/permission.hpp"
#include "log/logging.hpp"
#include "util/performance.hpp"

#include <iostream>

std::string cyrex::command::PerformanceCommand::name() const
{
    return "performance";
}

std::string cyrex::command::PerformanceCommand::description() const
{
    return "Shows server performance stats";
}

cyrex::command::Permission cyrex::command::PerformanceCommand::permission() const
{
    return {"server.performance", PermissionLevel::OP};
}

void cyrex::command::PerformanceCommand::execute(CommandContext&)
{
    cyrex::util::logServerStatus();
}
