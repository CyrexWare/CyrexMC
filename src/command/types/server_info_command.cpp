#include "command/types/server_info_command.hpp"

#include "command/command_context.hpp"
#include "info.hpp"
#include "log/console_logger.hpp"
#include "log/message_type.hpp"
#include "server.hpp"
#include "text/format/builder.hpp"
#include "text/format/color.hpp"

namespace cyrex::command::types
{

std::string ServerInfoCommand::name() const
{
    return "serverinfo";
}

std::vector<std::string> ServerInfoCommand::aliases() const
{
    return {"info", "sinfo"};
}

std::string ServerInfoCommand::description() const
{
    return "Shows server info that may or may not be helpful";
}

Permission ServerInfoCommand::permission() const
{
    return {"server.info", PermissionLevel::OP};
}

void ServerInfoCommand::execute(CommandContext& ctx)
{
    using cyrex::text::format::Builder;
    using cyrex::text::format::Color;

    Builder builder;

    builder.color(Color::GOLD)
        .text("\nCyrex MC Version: ")
        .color(Color::WHITE)
        .text(cyrex::Info::version().toString())
        .text(" (")
        .text(cyrex::Info::buildTypeString())
        .text(")\n");

    builder.color(Color::GOLD)
        .text("Players: ")
        .color(Color::WHITE)
        .text(std::to_string(ctx.server.getPlayerCount()))
        .text("/")
        .text(std::to_string(ctx.server.getMaxPlayers()))
        .text("\n");

    builder.color(Color::GOLD).text("Port: ").color(Color::WHITE).text(std::to_string(ctx.server.getPort())).text("\n");

    cyrex::log::sendConsoleMessage(cyrex::log::MessageType::INFO, builder.build());
}

} // namespace cyrex::command::types