#include <SFML/Network.hpp>
#include <iostream>
#include "Client.hpp"
#include "PlayerState.hpp"
#include "Animation.hpp"
#include <unordered_map>
#include "PacketType.h"


using namespace std;

void Client::createServer(const ServerInfo& info)
{
    sf::Packet packet;

    packet << static_cast<int>(PacketType::CreateServer)
        << info.name
        << info.port
        << info.maxPlayers
        << info.passwordProtected
        << info.password;

    lobbySocket.send(packet);
}

void Client::sendPlayerState(int playerID, float x, float y, PlayerState state, Direction dir)
{
    sf::Packet packet;
   
    packet << static_cast<int>(PacketType::PlayerState) << playerID << x << y << static_cast<int>(state) << static_cast<int>(dir);

    gameSocket.send(packet);
}

void Client::sendAttack(int playerID, float x, float y, Direction dir, int attackID)
{
    sf::Packet packet;
    packet << static_cast<int>(PacketType::AttackStart) << playerID << x << y << (int)dir << attackID;

    gameSocket.send(packet);
}

void Client::sendDamage(int playerID, int victimID, int damage, Direction dir)
{
    sf::Packet packet;

    packet << static_cast<int>(PacketType::PlayerDamage) << playerID << victimID << damage << (int)dir;

    gameSocket.send(packet);
}

void Client::sendDeath(int playerID)
{
    sf::Packet packet;

    packet << static_cast<int>(PacketType::Death) << playerID;

    gameSocket.send(packet);
}

void Client::sendDeflect(int defenderID, int attackerID)
{
    sf::Packet packet;

    packet << static_cast<int>(PacketType::Deflection) << defenderID << attackerID;
    gameSocket.send(packet);
}

void Client::sendHeal(int playerID, int healAmount)
{
    sf::Packet packet;

    packet << static_cast<int>(PacketType::Heal) << playerID << healAmount;

    gameSocket.send(packet);
}

void Client::sendArrow(int playerID, float xPos, float yPos, Direction dir, int arrowID, float arrowGravity)
{
    sf::Packet packet;

    packet << static_cast<int>(PacketType::ArrowShot) << playerID << xPos << yPos << static_cast<int>(dir) << arrowID << arrowGravity;

    gameSocket.send(packet);
}

void Client::sendArrowDeflection(int attackerID, int victimID, int arrowID, sf::Vector2f arrowPos, sf::Vector2f arrowVelocity)
{
    cout << "Sending deflection packet" << endl;

    sf::Packet packet;

    packet << static_cast<int>(PacketType::ArrowDeflection) << attackerID << victimID << arrowID << arrowPos.x << arrowPos.y << arrowVelocity.x << arrowVelocity.y;

    gameSocket.send(packet);
}

void Client::sendBowPullbackSound(int playerID)
{
    sf::Packet packet;

    packet << static_cast<int>(PacketType::BowPullbackSound) << playerID;

    gameSocket.send(packet);
}

void Client::sendCharacterChange(int playerID, CharacterType newCharacter)
{
    sf::Packet packet;

    packet << static_cast<int>(PacketType::ChangeCharacter) << playerID << static_cast<int>(newCharacter);

    gameSocket.send(packet);
}

void Client::leaveLobby()
{
    sf::Packet leavePacket;

    leavePacket << static_cast<int>(PacketType::PlayerLeave);

    gameSocket.send(leavePacket);
    gameSocket.disconnect();
}

void Client::rebuildLobbyPlayers()
{
    sf::Packet packet;

    moveToServerPage = false;

    packet << static_cast<int>(PacketType::RequestLobbySnapshot);
    gameSocket.send(packet);
}



