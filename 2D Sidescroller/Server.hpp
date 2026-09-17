#pragma once
#include <SFML/Network.hpp>
#include "PacketType.h"
#include "ServerInfo.h"
#include "Client.hpp"
#include <set>
#include <unordered_map>
using namespace std;

class Server
{
public:
    Server(unsigned short port)
    {
        this->port = port;
    }

    void relayPacketToClientsExceptMe(sf::Packet relayPacket, int index)
    {
        for (size_t j = 0; j < clients.size(); j++)
        {
            if (index == j) continue;

            clients[j]->send(relayPacket);
        }
    }

    void relayPacketToAllClients(sf::Packet relayPacket, int index)
    {
        for (size_t j = 0; j < clients.size(); j++)
        {           
            clients[j]->send(relayPacket);
        }
    }

    void broadcastDeflect(int defenderID, int attackerID)
    {
        sf::Packet deflectPacket;

        deflectPacket << static_cast<int>(PacketType::Deflection) << defenderID << attackerID;

        for (auto& client : clients)
        {
            client->send(deflectPacket);
        }
    }

    void heartBeatLogic(sf::Clock& heartbeatClock, float heartBeatInterval)
    {
        if (heartbeatClock.getElapsedTime().asSeconds() >= heartBeatInterval)
        {
            if (serverID != -1) // make sure lobby assigned an ID
            {
                // cout << "HEARTBEAT SENT" << endl;
                sf::Packet hbPacket;
                hbPacket << static_cast<int>(PacketType::Heartbeat)
                    << serverID
                    << static_cast<int>(clients.size()); // current players

                lobbySocket.send(hbPacket); // send to lobby
            }

            heartbeatClock.restart();
        }
    }

    void gameWinnerLogic(int playerID)
    {
        cout << "Remaining players before erase: ";
        for (auto p : remainingPlayers)
            cout << p << " ";
        cout << endl;

        if (!remainingPlayers.count(playerID)) return;

       // cout << "Player " << playerID << " Deleted" << endl;

        remainingPlayers.erase(playerID);

       // cout << "Remaining Players: " << remainingPlayers.size() << endl;

        if (remainingPlayers.size() == 1)
        {
            int lastPlayer = *remainingPlayers.begin();

            sf::Packet winPacket;

            winPacket << static_cast<int>(PacketType::GameWinner) << lastPlayer;

            for (auto& client : clients)
            {
                client->send(winPacket);
            }

            countdownStarted = false;
            countdownActive = false;
        }
    }

     void countdownLogic()
     {      
             float remaining = countdownDuration - countdownClock.getElapsedTime().asSeconds();

             sf::Packet packet;
             packet << static_cast<int>(PacketType::CountdownUpdate) << remaining;

             for (auto& client : clients)
                 client->send(packet);

             if (remaining <= 0)
             {
                 countdownActive = false;
                 countdownStarted = false;
                 gameStarted = true;

                 for (auto& dissconnectedPlayers : pendingDisconnects)
                 {
                     gameWinnerLogic(dissconnectedPlayers);

                     sf::Packet death;
                     death << static_cast<int>(PacketType::Death) << dissconnectedPlayers;

                     for (auto& client : clients)
                         client->send(death);
                 }

                 pendingDisconnects.clear();

                 sf::Packet startPacket;
                 startPacket << static_cast<int>(PacketType::GameStart);

                 for (auto& client : clients)
                     client->send(startPacket);

                
             }
         
     }


    // Uses a SET to reuse Player IDs when somone leaves the lobby. (PlayerIDs are 0,1,2,3. Since this is max a 4 player game
    // we dont want the IDs to exceed 3 since thta will mess up coloring
    int assignPlayerID()
    {
        if (!availableIDs.empty())
        {
            int id = *availableIDs.begin(); // begin() returns the iterator, deferencing it gives us the integer

            availableIDs.erase(availableIDs.begin());

            return id;
        }

        return nextPlayerID++;
    }

    void createServer(const ServerInfo& info)
    {
        sf::Packet packet;

        packet << static_cast<int>(PacketType::CreateServer) << info.name << info.port << info.maxPlayers << info.passwordProtected << info.password;

        this->lobbySocket.send(packet);
    }

    void connectToLobby(const std::string& hostname, unsigned short port)
    {
        auto ip = sf::IpAddress::resolve(hostname);

        if (!ip)
        {
            std::cout << "Failed to resolve lobby address\n";
            return;
        }

        if (lobbySocket.connect(*ip, port) != sf::Socket::Status::Done)
        {
            std::cout << "Failed to connect to lobby\n";
            return;
        }
    }

    void broadcastLobby()
    {
        sf::Packet packet;
        packet << static_cast<int>(PacketType::LobbyUpdate);
        packet << hostID;
        packet << static_cast<int>(lobbyPlayers.size());

        for (auto& [id, item] : lobbyPlayers)
        {
            packet << id << item.name << static_cast<int>(item.character);
        }

        for (auto& client : clients)
        {
            client->send(packet);
        }
    }

