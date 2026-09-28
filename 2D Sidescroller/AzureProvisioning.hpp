#pragma once
#include "ServerInfo.h"
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

static size_t curlWriteCallback(
    char* ptr,
    size_t size,
    size_t nmemb,
    void* userdata)
{
    std::string* response =
        static_cast<std::string*>(userdata);

    size_t total = size * nmemb;

    response->append(ptr, total);

    return total;
}

inline std::string getAzureAccessToken()
{
    CURL* curl = curl_easy_init();

    if (!curl)
    {
        std::cout << "Failed to initialize CURL\n";
        return "";
    }

    std::string response;

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Metadata: true");

    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        "http://169.254.169.254/metadata/identity/oauth2/token"
        "?api-version=2018-02-01"
        "&resource=https%3A%2F%2Fmanagement.azure.com%2F"
    );

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        curlWriteCallback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response
    );

    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        std::cout << "Azure token request failed: "
            << curl_easy_strerror(result)
            << '\n';
    }
    else
    {
        std::cout << "Azure token request succeeded\n";
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

inline std::string extractAccessToken(const std::string& response)
{
    std::string key = "\"access_token\":\"";

    size_t start = response.find(key);

    if (start == std::string::npos)
    {
        std::cout << "Could not find access token\n";
        return "";
    }

    start += key.length();

    size_t end = response.find("\"", start);

    if (end == std::string::npos)
    {
        std::cout << "Could not find end of access token\n";
        return "";
    }

    return response.substr(start, end - start);
}

inline bool createGameServer(const std::string& accessToken, int serverID, const ServerInfo& server, const std::string& acrPassword)
{
    CURL* curl = curl_easy_init();

    if (!curl)
    {
        std::cout << "Failed to initialize CURL\n";
        return false;
    }

    std::string response;

    std::string url =
        "https://management.azure.com"
        "/subscriptions/9392b348-3738-4cc6-8444-5b00cfa9601a"
        "/resourceGroups/last-paladin-rg"
        "/providers/Microsoft.ContainerInstance"
        "/containerGroups/game-server-" + std::to_string(serverID) +
        "?api-version=2023-05-01";

    std::string passwordProtected =
        server.passwordProtected ? "1" : "0";

    std::string json =
        "{"
        "\"location\":\"westus\","
        "\"properties\":{"
        "\"imageRegistryCredentials\":[{"
        "\"server\":\"lastpaladinregistry.azurecr.io\","
        "\"username\":\"LastPaladinRegistry\","
        "\"password\":\"" + acrPassword + "\""
        "}],"
        "\"containers\":[{"
        "\"name\":\"game-server\","
        "\"properties\":{"
        "\"image\":\"lastpaladinregistry.azurecr.io/last-paladin-server:latest\","
        "\"resources\":{"
        "\"requests\":{"
        "\"cpu\":1,"
        "\"memoryInGB\":1.5"
        "}"
        "},"
        "\"ports\":[{"
        "\"port\":54000,"
        "\"protocol\":\"TCP\""
        "}],"
        "\"environmentVariables\":["
        "{"
        "\"name\":\"SERVER_NAME\","
        "\"value\":\"" + server.name + "\""
        "},"
        "{"
        "\"name\":\"SERVER_PORT\","
        "\"value\":\"" + std::to_string(server.port) + "\""
        "},"
        "{"
        "\"name\":\"SERVER_MAX_PLAYERS\","
        "\"value\":\"" + std::to_string(server.maxPlayers) + "\""
        "},"
        "{"
        "\"name\":\"SERVER_PASSWORD_PROTECTED\","
        "\"value\":\"" + passwordProtected + "\""
        "},"
        "{"
        "\"name\":\"SERVER_PASSWORD\","
        "\"value\":\"" + server.password + "\""
        "},"
        "{"
        "\"name\":\"SERVER_ID\","
        "\"value\":\"" + std::to_string(serverID) + "\""
        "},"
        "{"
        "\"name\":\"LOBBY_HOST\","
        "\"value\":\"last-paladin-lobby-lucas.westus.azurecontainer.io\""
        "}"
        "]"
        "}"
        "}],"
        "\"osType\":\"Linux\","
        "\"restartPolicy\":\"OnFailure\","
        "\"ipAddress\":{"
        "\"type\":\"Public\","
        "\"ports\":[{"
        "\"port\":54000,"
        "\"protocol\":\"TCP\""
        "}]"
        "}"
        "}"
        "}";

    curl_slist* headers = nullptr;

    headers = curl_slist_append(
        headers,
        ("Authorization: Bearer " + accessToken).c_str()
    );

    headers = curl_slist_append(
        headers,
        "Content-Type: application/json"
    );

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        curlWriteCallback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response
    );

    CURLcode result = curl_easy_perform(curl);

    long responseCode = 0;

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &responseCode
    );

    std::cout << "Azure create server HTTP code: "
        << responseCode << '\n';

    if (result != CURLE_OK)
    {
        std::cout << "Azure create request failed: "
            << curl_easy_strerror(result)
            << '\n';

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return false;
    }

    std::cout << "Azure response:\n";
    std::cout << response << '\n';

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return responseCode >= 200 && responseCode < 300;
}

