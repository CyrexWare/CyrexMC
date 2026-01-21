#include <iostream>
#include <RakNet/RakPeerInterface.h>
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/RakSleep.h>


static void client() {
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	if (peer == nullptr) return;
	bool active = true;
	RakNet::SocketDescriptor socket_descriptor;
	peer->Startup(1, &socket_descriptor, 1);
	peer->Connect("127.0.0.1", 12345, nullptr, 0);

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
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	if (peer == nullptr) return;
	RakNet::SocketDescriptor socket_descriptor(12345, nullptr);
	peer->Startup(10, &socket_descriptor, 1);
	peer->SetMaximumIncomingConnections(5);

	while (true) {
		for (RakNet::Packet* packet{};
			(packet = peer->Receive()) != nullptr;
			peer->DeallocatePacket(packet)) {
			switch (packet->data[0]) {
				case ID_NEW_INCOMING_CONNECTION: std::puts("User joined"); break;
			}
		}
		RakSleep(10);
	}
	peer->Shutdown(50);
}

int main() {
	puts("[c]lient / [s]erver");
	const char ch = getchar();
	if (ch == 'c') client();
	else if (ch == 's') server();
	return 0;
}