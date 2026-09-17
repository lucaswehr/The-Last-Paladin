#pragma once
#include <SFML/Network.hpp>
#include "PlayerState.hpp"
#include "Animation.hpp"
#include "Knight.hpp"
#include "ServerInfo.h"
#include "PlayerTextures.h"
#include "Player.hpp"
#include "AssetManager.hpp"
#include "PlayerFactory.hpp"
#include "CharacterType.h"

using namespace std;

struct LobbyPlayer 
{
    string name;
    CharacterType character;
};


class Client
{
public:
    //Client(const sf::IpAddress& serverIp, unsigned short port);

    Client() = default;
  
    bool connectToGameServer(sf::IpAddress ip, unsigned short port);
    bool connectToLobby(sf::IpAddress ip, unsigned short port);
    void runClientServer();
    void requestServerList();
    void updateLobby();

    void sendPlayerState(int playerID, float x, float y, PlayerState state, Direction dir);
    void sendAttack(int playerID, float x, float y, Direction dir, int attackID);
    void sendDamage(int playerID, int victimID, int damage, Direction dir);
    void sendDeath(int playerID);
    void sendDeflect(int defenderID, int attackerID);
    void sendHeal(int playerID, int healAmount);
    void sendArrow(int playerID, float xPos, float yPos, Direction dir, int arrowID, float arrowGravity);
    void sendArrowDeflection(int playerID, int victimID, int arrowID, sf::Vector2f arrowPos, sf::Vector2f arrowVelocity);
    void sendInvulData(int victimID);
    void sendBowPullbackSound(int playerID);
    void sendCharacterChange(int playerID, CharacterType newCharacter);
    void createServer(const ServerInfo& info);

    void leaveLobby();
    void rebuildLobbyPlayers();

    void receiveNetworkEvent(std::unordered_map<int, std::unique_ptr<Player>>& players, float dt, std::vector<sf::Color>& playerColors, vector<sf::Vector2f> spawnpoints,
       AssetManager& assets,std::string& fonttext, CharacterType type, sf::Font& standardFont);
  
    int getPlayerID();
    void setPlayerName(string name);
    unordered_map<int, LobbyPlayer>& getLobbyPlayers();
    bool isHost();
    int getHostID();
    bool loadingStarted();
    bool moveServerPageStarted();
    bool gameEnded();
    void setGameEnded(bool x);
    int getServerID();

    string getWinnerName();

    void setCharacterType(CharacterType type);

    vector<ServerInfo>& getServerList();

    sf::TcpSocket lobbySocket;
    sf::TcpSocket gameSocket;
    int maxLobbySize = 0;
    bool startGame = false;
    float countdownTimer = -1.f;
    bool gameStarted = false;
    bool gameEnd = false;

    int goTimer;

private:

    int myPlayerID = -1;
    int hostID = -1;
    int myServerID = -1;

    string playerName;
    unordered_map<int, LobbyPlayer> lobbyPlayers;
    vector<ServerInfo> serverList;

    bool startLoading = false;
    bool moveToServerPage = false;

    CharacterType characterType;

    string winnerName;
    
   /* void sendInput();
    void receiveState();*/
};