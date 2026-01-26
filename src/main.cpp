// STD
#include <iostream>
// RakNet
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/RakPeerInterface.h>
#include <RakNet/RakSleep.h>
// Server
#include "server.hpp"

int main()
{
    try
    {
        Server server(Server::Config::makeDefault());
        server.run();
    } catch (const Server::InitFailedError& initFailedError)
    {
        std::cerr << "Server Initialization Failed: " << initFailedError.what() << '\n';
    }

    return 0;
}