inline std::string getGameServerPublicIP(
    const std::string& accessToken,
    int serverID)
{
    CURL* curl = curl_easy_init();

    if (!curl)
    {
        std::cout << "Failed to initialize CURL\n";
        return "";
    }

    std::string response;

    std::string url =
        "https://management.azure.com"
        "/subscriptions/9392b348-3738-4cc6-8444-5b00cfa9601a"
        "/resourceGroups/last-paladin-rg"
        "/providers/Microsoft.ContainerInstance"
        "/containerGroups/game-server-" + std::to_string(serverID) +
        "?api-version=2023-05-01";

    curl_slist* headers = nullptr;

    headers = curl_slist_append(
        headers,
        ("Authorization: Bearer " + accessToken).c_str()
    );

    headers = curl_slist_append(
        headers,
        "Content-Type: application/json"
    );

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        curlWriteCallback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response
    );

    CURLcode result = curl_easy_perform(curl);

    long responseCode = 0;

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &responseCode
    );

    std::cout << "Azure get server HTTP code: "
        << responseCode << '\n';

    if (result != CURLE_OK)
    {
        std::cout << "Azure get server request failed: "
            << curl_easy_strerror(result)
            << '\n';

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return "";
    }

    std::cout << "Azure GET response:\n";
    std::cout << response << '\n';

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (responseCode < 200 || responseCode >= 300)
    {
        std::cout << "Azure GET request returned an error\n";
        return "";
    }

    // Find the public IP inside:
    // "ipAddress": {
    //     "ip": "52.x.x.x"
    // }

    std::string key = "\"ip\": \"";

    size_t start = response.find(key);

    if (start == std::string::npos)
    {
        std::cout << "Could not find public IP in Azure response\n";
        return "";
    }

    start += key.length();

    size_t end = response.find("\"", start);

    if (end == std::string::npos)
    {
        std::cout << "Could not find end of public IP\n";
        return "";
    }

    return response.substr(start, end - start);
}

inline std::string waitForGameServerPublicIP(
    const std::string& accessToken,
    int serverID)
{
    const int maxAttempts = 180;

    for (int attempt = 0; attempt < maxAttempts; attempt++)
    {
        std::cout << "Checking game server status... "
            << (attempt + 1)
            << "/"
            << maxAttempts
            << std::endl;

        std::string ip = getGameServerPublicIP(
            accessToken,
            serverID
        );

        if (!ip.empty())
        {
            std::cout << "Game server is ready at "
                << ip
                << '\n';

            return ip;
        }

        std::cout << "Game server is not ready yet. Waiting..."
            << std::endl;

        std::this_thread::sleep_for(
            std::chrono::seconds(2)
        );
    }

    std::cout << "Timed out waiting for game server\n";

    return "";
}


inline bool deleteGameServer(const std::string& accessToken,int serverID)
{
    CURL* curl = curl_easy_init();

    if (!curl)
    {
        std::cout << "Failed to initialize CURL for deletion\n";
        return false;
    }

    std::string url =
        "https://management.azure.com/subscriptions/"
        "9392b348-3738-4cc6-8444-5b00cfa9601a"
        "/resourceGroups/last-paladin-rg"
        "/providers/Microsoft.ContainerInstance"
        "/containerGroups/game-server-" +
        std::to_string(serverID) +
        "?api-version=2023-05-01";

    struct curl_slist* headers = nullptr;

    headers = curl_slist_append(
        headers,
        ("Authorization: Bearer " + accessToken).c_str());

    headers = curl_slist_append(
        headers,
        "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        std::cout << "Azure delete request failed: "
            << curl_easy_strerror(res)
            << '\n';

        return false;
    }

    std::cout << "Azure delete response: "
        << responseCode << '\n';

    if (responseCode == 200 ||
        responseCode == 202 ||
        responseCode == 204)
    {
        std::cout << "Game server "
            << serverID
            << " deletion started successfully.\n";

        return true;
    }

    std::cout << "Failed to delete game server "
        << serverID
        << ". Azure response code: "
        << responseCode << '\n';

    return false;
}