#include "LobbyServer.hpp"

void LobbyServer::start(int port)
{
  
    listener.setBlocking(false);

    listener.listen(port);
    
    std::cout << "LobbyServer listening on port " << port << std::endl;
       
    while (true)
    {
        // Accept new clients
        auto newClient = std::make_unique<sf::TcpSocket>();
        if (listener.accept(*newClient) == sf::Socket::Status::Done) {
            newClient->setBlocking(false);
            clients.push_back(std::move(newClient));
            std::cout << "New client connected to lobby" << std::endl;
        }

        // Poll each client for packets
        for (auto& client : clients) {
            sf::Packet packet;
            if (client->receive(packet) == sf::Socket::Status::Done) {
                //std::cout << "Packet received from client\n";
                handlePacket(*client, packet);
            }
        }

        update();

        sf::sleep(sf::milliseconds(1));
    }
}

void LobbyServer::update()
{
    // Handles deleting lobbies if host crashes or closes game
    // If theres no heartbeat for 10 seconds, delete lobby
    for (auto it = servers.begin(); it != servers.end(); )
    {
        if (it->second.heartBeat.getElapsedTime().asSeconds() > 10)
        {
            std::cout << "Server timed out\n";
            it = servers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void LobbyServer::handlePacket(sf::TcpSocket& client, sf::Packet& packet)
{
    client.setBlocking(false);
    int typeInt;
    packet >> typeInt;

    PacketType type = static_cast<PacketType>(typeInt);

    if (type == PacketType::CreateServer)
    {
        ServerInfo server;

        packet >> server.name;
        packet >> server.port;
        packet >> server.maxPlayers;
        packet >> server.passwordProtected;
        packet >> server.password;

        cout << "SERVER RECEIVED PASSWORD: [" << server.password << "]" << endl;
        cout << "SERVER RECEIVED PROTECTED: "
            << server.passwordProtected << endl;

        server.ip = client.getRemoteAddress().value();

        server.currentPlayers = 0;
        servers.emplace(nextServerID, server);

        sf::Packet response;
        response << static_cast<int>(PacketType::AssignID) << nextServerID;
        client.send(response);

        std::cout << "Server registered: "
            << server.name
            << " at "
            << server.ip
            << ":"
            << server.port
            << std::endl;

        nextServerID++;
    }

    // Updates when players join server and restarts
    // hearbeat to keep server alive
    //---------------------------------------------//
    if (type == PacketType::Heartbeat)
    {
        int serverID, players;

        packet >> serverID >> players;

        if (servers.count(serverID))
        {
            servers[serverID].currentPlayers = players;
            servers[serverID].heartBeat.restart();
        }
    }
    //----------------------------------------------//
     
    if (type == PacketType::RequestServerList)
    {
        sf::Packet packet;

        std::cout << "Sending server list: " << servers.size() << std::endl;
        packet << static_cast<int>(PacketType::ServerList);
        packet << static_cast<int>(servers.size());

        for (auto& [id, server] : servers)
        {
            packet << id << server.name << server.ip.toString()
                   << server.port << server.currentPlayers 
                   << server.maxPlayers << server.passwordProtected << server.password;
        }

        client.send(packet);
        
    }

    if (type == PacketType::ServerDelete)
    {
        int id;
        packet >> id;

        servers.erase(id);
       
    }
}