void Client::receiveNetworkEvent(std::unordered_map<int, unique_ptr<Player>>& players, float dt, std::vector<sf::Color>& playerColors, vector<sf::Vector2f> spawnpoints,
     AssetManager& assets,std::string& fonttext, CharacterType characterType, sf::Font& standardFont)
{

    while (true)
    {      
        sf::Packet packet;
        auto status = gameSocket.receive(packet);

        if (status == sf::Socket::Status::Done)
        {
            int typeInt;

            packet >> typeInt;

            PacketType type = static_cast<PacketType>(typeInt);

            if (type == PacketType::AssignID)
            {
                packet >> myPlayerID;

                std::cout << "My ID is: " << myPlayerID << std::endl;
                
                sf::Packet namePacket;

                namePacket << static_cast<int>(PacketType::PlayerJoin) << playerName << static_cast<int>(characterType);

                gameSocket.send(namePacket);

            }

            if (type == PacketType::AssignServerID)
            {
                packet >> myServerID;
                cout << "My server ID: " << myServerID << endl;
            }

            if (type == PacketType::PlayerState)
            {
                int stateInt, dirInt, id;
                float x, y;

                packet >> id >> x >> y >> stateInt >> dirInt;

                PlayerState state = static_cast<PlayerState>(stateInt);
                Direction dir = static_cast<Direction>(dirInt);

                if (id == -1) continue;

                if (id != myPlayerID && !players.count(id))
                {
                    sf::Color color = playerColors[id % playerColors.size()];

                    players.try_emplace(id, PlayerFactory::create(lobbyPlayers[id].character, spawnpoints[id].x, spawnpoints[id].y, assets, fonttext, color, standardFont));
                }

                players.at(id)->setNetworkState(x, y, state, dir);

            }

            if (type == PacketType::AttackStart)
            {
                float x, y;
                int id, dir, attackID;

                packet >> id >> x >> y >> dir >> attackID;

                Direction direction = static_cast<Direction>(dir);

                if (id == -1) continue;

                if (id != myPlayerID && !players.count(id))
                {
                    sf::Color color = playerColors[id % playerColors.size()];

                    players.try_emplace(id, PlayerFactory::create(lobbyPlayers[id].character, spawnpoints[id].x, spawnpoints[id].y, assets, fonttext, color, standardFont));
                }

                players.at(id)->startAttack(players.at(id)->getAttackTable()[attackID], direction);

            }

            if (type == PacketType::ArrowShot)
            {
                int playerID;
                float xPos, yPos;
                int dir;
                int arrowID;
                float arrowGravity;

                packet >> playerID >> xPos >> yPos >> dir >> arrowID >> arrowGravity;

                int yOffset = 125.f;
              
                cout << "ARROW GRAVITY: " << arrowGravity << endl;

                if (auto* samurai = dynamic_cast<Samurai*>(players.at(playerID).get()))
                {
                    sf::Vector2f pos{ xPos, yPos + yOffset };
                    samurai->spawnArrow(pos, static_cast<Direction>(dir), arrowID, arrowGravity);
                    samurai->playBowShotSound();
                }
            }
       
            if (type == PacketType::BowPullbackSound)
            {
                int playerID;

                packet >> playerID;

                players.at(playerID)->playBowPullBackSound();
            }

            if (type == PacketType::ArrowDeflection)
            {
                cout << "ARROW DEFLECTED IN CLIENT" << endl;

                int playerID, victimID, dir, arrowID;

                float xPos, yPos, xVelo, yVelo;

                packet >> playerID >> victimID >> arrowID >> xPos >> yPos >> xVelo >> yVelo;              

                if (auto* samurai = dynamic_cast<Samurai*>(players.at(playerID).get()))
                {     
                    for (auto& arrow : samurai->getArrows())
                    {
                        samurai->playDeflectionSounds();

                        if (arrow.id == arrowID)
                        {                        
                            arrow.wasReflected = true;
                            arrow.hasBeenReflected = true;
                            arrow.hasHit = false;
                            
                            arrow.velocity = { xVelo, yVelo };
                            arrow.arrowBox.setPosition({ xPos, yPos });
                            arrow.sprite.setPosition({ xPos, yPos });

                            arrow.updateDirectionVisuals();

                            cout << arrow.arrowBox.getPosition().x << ", "
                                << arrow.arrowBox.getPosition().y << endl;

                        }
                    }           
                }

                if (players.count(playerID))
                {
                    auto& victim = players.at(victimID);

                    int scaleX = 2, scaleY = 2;
                    sf::Vector2f pos = victim->getAnimation()->getPosition();
                    DamageNumber damageNumber = victim->createDamageNumberText(standardFont, "BLOCKED!", scaleX, scaleY, sf::Color::White, pos);
                             
                    victim->getDamageNumber().push_back(std::move(damageNumber));
                }

            }

            if (type == PacketType::PlayerDamage)
            {                
                int attackerID, victimID, damage, dirInt;

                packet >> attackerID >> victimID >> damage >> dirInt;

                Direction dir = static_cast<Direction>(dirInt);

                if (players.count(victimID))
                {
                    auto& victim = players.at(victimID);

                    victim->applyDamage(damage, dir, standardFont);
                    victim->setInvul(true);
                    victim->setInvulTimer(0.5f);                   

                    victim->determineHitSound();               
                }
            }

            if (type == PacketType::Heal)
            {
                int playerID;
                int heal;

                packet >> playerID >> heal;

                auto& drinker = players.at(playerID);

                drinker->setHealth(drinker->getHealth() + heal);

                drinker->setPotionNumber(drinker->getPotionNumber() - 1);

                drinker->playPotionSound();

                int scaleX = 2, scaleY = 2;
                sf::Vector2f pos = drinker->getPlayerBox().getPosition();
                DamageNumber damageNumber = drinker->createDamageNumberText(standardFont, "+" + std::to_string(heal), scaleX, scaleY, sf::Color::Green, pos);

                drinker->getDamageNumber().push_back(std::move(damageNumber));

                cout << "HEALTH: " << drinker->getHealth() << endl;
            }

            if (type == PacketType::Death)
            {
                int playerID;

                packet >> playerID;

                cout << "INSIDE DEATH PACKET" << endl;

                if (players.count(playerID))
                {
                   players.at(playerID)->multiplayerDeath();
                   players.at(playerID)->playDeathSound();
                }       
            }

            if (type == PacketType::LobbyUpdate)
            {
                moveToServerPage = false;
                lobbyPlayers.clear();

                int count;

                packet >> hostID;
                packet >> count;

                for (int i = 0; i < count; i++)
                {
                    int id;
                    string name;
                    int characterInt;

                    packet >> id >> name >> characterInt;

                    CharacterType character = static_cast<CharacterType>(characterInt);

                    lobbyPlayers[id] = { name, character };

                    std::cout << "===== LOBBY UPDATE =====\n";

                    for (const auto& [id, player] : lobbyPlayers)
                    {
                        std::cout << "ID: " << id
                            << " | Character: "
                            << static_cast<int>(player.character)
                            << '\n';
                    }
                }
            }

            if (type == PacketType::StartLoading)
            {
                for (auto& [id,player] : players)
                {
                    std::cout << "Resetting player " << id << '\n';

                    if (!player)
                    {
                        std::cout << "Null player!\n";
                        continue;
                    }

                    player->resetPlayer(spawnpoints[id]);
                }
              
                this->startLoading = true;

            }

            if (type == PacketType::PlayerLeave)
            {
                int id;
                packet >> id;

                cout << "ERASING " << lobbyPlayers[id].name << endl;

                lobbyPlayers.erase(id);

            }

            if (type == PacketType::Deflection)
            {
                int defenderID, attackerID;

                packet >> defenderID >> attackerID;

                auto& attacker = players.at(attackerID);      

                attacker->playParrySound();
                
                attacker->applyStun(3.0f);
            }

            if (type == PacketType::ClientDisconnect)
            {
                gameSocket.disconnect();
                this->moveToServerPage = true;
            }

            if (type == PacketType::CountdownUpdate)
            {
                float remaining;
                packet >> remaining;

                this->countdownTimer = remaining;
            }

            if (type == PacketType::GameStart)
            {
                this->gameStarted = true;
                goTimer = 50.f;
            }

            if (type == PacketType::GameWinner)
            {               
                int winnerID;
                packet >> winnerID;
         
                this->moveToServerPage = false;
                this->startLoading = false;
                
                this->gameEnd = true;
                this->gameStarted = false;
                winnerName.clear();

                winnerName = lobbyPlayers[winnerID].name;

            }
        }
        else if (status == sf::Socket::Status::NotReady)
        {
            break;
        }
        else
        {
            break;
        }
    }
}

