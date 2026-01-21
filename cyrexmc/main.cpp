#include <iostream>
#include <RakNet/RakPeerInterface.h>
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/RakSleep.h>
#include "server.hpp"

static void debug_client() {
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	if (peer == nullptr) return;
	bool active = true;
	RakNet::SocketDescriptor socket_descriptor;
	peer->Startup(1, &socket_descriptor, 1);
	peer->Connect("127.0.0.1", 25565, nullptr, 0);

	while (active) {
		for (RakNet::Packet* packet{};
			(packet = peer->Receive()) != nullptr;
			peer->DeallocatePacket(packet)) {
			switch (packet->data[0]) {
				case ID_CONNECTION_REQUEST_ACCEPTED: puts("client connected to server"); break;
				case ID_CONNECTION_ATTEMPT_FAILED:	 puts("failed to connect to the server"); active = false;  break;
			}
			if (!active) break;
		}
		RakSleep(10);
	}

	peer->Shutdown(50);
}

static void server() {
	Server server;
	
	if (server.init(Server::Config::make_typical())) {
		server.run();
	}

	server.stop();
}

int main() {
	puts("[c]lient / [s]erver");
	const char ch = getchar();
	if (ch == 'c') debug_client();
	else if (ch == 's') server();
	return 0;
}