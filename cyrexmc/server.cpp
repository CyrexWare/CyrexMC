#include "server.hpp"
#include <cstdio>

// Raknet
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/RakSleep.h>

bool Server::init(const Config config) {
	RakNet::SocketDescriptor socket_descriptor (config.port, nullptr);
	RakNet::StartupResult startup_result = peer->Startup(config.max_users, &socket_descriptor, 1);

	// All clear
	if (startup_result == RakNet::RAKNET_STARTED) {
		peer->SetMaximumIncomingConnections(config.max_incoming_connections);
		was_init = true;
		return true;
	}

#define X(e) case e: std::puts("Server init failed. Reason: " #e); break
	switch (startup_result) {
		X(RakNet::RAKNET_ALREADY_STARTED);
		X(RakNet::INVALID_SOCKET_DESCRIPTORS);
		X(RakNet::INVALID_MAX_CONNECTIONS);
		X(RakNet::SOCKET_FAMILY_NOT_SUPPORTED);
		X(RakNet::SOCKET_PORT_ALREADY_IN_USE);
		X(RakNet::SOCKET_FAILED_TO_BIND);
		X(RakNet::SOCKET_FAILED_TEST_SEND);
		X(RakNet::PORT_CANNOT_BE_ZERO);
		X(RakNet::FAILED_TO_CREATE_NETWORK_THREAD);
		X(RakNet::COULD_NOT_GENERATE_GUID);
		X(RakNet::STARTUP_OTHER_FAILURE);
	}
#undef X
	return false;
}

void Server::run() {
	if (!was_init) {
		std::puts("Internal error: Tried to run an uninitialized server");
		return;
	}

	while (true) {
		receive_packets();
		RakSleep(15);
	}
}

void Server::stop() {
	if (peer) {
		peer->Shutdown(50);
	}
}

void Server::receive_packets() {
	for (RakNet::Packet* packet{};
		(packet = peer->Receive()) != nullptr;
		peer->DeallocatePacket(packet)) {
		on_packet_received(packet);
	}
}

void Server::on_packet_received(const RakNet::Packet* packet) {
	switch (packet->data[0]) {
		case ID_NEW_INCOMING_CONNECTION: std::puts("User joined"); break;
	}
}
