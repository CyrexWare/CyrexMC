// STD
#include <iostream>
// RakNet
#include <RakNet/RakPeerInterface.h>
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/RakSleep.h>
// Server
#include "server.hpp"

namespace {
	void debug_client() {
		RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
		if (peer == nullptr) return;
		bool active = true;
		RakNet::SocketDescriptor socket_descriptor;
		peer->Startup(1, &socket_descriptor, 1);
		peer->Connect("127.0.0.1", 19132, nullptr, 0);

		while (active) {
			for (RakNet::Packet* packet{};
				(packet = peer->Receive()) != nullptr;
				peer->DeallocatePacket(packet)) {
				switch (packet->data[0]) {
					case ID_CONNECTION_REQUEST_ACCEPTED: std::cerr << "client connected to server\n"; break;
					case ID_CONNECTION_ATTEMPT_FAILED:	 std::cerr << "failed to connect to the server\n"; active = false;  break;
				}
				if (!active) break;
			}
			RakSleep(10);
		}

		peer->Shutdown(50);
	}

	void server() {
		try {
			Server server(Server::Config::make_default_mc_be());
			server.run();
		} catch (const Server::InitFailedError& init_failed_error) {
			std::cerr << "Server Initialization Failed: " << init_failed_error.what() << '\n';
		}
	}
}

int main() {
	puts("[c]lient / [s]erver");
	const std::int32_t ch = getchar();
	if (ch == 'c') debug_client();
	else if (ch == 's') server();
	return 0;
}