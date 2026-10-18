THE LAST PALADIN
----------------
A C++ / SFML 2D side-scroller with a single player adventure, online multiplayer, dedicated game servers, and Azure deployment.

Trailer:
https://www.youtube.com/watch?v=KqDAj2IHK88&t=25s

Play it Here:
https://lucaswehr.itch.io/the-last-paladin

<img width="394" height="236" alt="image" src="https://github.com/user-attachments/assets/7a8efe51-6727-4b68-b537-d288a7ae8aa0" />
<img width="369" height="240" alt="Media Player 9_30_2025 7_15_44 PM" src="https://github.com/user-attachments/assets/c1ad99cc-40da-41a1-bdc5-a76fecfc1537" />
<img width="522" height="284" alt="Media Player 9_30_2025 7_17_25 PM" src="https://github.com/user-attachments/assets/6b38e1ce-7974-4ead-88b9-85032b05e35a" />
<img width="238" height="279" alt="Media Player 9_30_2025 7_19_09 PM" src="https://github.com/user-attachments/assets/4317bb38-b4a3-48d3-af66-597fd035737b" />

OVERVIEW
---------------------------------------------------------------------
The Last Paladin is a 2D side-scrolling game I've been developing for over 1.5 years. I originally built it as a single-player C++ game in 2025 and published it on itch.io. In September 2026, I expanded the project by adding an online multiplayer battle mode, including a lobby system and dedicated game servers deployed to Microsoft Azure.

I built the game using C++, SFML 3.0, and Visual Studio, implementing the game mechanics, animations, physics, collision detection, level rendering, UI, and networking myself.

This project has been a way for me to learn more about C++ and systems programming while working on something I genuinely enjoy. Adding multiplayer pushed me to learn how computers communicate over the internet, how client-server architectures work, and how to deploy and manage server applications in the cloud.

FEATURES
---------------------------------------------------------------------
- Single player sides-scroller combat with multiple enemies and boss encounters
- 3 Unique enemies and 2 playable characters, each with their own unique set of moves and stats
- Custom animations handler, physics, level editor, and collision detection
- Online Multiplayer battle mode
- TCP Client-server communication
- Lobby-based server discovery and match creation
- Dedicated game servers running on docker containers on Azure
- Automatic game-server clean up when matches are empty

Game Architecture:
---------------------------------------------------------------------

**Why I chose SFML:**

I'm using SFML, a C++ library that handles windows, 2D graphics, audio, input, and networking, so you don’t have to deal with the low-level OS and graphics APIs yourself. 

I chose to use this library because I had prior knowledge of it through coursework in college, and because it teaches you about C++ way more than engines like Godot and Unity where a lot of stuff is done for you (Physics, animations, and more are done for you). 

The tradeoff of course is that I would have to create physics, animations, level design, UI, etc. myself which took time. 

**High-Level Architecture:**

The single player mode runs locally while the multiplayer uses a client-server architecture that has a lobby contianer to spins up game servers on Azure

<img width="656" height="452" alt="multiplayerDiagram" src="https://github.com/user-attachments/assets/4aa25e19-a999-4104-9171-61f5ed3fbb85" />


The following picture is an AI-Generated diagram of what's actually happening when clients create servers and join them.

You can see that when a client creates a server, the only thing that happens client-side is a request to create one. 

Then lobby server handles the rest where it takes the request and directly connects with Azure to spin up a game server with the given parameters (Server name, password, player count, etc.) 

Azure then runs a brand-new game-server container where clients can directly connect to.

The lobby server does more than that which is shown above, but it basically oversees the game-servers and tracks active servers by getting heartbeats from them to show that they're still running. (if the server isn't showing a heartbeat, it gets removed from the server list and deletes the game-server container associated with it)

It also has a list that is shown on the client side of active servers where clients can click on the name of the server to directly connect to that specific multiplayer game.

KEY ENGINEERING DECISIONS
--------------------------------------------------------------------
**TCP over UDP:**

When I did research on how multiplayer games send packets with each other, it's obvious to use UDP since it sends packets constantly and doesn't need a connection to start sending. This makes the experience much better because games need constant packets sending every second. TCP is usually not the right choice because it's very strict with packet ordering and connection between the two clients first. Head of line blocking is an issue with games because if a packet is lost, the rest of the packets have to wait until the lost packet is retransmitted before sending the rest. For online games, that could cause lag spikes and a bad user experience. 

