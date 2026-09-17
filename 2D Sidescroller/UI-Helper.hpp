#include "ServerInfo.h"
#include "Animation.hpp"
#include "TextBox.hpp"

using namespace std;

ServerInfo extractServerInfoFromUI(TextBox& nameBox, TextBox& passwordBox, int maxPlayers, unsigned short port, sf::IpAddress ip)
{
	ServerInfo info;

	info.name = nameBox.str;
	info.password = passwordBox.str;
	info.maxPlayers = maxPlayers;
	info.port = port;
	info.ip = ip;
	info.passwordProtected = !passwordBox.str.empty(); // False if no password is given
	info.currentPlayers = 1; // Creating server only has 1 Player initally 

	cout << "PASSWORD: [" << info.password << "]" << endl;
	cout << "PASSWORD PROTECTED: "
		<< info.passwordProtected << endl;

	return info;
}

