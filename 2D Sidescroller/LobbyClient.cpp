#include "LobbyClient.hpp"

void LobbyClient::requestServerList()
{
	sf::Packet packet;

	packet << static_cast<int>(PacketType::RequestServerList);

	this->socket.send(packet);
}

void LobbyClient::createServer(std::string name, int port, int maxPlayers)
{
	sf::Packet packet;

	packet << static_cast<int>(PacketType::CreateServer) << name << port << maxPlayers << false;

	this->socket.send(packet);
}
