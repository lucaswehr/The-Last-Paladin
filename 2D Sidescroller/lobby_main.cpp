#include "LobbyServer.hpp"

int main()
{
    LobbyServer lobby;
    lobby.start(54001);

    return 0;
}