    void removePlayer(sf::TcpSocket* socket)
    {
        if (!socketToPlayerID.count(socket)) return;

        int playerID = socketToPlayerID[socket];

        cout << "Removing Player " << playerID << endl;

        bool wasHost = (playerID == hostID);

        lobbyPlayers.erase(playerID);

        socketToPlayerID.erase(socket);

        availableIDs.insert(playerID);

        for (auto it = clients.begin(); it != clients.end(); ++it)
        {
            if (it->get() == socket)
            {
                clients.erase(it);
                break;
            }
        }

        if (wasHost)
        {
            if (!clients.empty())
            {
                hostID = socketToPlayerID[clients[0].get()];
                cout << "HOST ID: " << hostID << endl;
                cout << "NEW HOST:" << lobbyPlayers[hostID].name << endl;
            }
            else
            {
                hostID = -1;
            }
        }

        sf::Packet packet;

        packet << static_cast<int>(PacketType::PlayerLeave) << playerID;
       
        for (auto& client : clients)
        {
            client->send(packet);
        }

        broadcastLobby();
    }

    void runRelayServer()
    {
        listener.listen(port);
        lobbySocket.setBlocking(false);
        listener.setBlocking(false);

        cout << "Server listening on port 54000" << endl;

        sf::Clock heartbeatClock;
        const float HEARTBEAT_INTERVAL = 3.f;

        while (running) // main server loop
        {
            // If server has been created, give it a serverID. Retrieves serverID from 
            // lobbyServer.handlePacket()
            //---------------------------------------------------------------------------//
            sf::Packet response;

            if (lobbySocket.receive(response) == sf::Socket::Status::Done)
            {
                int typeInt;
                response >> typeInt;

                PacketType type = static_cast<PacketType>(typeInt);

                if (type == PacketType::AssignID)
                {
                    response >> serverID;

                    std::cout << "Lobby assigned serverID: " << serverID << std::endl;
                }
            }
            //---------------------------------------------------------------------------//
            
            // Server connects with7 up to 4 players
            if (clients.size() < 4)
            {
                std::unique_ptr<sf::TcpSocket> newClient = std::make_unique<sf::TcpSocket>();

                if (listener.accept(*newClient) == sf::Socket::Status::Done)
                {
                    int playerID = assignPlayerID();
                    lobbyPlayers[playerID] = { "" , CharacterType::Unknown};
                
                    socketToPlayerID[newClient.get()] = playerID;

                    if (hostID == -1)
                    {
                        hostID = playerID;
                    }

                    cout << "Player " << playerID << " connected\n";

                    sf::Packet idPacket;
                    idPacket << static_cast<int>(PacketType::AssignID) << playerID;

                    newClient->send(idPacket);

                    sf::Packet serverIDPacket;
                    serverIDPacket << static_cast<int>(PacketType::AssignServerID) << serverID;

                    newClient->send(serverIDPacket);

                    newClient->setBlocking(false);
                 
                    clients.push_back(std::move(newClient));
                }
            }

                // Packets are sent to every other player. Ex.) Player 2 moves, server sends movement to Players 0,1,3
                for (size_t i = 0; i < clients.size(); i++)
                {
                    sf::Packet packet;  

                    sf::Socket::Status status = clients[i]->receive(packet);

                    if (status == sf::Socket::Status::Done)
                    {
                        sf::Packet relayPacket = packet;

                        int typeInt;
                        packet >> typeInt;
                        PacketType type = static_cast<PacketType>(typeInt);

                        if (type == PacketType::ArrowDeflection)
                        {
                            cout << "Relaying to " << clients.size() << " clients\n";
                        }

                        if (type == PacketType::PlayerJoin)
                        {
                            std::string name;
                            int characterType;
                            packet >> name >> characterType;

                            string characterString;

                            CharacterType character = static_cast<CharacterType>(characterType);

                            int playerID = socketToPlayerID[clients[i].get()];

                            std::cout << "\n===== PLAYER JOIN PACKET =====\n";
                            std::cout << "Socket player ID: " << playerID << '\n';
                            std::cout << "Name: " << name << '\n';
                            std::cout << "Character: " << characterType << '\n';

                            std::cout << "BEFORE:\n";

                            lobbyPlayers[playerID] = { name, character };

                            if (character == CharacterType::Knight) characterString = "Knight";
                            else characterString = "Samurai";

                            std::cout << "Player " << playerID << "\nname: "  << name << "\nCharacter: " << characterString << std::endl;

                            std::cout << "AFTER:\n";

                            for (const auto& [id, player] : lobbyPlayers)
                            {
                                std::cout << "  ID " << id
                                    << " -> "
                                    << static_cast<int>(player.character)
                                    << '\n';
                            }

                            broadcastLobby();
                            continue; // don't relay this packet after
                        }
                        else if (type == PacketType::StartLoading)
                        {
                            sf::Packet startPacket;

                            startPacket << static_cast<int>(PacketType::StartLoading);

                            for (auto& client : clients)
                            {
                                client->send(startPacket);
                            }

                            remainingPlayers.clear();

                            cout << "-----------------------------" << endl;
                            for (auto& [id,player] : lobbyPlayers)
                            {                
                                cout << "Players: " << player.name << "ID: " << id << endl;
                                remainingPlayers.insert(id);
                            }
                            cout << "-----------------------------" << endl;
                           
                            countdownPause.restart();
                            countdownStarted = true;   // pause started
                            countdownActive = false;   // countdown NOT started yet
                            gameStarted = false;
                                                        
                            continue;
                        }
                        else if (type == PacketType::PlayerLeave)
                        {
                            removePlayer(clients[i].get());
                            continue;
                        }
                        else if  (type == PacketType::RequestLobbySnapshot)
                        {
                            sf::Packet snapshot;
                            snapshot << static_cast<int>(PacketType::LobbyUpdate) << lobbyPlayers.size();

                            cout << "SIZE OF LOBBY: " << lobbyPlayers.size() << endl;

                            for (auto& [id, player] : lobbyPlayers) {
                                snapshot << id << player.name;
                            }
                           
                            for (auto& client : clients)
                            {
                                client->send(snapshot);
                            }
                        }
                        else if (type == PacketType::ServerDelete)
                        {
                            int playerID = socketToPlayerID[clients[i].get()];
                                
                            if (playerID != hostID)
                            {
                                cout << "Non-Host tried to delete server" << endl;
                                continue;
                            }

                            cout << "Host Deleting Server..." << endl;

                            sf::Packet shutdown;

                            shutdown << static_cast<int>(PacketType::ClientDisconnect);

                            for (auto& client : clients)
                            {                             
                                client->send(shutdown); // disconnect the gamesocket that each client is connected to

                                // gracefully shutdown each client, this needs to be here before clients.clear() 
                                // because cleints.clear() just kills the vector<Tcp*socket> object without properly closign the tcp connections
                                client->disconnect(); 
                            }

                            // Remove everything that had players and IDs inside of them
                            clients.clear();
                            lobbyPlayers.clear();
                            socketToPlayerID.clear();

                            sf::Packet packet;

                            packet << static_cast<int>(PacketType::ServerDelete) << serverID;

                            lobbySocket.send(packet); 

                            running = false; // stop the server loop
                            break;
                        }
                        else if (type == PacketType::Death)
                        {
                            int id = 0;

                            packet >> id;

                            gameWinnerLogic(id);

                            relayPacketToClientsExceptMe(relayPacket, i);

                        }
                        else if (type == PacketType::Deflection)
                        {
                            int defenderID = 0, attackerID = 0;

                            packet >> defenderID >> attackerID;
                            
                            broadcastDeflect(defenderID, attackerID);
                        }  
                        else if (type == PacketType::ChangeCharacter)
                        {        
                            int playerID;
                            int characterInt;

                            packet >> playerID >> characterInt;

                            CharacterType newCharacter = static_cast<CharacterType>(characterInt);;

                            lobbyPlayers[playerID].character = newCharacter;
                               
                            broadcastLobby();                        
                        }
                        else
                        {
                            relayPacketToClientsExceptMe(relayPacket, i);                          
                        }
                    }
                    else if (status == sf::Socket::Status::Disconnected)
                    {
                        cout << "Client disconnected unexpectedly" << endl;                        

                        int playerID = socketToPlayerID[clients[i].get()];

                        if (countdownStarted || countdownActive)
                        {
                            pendingDisconnects.push_back(playerID);
                        }
                        else
                        {

                            gameWinnerLogic(playerID);

                            sf::Packet death;
                            death << static_cast<int>(PacketType::Death) << playerID;

                            for (auto& client : clients)
                            {
                                client->send(death);
                            }                           
                        }

                        removePlayer(clients[i].get());

                        continue;
                    }
                }

                // Sends heartbeat to lobby every 3 seconds. Tells lobby that the server is still
                // alive. Sends packet to lobbyServer.handlePacket()
                //---------------------------------------------------------------------------//               
                 heartBeatLogic(heartbeatClock, HEARTBEAT_INTERVAL);
                //---------------------------------------------------------------------------//

                // inside main server loop
                if (countdownStarted) // pause phase
                {
                    if (countdownPause.getElapsedTime().asSeconds() >= 4.f)
                    {
                        countdownClock.restart();
                        countdownStarted = false;
                        countdownActive = true; // countdown begins
                    }
                }


                if (countdownActive) countdownLogic();
               
                sf::sleep(sf::milliseconds(1));
            
        }
    }


    sf::TcpSocket lobbySocket;

private:

    sf::TcpListener listener;
    
    unsigned short port;

    std::unordered_map<int, LobbyPlayer> lobbyPlayers;
    std::set<int> remainingPlayers;
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;
    std::unordered_map<sf::TcpSocket*, int> socketToPlayerID;
    std::vector<int> pendingDisconnects;
    std::set<int> availableIDs;

    int nextPlayerID = 0;
    int serverID = -1;
    int hostID = -1;
    bool running = true;

    bool countdownStarted = false;
    sf::Clock countdownClock;
    sf::Clock countdownPause;
    float countdownDuration = 3.f;
    bool countdownActive = false;

    bool gameStarted = false;

    /*void acceptPlayers();
    void handleGame();*/
};