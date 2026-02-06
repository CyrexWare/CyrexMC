#pragma once
#include "server.hpp"

#include <string>

namespace cyrex::network::raknet
{
std::string buildMotd(const cyrex::Server& server);
}
