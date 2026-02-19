#pragma once
#include <cstdint>

namespace cyrex::nw::protocol
{
enum class ViolationSeverity : int32_t
{
    Warning,
    FinalWarning,
    TerminatingConnection
};
} // namespace cyrex::nw::protocol