int Client::getPlayerID()
{
    return myPlayerID;
}

void Client::setPlayerName(string name)
{
    this->playerName = name;   
}

unordered_map<int, LobbyPlayer>& Client::getLobbyPlayers()
{
    return lobbyPlayers;
}

bool Client::isHost()
{
    return myPlayerID == hostID;
}

int Client::getHostID()
{
    return hostID;
}

bool Client::loadingStarted()
{
    return startLoading;
}

bool Client::moveServerPageStarted()
{
    return moveToServerPage;
}

bool Client::gameEnded()
{
    return this->gameEnd;
}

void Client::setGameEnded(bool x)
{
    this->gameEnd = x;
}

int Client::getServerID()
{
    return myServerID;
}

string Client::getWinnerName()
{
    return winnerName;
}

void Client::setCharacterType(CharacterType type)
{
    characterType = type;
}

vector<ServerInfo>& Client::getServerList()
{
    return serverList;
}


bool Client::connectToGameServer(sf::IpAddress ip, unsigned short port)
{
    moveToServerPage = false;

    gameSocket.setBlocking(true);

    std::cout << "Trying to connect to " << ip.toString() << ":" << port << std::endl;

    if (gameSocket.connect(ip, port) != sf::Socket::Status::Done)
    {
        std::cout << "Game Connection failed\n";
        return false;
    }

    gameSocket.setBlocking(false);
    std::cout << "Game Connected to server\n";
    return true;
}

