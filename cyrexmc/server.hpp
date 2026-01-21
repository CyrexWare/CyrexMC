#pragma once
#include <RakNet/RakPeerInterface.h>

class Server {
public:
	struct Config {
		unsigned short port{};
		unsigned int max_users{};
		unsigned short max_incoming_connections{};

		constexpr static Config make_typical() {
			return Config{
				.port = 25565,
				.max_users = 20,
				.max_incoming_connections = 5
			};
		}
	};

public:
	bool init(const Config config);
	void run();
	void stop();

private:
	void receive_packets();
	void on_packet_received(const RakNet::Packet* packet);

private:
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	bool was_init = false;
};