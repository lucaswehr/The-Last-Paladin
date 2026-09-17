#pragma once
#include <SFML/Network.hpp>
#include <unordered_map>
#include "PacketType.h"
#include "ServerInfo.h"
#include <iostream>
using namespace std;

class LobbyServer
{
  private:

    sf::TcpListener listener;
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;

    std::unordered_map<int, ServerInfo> servers;

    int nextServerID = 0;

  public:

      LobbyServer() = default;

    void start(int port);
    void update();

    void handlePacket(sf::TcpSocket& client, sf::Packet& packet);

    void sendServerList(sf::TcpSocket& client);
};