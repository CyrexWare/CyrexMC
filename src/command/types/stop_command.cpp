#include "stop_command.hpp"

#include "command/command_context.hpp"
#include "command/permission.hpp"
#include "server.hpp"

std::string cyrex::command::types::StopCommand::name() const
{
    return "stop";
}

std::vector<std::string> cyrex::command::types::StopCommand::aliases() const
{
    return {"shutdown", "exit"};
}

std::string cyrex::command::types::StopCommand::description() const
{
    return "Stops the server";
}

cyrex::command::Permission cyrex::command::types::StopCommand::permission() const
{
    return {"server.stop", PermissionLevel::CONSOLE};
}

void cyrex::command::types::StopCommand::execute(CommandContext& ctx)
{
    ctx.server.stop();
}
