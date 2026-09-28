#include "LobbyServer.hpp"
#include "AzureProvisioning.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

void LobbyServer::start(int port)
{
    listener.setBlocking(false);

    auto status = listener.listen(port);
    if (status != sf::Socket::Status::Done)
    {
        std::cout << "Failed to listen on port " << port << std::endl;
        return;
    }

    std::cout << "LobbyServer listening on port " << port << std::endl;

    std::vector<PendingConn> pending;

    while (true)
    {
        // --- Accept new raw connections into pending, NOT into clients ---
        auto newClient = std::make_shared<sf::TcpSocket>();
        auto acceptStatus = listener.accept(*newClient);
        if (acceptStatus == sf::Socket::Status::Done) {
            std::cout << "[LOBBY] TCP connection accepted"
                << std::endl;

            std::cout << "[LOBBY] Moving connection into pending"
                << std::endl;

            newClient->setBlocking(false);
            pending.push_back({ std::move(newClient), sf::Clock() });
            // no log here — could be a health probe, we don't know yet
        }
        /*else
        {
            std::cout << "[LOBBY] No new connection accepted. Status: "
                << static_cast<int>(acceptStatus)
                << std::endl;
        }*/

        // --- Poll pending sockets: promote on real packet, silently drop otherwise ---
        for (auto it = pending.begin(); it != pending.end(); )
        {
           /* std::cout << "[LOBBY] Checking pending socket..."
                << std::endl;*/

            sf::Packet packet;
            auto pstatus = it->sock->receive(packet);

            if (pstatus != sf::Socket::Status::NotReady)
            {
                std::cout << "[LOBBY] Pending receive status: "
                    << static_cast<int>(pstatus)
                    << std::endl;
            }

            if (pstatus == sf::Socket::Status::Done)
            {
                // Real client: log, handle its first packet, promote to clients
                std::cout << "New client connected to lobby from "
                    << it->sock->getRemoteAddress().value().toString()
                    << std::endl;

                handlePacket(it->sock, packet);

                clients.push_back(std::move(it->sock));
                it = pending.erase(it);
            }
            else if (pstatus == sf::Socket::Status::Disconnected)
            {
                std::cout << "[LOBBY] Pending socket disconnected"
                    << std::endl;
                // Probe connected and closed without sending anything — ignore silently
                it = pending.erase(it);
            }
            else if (it->age.getElapsedTime().asSeconds() > 10.f)
            {
                std::cout << "[LOBBY] Pending socket timed out after "
                    << it->age.getElapsedTime().asSeconds()
                    << " seconds" << std::endl;
                // Sat idle too long without sending or disconnecting — reap it
                it = pending.erase(it);
            }
            else
            {
                /*std::cout << "[LOBBY] Waiting for client packet..."
                    << std::endl;*/
                ++it; // still waiting, give it more time
            }
        }

        // --- Poll verified clients for packets (unchanged from before) ---
        for (auto it = clients.begin(); it != clients.end(); )
        {
            sf::Packet packet;
            auto status = (*it)->receive(packet);

            if (status == sf::Socket::Status::Done)
            {
               // std::cout << "LOBBY RECEIVED PACKET" << std::endl;
                handlePacket(*it, packet);
                ++it;
            }
            else if (status == sf::Socket::Status::Disconnected)
            {
                std::cout << "LOBBY CLIENT DISCONNECTED FROM "
                    << (*it)->getRemoteAddress().value().toString()
                    << std::endl;
                it = clients.erase(it);
            }
            else if (status == sf::Socket::Status::Error)
            {
                std::cout << "LOBBY RECEIVE ERROR" << std::endl;
                ++it;
            }
            else
            {
                ++it;
            }
        }
    

        update();

        sf::sleep(sf::milliseconds(1));
    }
}

void LobbyServer::update()
{
    // Handles deleting lobbies if host crashes or closes game
    // If theres no heartbeat for 30 seconds, delete lobby

    vector<int> serversToDelete;

    {
        std::lock_guard<std::mutex> lock(serverMutex);

        for (auto& server : servers)
        {
            if (server.second.heartBeat.getElapsedTime().asSeconds() > 30)
            {
                std::cout << "Server " << server.first << " timed out after "
                    << server.second.heartBeat.getElapsedTime().asSeconds()
                    << " seconds with no heartbeat\n";

                serversToDelete.push_back(server.first);
                continue;
            }

            if (server.second.isEmpty && server.second.emptyClock.getElapsedTime().asSeconds() >= 90.f)
            {
                cout << "Server " << server.first << " was empty, deleting..." << endl;

                serversToDelete.push_back(server.first);
            }
        }
    }

    if (!serversToDelete.empty())
    {
        std::string tokenResponse = getAzureAccessToken(); 
        std::string accessToken = extractAccessToken(tokenResponse); 

        if (accessToken.empty())
        { 
            std::cout << "Failed to get Azure access token for deletion" << endl;
            return;
        }

        for (auto serverID : serversToDelete)
        {
            if (deleteGameServer(accessToken, serverID))
            {
                std::lock_guard<std::mutex> lock(serverMutex);

                auto it = servers.find(serverID);

                if (it != servers.end())
                {
                    servers.erase(it);
                    std::cout << "Removed server " << serverID << " from lobby.\n";
                }
            }
        }
    }
    
}

