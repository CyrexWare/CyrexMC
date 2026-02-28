#include "command/types/server_info_command.hpp"

#include "command/command_context.hpp"
#include "info.hpp"
#include "log/color.hpp"
#include "log/level.hpp"
#include "log/logging.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "server.hpp"

namespace cyrex::command
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
    logging::info("{}",
                  std::format("\n{}=== CyrexMC Server Information ==={}\n"
                              "{}Build Version:{} {} ({})\n"
                              "{}Supported Game Version:{} {}\n"
                              "{}Players:{} {}/{}\n"
                              "{}Port:{} {}\n",
                              logging::Color::BLUE,
                              logging::Color::GOLD,

                              logging::Color::WHITE,
                              logging::Color::GOLD,
                              Info::version().toString(),
                              Info::buildTypeString(),

                              logging::Color::WHITE,
                              logging::Color::GOLD,
                              network::protocol::ProtocolInfo::minecraftVersion,

                              logging::Color::WHITE,
                              logging::Color::GOLD,
                              ctx.server.getPlayerCount(),
                              ctx.server.getMaxPlayers(),

                              logging::Color::WHITE,
                              logging::Color::GOLD,
                              ctx.server.getPort()));
}

} // namespace cyrex::command