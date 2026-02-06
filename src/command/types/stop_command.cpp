#include "stop_command.hpp"

#include "command/command_context.hpp"
#include "command/permission.hpp"
#include "server.hpp"

std::string cyrex::command::StopCommand::name() const
{
    return "stop";
}

std::vector<std::string> cyrex::command::StopCommand::aliases() const
{
    return {"shutdown", "exit"};
}

std::string cyrex::command::StopCommand::description() const
{
    return "Stops the server";
}

cyrex::command::Permission cyrex::command::StopCommand::permission() const
{
    return {"server.stop", PermissionLevel::CONSOLE};
}

void cyrex::command::StopCommand::execute(CommandContext& ctx)
{
    ctx.server.stop();
}
