#include "LobbyServer.hpp"
#include "AzureProvisioning.hpp"

int main()
{
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