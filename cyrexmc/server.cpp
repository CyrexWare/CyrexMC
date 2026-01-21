#include "server.hpp"
#include <iostream>

#include <magic_enum/magic_enum.hpp>

// Raknet
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/RakSleep.h>

Server::InitFailedError::InitFailedError(const std::string& message) : std::runtime_error(message) {}

Server::Server(const Config& config) {
	RakNet::SocketDescriptor socket_descriptor(config.port, nullptr);
	RakNet::StartupResult startup_result = peer->Startup(config.max_users, &socket_descriptor, 1);

	// All clear
	if (startup_result == RakNet::RAKNET_STARTED) {
		peer->SetMaximumIncomingConnections(config.max_incoming_connections);
		return;
	}

	throw InitFailedError(std::string(magic_enum::enum_name(startup_result)));
}

Server::~Server() {
	stop();
}

Server::Server(Server&& other) noexcept :
	peer { other.peer }
{
	other.peer = nullptr;
}

Server& Server::operator=(Server&& other) noexcept {
	stop();
	peer = other.peer;
	other.peer = nullptr;
	return *this;
}

void Server::run() {
	while (true) {
		receive_packets();
		RakSleep(15);
	}
}

void Server::stop() {
	if (peer && peer->IsActive()) {
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
		case ID_NEW_INCOMING_CONNECTION: std::cerr << "A user is connecting...\n"; break;
	}
}

