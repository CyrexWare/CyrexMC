#include "performance_command.hpp"

#include "command/permission.hpp"
#include "log/console_logger.hpp"
#include "log/message_type.hpp"
#include "text/format/builder.hpp"
#include "text/format/color.hpp"
#include "util/performance.hpp"

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
    cyrex::log::sendConsoleMessage(cyrex::log::MessageType::INFO,
                                   cyrex::text::format::Builder()
                                       .color(text::format::Color::AQUA)
                                       .text(cyrex::util::getReport())
                                       .build());
}
