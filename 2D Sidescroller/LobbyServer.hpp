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

    // "local" backend: game servers run as child processes of the lobby
    // instead of Azure containers (see LOBBY_BACKEND in README).
    bool localBackend = false;
    std::string publicHost;
    int gamePortBase = 54010;
    std::unordered_map<int, long> children; // serverID -> pid (guarded by serverMutex)

    void createLocalGameServer(int assignedID, ServerInfo server, std::shared_ptr<sf::TcpSocket> client);
    void stopLocalGameServer(int serverID);
    void stopAllLocalGameServers();


    // Sockets that have connected but not yet proven themselves with a real packet.
    // Probes live and die here without ever touching `clients`.
    struct PendingConn {
        std::shared_ptr<sf::TcpSocket> sock;
        sf::Clock age;
    };

  public:

    LobbyServer();
    ~LobbyServer();

    void start(int port);
    void update();
    void handlePacket(std::shared_ptr<sf::TcpSocket> client, sf::Packet& packet);
};