bool Client::connectToLobby(sf::IpAddress ip, unsigned short port)
{
    lobbySocket.setBlocking(true);

    std::cout << "Trying to connect to " << ip.toString() << ":" << port << std::endl;

    if (lobbySocket.connect(ip, port) != sf::Socket::Status::Done)
    {
        std::cout << "Lobby Connection failed\n";
        return false;
    }

    lobbySocket.setBlocking(false);
    std::cout << "Lobby Connected to server\n";
    return true;
}

void Client::requestServerList()
{
    sf::Packet packet;

    moveToServerPage = false;

    cout << "Sending SERVER LIST" << endl;

    packet << static_cast<int>(PacketType::RequestServerList);

    this->lobbySocket.send(packet);
}


void Client::updateLobby()
{
    sf::Packet packet;

    while (lobbySocket.receive(packet) == sf::Socket::Status::Done)
    {
        int typeInt;
        packet >> typeInt;

        PacketType type = static_cast<PacketType>(typeInt);

        if (type == PacketType::ServerList)
        {
            serverList.clear();
            int count;
            packet >> count;

            for (int i = 0; i < count; i++)
            {
                ServerInfo server;
                string ipStr;

                packet >> server.id >> server.name >> ipStr >> server.port >> server.currentPlayers >> server.maxPlayers >> server.passwordProtected >> server.password;

                auto resolved = sf::IpAddress::resolve(ipStr);
                if (resolved.has_value()) {
                    server.ip = resolved.value();
                }
                else {
                    server.ip = sf::IpAddress::Any; // fallback if invalid
                }

                serverList.push_back(server);

                std::cout << server.name << " (" << server.currentPlayers << "/" << server.maxPlayers << ")\n";
            }
        }

        packet.clear();
    }
}
