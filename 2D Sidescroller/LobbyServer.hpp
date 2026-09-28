#pragma once
#include <SFML/Network.hpp>
#include <unordered_map>
#include "PacketType.h"
#include "ServerInfo.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <memory>

using namespace std;

class LobbyServer
{
  private:

    sf::TcpListener listener;
    std::vector<std::shared_ptr<sf::TcpSocket>> clients;

    std::unordered_map<int, ServerInfo> servers;
    std::mutex serverMutex;

    int nextServerID = 0;


    // Sockets that have connected but not yet proven themselves with a real packet.
    // Probes live and die here without ever touching `clients`.
    struct PendingConn {
        std::shared_ptr<sf::TcpSocket> sock;
        sf::Clock age;
    };

  public:

    LobbyServer() = default;

    void start(int port);
    void update();
    void handlePacket(std::shared_ptr<sf::TcpSocket> client, sf::Packet& packet);
};