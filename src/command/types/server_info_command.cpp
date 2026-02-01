#include "command/types/server_info_command.hpp"

#include "command/command_context.hpp"
#include "info.hpp"
#include "log/color.hpp"
#include "log/level.hpp"
#include "log/logging.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "server.hpp"

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
    cyrex::logging::info("{}",
                         std::format("\n{}=== CyrexMC Server Information ==={}\n"
                                     "{}Build Version:{} {} ({})\n"
                                     "{}Supported Game Version:{} {}\n"
                                     "{}Players:{} {}/{}\n"
                                     "{}Port:{} {}\n",
                                     cyrex::logging::Color::BLUE,
                                     cyrex::logging::Color::GOLD,

                                     cyrex::logging::Color::WHITE,
                                     cyrex::logging::Color::GOLD,
                                     cyrex::Info::version().toString(),
                                     cyrex::Info::buildTypeString(),

                                     cyrex::logging::Color::WHITE,
                                     cyrex::logging::Color::GOLD,
                                     cyrex::network::mcbe::protocol::ProtocolInfo::minecraftVersion,

                                     cyrex::logging::Color::WHITE,
                                     cyrex::logging::Color::GOLD,
                                     ctx.server.getPlayerCount(),
                                     ctx.server.getMaxPlayers(),

                                     cyrex::logging::Color::WHITE,
                                     cyrex::logging::Color::GOLD,
                                     ctx.server.getPort()));
}

} // namespace cyrex::command::types