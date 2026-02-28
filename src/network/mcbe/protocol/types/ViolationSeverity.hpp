#pragma once
#include <cstdint>

namespace cyrex::network::protocol
{
enum class ViolationSeverity : int32_t
{
    Warning,
    FinalWarning,
    TerminatingConnection
};
} // namespace cyrex::network::protocol