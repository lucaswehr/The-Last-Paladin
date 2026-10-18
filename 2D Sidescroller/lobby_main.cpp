#include "LobbyServer.hpp"
#include "AzureProvisioning.hpp"
#include <cstdlib>
#include <string>

int main()
{
    const char* backend = std::getenv("LOBBY_BACKEND");
    if (backend && std::string(backend) == "local")
    {
        const char* host = std::getenv("PUBLIC_HOST");
        if (!host || !*host)
        {
            std::cout << "LOBBY_BACKEND=local requires PUBLIC_HOST (the address clients should connect to)\n";
            return 1;
        }
        std::cout << "Local backend: game servers are child processes, advertised at " << host << "\n";
        LobbyServer lobby;
        lobby.start(54001);
        return 0;
    }

    std::string tokenResponse = getAzureAccessToken();

    std::string accessToken = extractAccessToken(tokenResponse);

    if (accessToken.empty())
    {
        std::cout << "Failed to extract Azure access token\n";
        return 1;
    }

    std::cout << "Azure access token obtained successfully\n";

    LobbyServer lobby;
    lobby.start(54001);

    return 0;
}