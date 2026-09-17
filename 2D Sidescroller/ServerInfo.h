#pragma once
#include <SFML/Network.hpp>
#include <iostream>

struct ServerInfo
{
    int id;
    std::string name;
    std::string playerName;
    sf::IpAddress ip = sf::IpAddress::Any;
    unsigned short port;

    int currentPlayers;
    int maxPlayers;

    bool passwordProtected;
    std::string password;

    sf::Clock heartBeat; // If server doesnt respond for around 5 seconds, delete server (Prevents server still up if host crashes_
};