For The Last Paladin however, I chose TCP because the game is relatively small in scope, with a limited number of players and gameplay events. Reliable, ordered delivery simplifies synchronization and reduces the amount of custom networking logic I need to implement. Using UDP would give me more control over latency and packet delivery, but would also require additional logic wherever the game depends on ordering, duplicate suppression, or reliable delivery.

This was a tradeoff between performance and implementation complexity. For the current scale of the game, TCP provides a simpler and sufficiently reliable networking model. If the game evolved to require more frequent real-time updates or became more sensitive to latency under packet loss, I would consider UDP or a hybrid approach that uses different transport strategies for different types of gameplay data.


**Separating the Game-Sever and Lobby-Sever:**

I separated the game-server from the lobby-server itself. This decision was made because it would avoid huge complexity having one component do two entirely different things. Having it separated keeps it simple to understand and easier to manage since both run independent of each other. Now lobby-server just needs to manage these game-servers and handle requests to create or find a match. Game-server would just need to manage the game itself, such as animations, combat, health, physics, etc. sent out to the server and sent back out to all clients.

Having it like this means that these components would need a way to communicate via an extra network connection, so additional logic is needed to make this work, but I would say the labor to make this happen is worth it having simplicity and scalability in the future.


**Docker and Azure:**

I containerized the lobby and game server so they can run in a consistent environment outside my local development machine.

Azure Container Instances runs the deployed server containers, while the lobby uses Azure's management API to create and delete game-server instances. The lobby obtains the actual public IP address assigned to each game-server container and returns it to the client.

This required handling asynchronous provisioning, public-IP discovery, server readiness, authentication, and cleanup rather than simply launching a local executable.

Server lifecycle management
--------------------------------------------------

The lobby tracks each dedicated game server using its server ID. Heartbeats update the server's player count and last-seen time.

When a server stops sending heartbeats, the lobby can identify it as unresponsive. When a server remains empty for the configured timeout, the lobby requests deletion of its Azure container and removes it from the available server list after successful deletion.

This makes server cleanup part of the application's architecture instead of requiring manual cleanup after every match.

Cloud Deployment
--------------------------------------------------

The multiplayer infrastructure uses Docker and Microsoft Azure.

**Docker:** Packages the lobby and game-server applications into container images.

**Azure Container Instances:** Runs the deployed lobby and dedicated game servers.

**Azure management API:** Allows the lobby to provision and delete game-server containers.

**Managed identity:** Allows the lobby to authenticate to Azure without embedding a long-lived Azure management credential in the application.

**TCP ports:** The lobby listens on port 54001, and dedicated game servers listen on port 54000.

The lobby and game-server containers are built and pushed to Azure Container Registry before deployment.

One challenge was discovering the correct public IP address for a game server. The outbound IP reported from inside a container is not necessarily the container's public IP. The lobby therefore retrieves the assigned IP address through Azure's management API before returning the connection details to the client.

Another challenge was handling the fact that container provisioning is asynchronous. The lobby must wait for the game server to become available instead of assuming it can accept connections as soon as the creation request succeeds.

Download Instructions
----------------------

1.) Extract zip file

2.) Click on '2D Sidescroller' executable

Building on Linux
-----------------

Requires a C++17 compiler, CMake 3.26+, and SFML 3 (e.g. `sudo pacman -S sfml cmake` on Arch, or build SFML 3 from source on distros that only package SFML 2).

```sh
cmake -S . -B build
cmake --build build -j
cmake --build build --target run    # or: cd build && ./the_last_paladin
```

Assets are copied next to the executable at build time; the game loads them relative to the working directory, so run it from `build/`. Pass `-DBUILD_LOBBY_SERVER=ON` (needs libcurl) to also build the lobby server.

Self-hosting the lobby (no Azure)
---------------------------------

The lobby can run game servers as child processes instead of Azure containers:

```sh
docker compose up -d --build     # see compose.yaml; set PUBLIC_HOST to the address clients use
```

| Variable | Where | Meaning |
|---|---|---|
| `LOBBY_BACKEND=local` | lobby | Spawn `./the_last_paladin server` per game instead of calling Azure |
| `PUBLIC_HOST` | lobby | IP clients connect to (e.g. the Tailscale IP). Required in local mode |
| `GAME_PORT_BASE` | lobby | Game server for id N listens on base+N (default 54010) |
| `LOBBY_HOST` / `LOBBY_PORT` | client | Lobby to connect to (default: the Azure host, 54001) |

Players run the client with `LOBBY_HOST=<tailscale ip> ./the_last_paladin` and must be on the same tailnet.
