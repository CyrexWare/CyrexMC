#pragma once
#include <stdexcept>
// RakNet
#include <RakNet/RakPeerInterface.h>

enum class DefaultMinecraftPorts : unsigned short {
	JavaEdition = 25565,
	BedrockEdition = 19132
};

class Server {
public:
	struct Config {
		unsigned short port{};
		unsigned int max_users{};
		unsigned short max_incoming_connections{};

		// Default configuration for a typical minecraft bedrock server
		constexpr static Config make_default_mc_be() {
			return {
				.port = static_cast<unsigned short>(DefaultMinecraftPorts::BedrockEdition),
				.max_users = 20,
				.max_incoming_connections = 5
			};
		}
	};

	struct InitFailedError : std::runtime_error {
		explicit InitFailedError(const std::string& message);
	};

public:
	// Initializes the server to a usable state
	// Throws: InitFailedError
	explicit Server(const Config& config);
	~Server();

private:
	// Disable copying
	Server(const Server&) = delete;
	Server& operator = (const Server&) = delete;

public:
	// Moves
	Server(Server&& server) noexcept;
	Server& operator = (Server&& server) noexcept;

public:
	void run();

private:
	void stop();

private:
	void receive_packets();
	void on_packet_received(const RakNet::Packet* packet);

private:
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
};