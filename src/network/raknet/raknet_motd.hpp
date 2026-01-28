#pragma once
#include <string>
#include "server.hpp"

namespace cyrex::network::raknet
{
std::string buildRaknetMotd(const cyrex::Server& server);
}
