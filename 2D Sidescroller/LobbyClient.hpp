#include <SFML/Network.hpp>
#include "PacketType.h"
using namespace std;

class LobbyClient
{

    private:

        sf::TcpSocket socket;

    public:

        bool connect(std::string ip, int port);

        void requestServerList();

        void createServer(std::string name, int port, int maxPlayers);
};