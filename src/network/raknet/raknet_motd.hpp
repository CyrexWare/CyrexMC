#pragma once
#include "server.hpp"

#include <string>

namespace cyrex::network::raknet
{
std::string buildRaknetMotd(const cyrex::Server& server);
}