void LobbyServer::handlePacket(std::shared_ptr<sf::TcpSocket> client, sf::Packet& packet)
{
    client->setBlocking(false);
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

        std::cout << "[LOBBY] Received maxPlayers: "
            << server.maxPlayers
            << std::endl;

        int assignedID;
        {
            std::lock_guard<std::mutex> lock(serverMutex);
            assignedID = nextServerID++;
        }

        const char* acrPasswordEnv = std::getenv("ACR_PASSWORD");
        std::string acrPassword = acrPasswordEnv ? acrPasswordEnv : "";
       
        std::thread(
            [this, server, acrPassword, assignedID, client]() mutable
            {
                std::string tokenResponse = getAzureAccessToken();

                std::string accessToken = extractAccessToken(tokenResponse);

                if (accessToken.empty())
                {
                    std::cout << "Failed to get Azure access token\n";
                    return;
                }

                std::cout << "Creating Azure game server with ID: "
                    << assignedID << '\n';

                bool created = createGameServer(accessToken, assignedID, server, acrPassword);

                if (!created)
                {
                    std::cout << "Failed to create Azure game server\n";
                    return;
                }

                std::cout << "Azure game server created successfully\n";

                std::string publicIP = waitForGameServerPublicIP(accessToken, assignedID);

                if (publicIP.empty())
                {
                    std::cout << "Failed to get Azure game server public IP\n";
                    return;
                }

                auto resolvedIP = sf::IpAddress::resolve(publicIP);

                if (!resolvedIP)
                {
                    std::cout << "Failed to resolve game server IP: "
                        << publicIP << '\n';
                    return;
                }

                std::cout << "AZURE Public IP: " << publicIP << endl;

                server.ip = resolvedIP.value();
                server.currentPlayers = 1;

                server.heartBeat.restart();

                {
                    std::lock_guard<std::mutex> lock(serverMutex);
                        servers.emplace(assignedID, server);
                }

                sf::Packet response;
                response << static_cast<int>(PacketType::ServerReady) << assignedID << publicIP << server.port;
                client->send(response);

                std::cout << "Server registered: "
                    << server.name
                    << " at "
                    << server.ip
                    << ":"
                    << server.port
                    << std::endl;
            }
          ).detach();
    }

    // Updates when players join server and restarts
    // hearbeat to keep server alive
    //---------------------------------------------//
    if (type == PacketType::Heartbeat)
    {
        int serverID, players;

        packet >> serverID >> players;

        std::lock_guard<std::mutex> lock(serverMutex);

        if (!servers.count(serverID))
        {
            std::cout << "Heartbeat received for UNKNOWN server " << serverID << " — already expired?\n";
            return; // don't fall through and create a phantom entry
        }

        if (servers.count(serverID))
        {          
            servers[serverID].currentPlayers = players;
            servers[serverID].heartBeat.restart();
        }
        else
        {
            std::cout << "Heartbeat received for UNKNOWN server " << serverID << " — already expired?\n";
        }

        if (players == 0)
        {
            if (!servers[serverID].isEmpty)
            {
                servers[serverID].isEmpty = true;
                servers[serverID].emptyClock.restart();

                std::cout << "Server " << serverID
                    << " is now empty. Starting 90 second timer.\n";
            }
        }
        else
        {
            servers[serverID].isEmpty = false;
        }
    }
    //----------------------------------------------//
     
    if (type == PacketType::RequestServerList)
    {
        sf::Packet packet;
        std::lock_guard<std::mutex> lock(serverMutex);

        std::cout << "Sending server list: " << servers.size() << std::endl;
      
        packet << static_cast<int>(PacketType::ServerList);
        packet << static_cast<int>(servers.size());

        for (auto& [id, server] : servers)
        {
            packet << id << server.name << server.ip.toString()
                   << server.port << server.currentPlayers 
                   << server.maxPlayers << server.passwordProtected << server.password;
        }

        client->send(packet);
        
    }
}
