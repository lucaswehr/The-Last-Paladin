#include "Animation.hpp"
#include "Knight.hpp"
#include "Tile.hpp"
#include "wolf.hpp"
#include "Enemy.hpp"
#include "Skeleton.hpp"
#include "Arrow.hpp"
#include "Dragon.hpp"
#include "text.hpp"
#include "Healthbar.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Level.hpp"
#include "LobbyServer.hpp"
#include "TextBox.hpp"
#include "UI-Helper.hpp"
#include "HelperMain.hpp"
#include "PlayerTextures.h"
#include "DebugOverlay.hpp"
#include "Samurai.hpp"
#include "AssetManager.hpp"
#include "CharacterType.h"
#include "PlayerFactory.hpp"
#include "DeflectionDemo.hpp"
#include "ParryDemo.hpp"
#include <thread>
#include <cstdlib>
#include <ctime>
#include <atomic>


enum class GameState
{
    mainMenu,
    playing,
    paused,
    howtoPlay,
    howToPlay2,
    howToPlay3,
    specialInteractions,
    specialInteractions2,
    gameOver,
    level1Pause,
    enterUsername,
    characterSelect,
    joinServer,
    createServer,
    serverPage,
    enterPassword,
    loading,
    preGameLobby,
    countdown,
    battleMode,
    end,
    testingGrounds


};

std::vector<sf::Color> playerColors =
{
   sf::Color(170, 20, 20),    // Brighter Blood Red
   sf::Color(25, 55, 120),    // Dark Blue
   sf::Color(180, 150, 30),   // Antique Gold
   sf::Color(34, 139, 34)     // Forest Green
};

std::vector<sf::Vector2f> spawnpoints = {
    {350.f, 690.f},
    {1550.f,692.f},
    {250.f,213.f},
    {1700.f,117.f}
};

struct ServerEntryUI
{
    sf::FloatRect bounds;
    sf::Text text;
    bool hovered = false;
    int serverIndex;

    ServerEntryUI(const sf::Font& font)
        : text(font)  // construct sf::Text with a valid font
    {
    }
};

int main(int argc, char** argv)
{

    //// ---------------------NETWORKING--------------------------//

        Client client;
        sf::IpAddress Ip(127,0,0,1); // My IP
      
        srand(static_cast<unsigned>(time(nullptr)));
        unsigned short gamePort = 54000 + (rand() % 1000);
     
        unsigned short lobbyPort = 54001;  

      
        if (argc > 1 && std::string(argv[1]) == "lobby")
        {
            LobbyServer lobby;
            lobby.start(lobbyPort);
            return 0;
        }
        else if (argc > 1 && std::string(argv[1]) == "server")
        {
            ServerInfo info;

            info.name = "Test Server";
            info.port = 54000;
            info.maxPlayers = 3;
            info.passwordProtected = true;
            info.password = "test";

            Server server(info.port);

            server.connectToLobby("lobby", lobbyPort);
            server.createServer(info);

            server.runRelayServer();
        }
      
    //// -------------------NETWORKING END--------------------------//

    constexpr float GAME_WIDTH = 1920.f;
    constexpr float GAME_HEIGHT = 1080.f;

    std::srand(std::time(0));
    std::cout << "Game starting..." << std::endl;

    GameState gameState = GameState::mainMenu;


    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();

    sf::RenderWindow window(desktopMode, "The Last Paladin", sf::Style::Default);
    window.setVerticalSyncEnabled(true);
    window.setMouseCursorVisible(false);

    sf::Vector2u windowSize = window.getSize();
    float windowWidth = (float)windowSize.x;
    float windowHeight = (float)windowSize.y;
    const float margin = 200.f;
    
    sf::View menuView(sf::FloatRect({ 0.f, 0.f },{ GAME_WIDTH, GAME_HEIGHT }));
    sf::View gameView(sf::FloatRect({ 0.f, 0.f }, { GAME_WIDTH, GAME_HEIGHT }));
    sf::View battleView(sf::FloatRect({ 0.f, 0.f }, { GAME_WIDTH, GAME_HEIGHT }));

    window.setFramerateLimit(60);
    sf::Clock clock;

    sf::Clock levelDelay;
    sf::Time levelTime = sf::seconds(3);

    sf::Clock deathDelay;
    sf::Time deathTime = sf::seconds(2);

    sf::Clock backToMenuDelay;
    sf::Time backToMenuTime = sf::seconds(5);

    sf::Clock endDelay;
    sf::Time endTime = sf::seconds(5);

    sf::Clock endDelayToMenu;
    sf::Time endTimeToMenu = sf::seconds(5);

    sf::Clock networkClock;

    sf::Font font;
    if (!font.openFromFile("Fonts/OldLondon.ttf"))
    {
        std::cout << "Error" << std::endl;
        return 0;
    }

    sf::Font standardFont;
    if (!standardFont.openFromFile("Fonts/Jersey10-Regular.ttf"))
    {
        std::cout << "Error" << std::endl;
        return 0;
    }
    
    std::string fancyFontText = "Fonts/OldLondon.ttf";
    std::string standardFontText = "Fonts/Jersey10-Regular.ttf";
   
    Text title(fancyFontText, "The Last Paladin", 300.f, 60.f, sf::Color::White, 200);
    Text play(fancyFontText, "Play", 830.f, 370.f, sf::Color::White, 130);
    Text BattleMode(fancyFontText, "Battle Mode", 650.f, 530.f, sf::Color::White, 130);
    Text howToPlay(fancyFontText, "How To Play", 620.f, 700.f, sf::Color::White, 130);
    Text exit(fancyFontText, "Exit", 830.f, 870.f, sf::Color::White, 130);

    Text levelOne(fancyFontText, "Zone 1: ", 780.f, 450.f, sf::Color::White, 130);
    Text Menu(fancyFontText, "Menu", 100.f, 900.f, sf::Color::White, 130);
    Text previousHowToPlayPage(fancyFontText, "Back", 450.f, 900.f, sf::Color::White, 130);
    Text Next(fancyFontText, "Next", 1600.f, 900.f, sf::Color::White, 130);

    Text CreateServer(fancyFontText, "Create Server", 600.f, 320.f, sf::Color::White, 150);
    Text JoinServer(fancyFontText, "Join Server", 650.f, 600.f, sf::Color::White, 150);
    Text Back(fancyFontText, "Back", 50.f, 900.f, sf::Color::White, 130);

    Text Username(fancyFontText, "Username:", 560.f, 350.f, sf::Color::White, 100);
    TextBox nametag(standardFont, 550.f, 500.f, 800.f, 150.f, 70);

    Text Continue(fancyFontText, "Continue", 680.f, 800.f, sf::Color::White, 150);

    Text ServerName(fancyFontText, "Server Name:", 640.f, 300.f, sf::Color::White, 50);
    TextBox serverNameBox(standardFont, 650.f, 380.f, 600.f, 75.f, 40);

    Text Password(fancyFontText, "Password:", 640.f, 470.f, sf::Color::White, 50);
    TextBox passwordBox(standardFont, 650.f, 550.f, 600.f, 75.f, 40);

    Text EnterPassword(fancyFontText, "Enter Password:", 560.f, 350.f, sf::Color::White, 100);
    TextBox passwordAttemptBox(standardFont, 550.f, 500.f, 800.f, 150.f, 70);
    Text errorMessage(fancyFontText, "Incorrect Password", 585.f, 675.f, sf::Color::Red, 50);
    int errorMessageCount = 0;
    bool showErrorMessage = false;

    Text Create(fancyFontText, "Create", 775.f, 850.f, sf::Color::White, 150);
    Text Loading(fancyFontText, "Loading...", 600.f, 400.f, sf::Color::White, 200);
    Text Refresh(fancyFontText, "Refresh", 1425.f, 850.f, sf::Color::White, 50);

    Text Leave(fancyFontText, "Leave", 50.f, 900.f, sf::Color::White, 100);
    Text characterSelectLobby(fancyFontText, "Character Select", 275.f, 390.f, sf::Color::White, 80);
    //Text DeleteServer(fancyFontText, "Delete Server", 305.f, 440.f, sf::Color::White, 80);
    Text StartGame(fancyFontText, "Start Game", 325.f, 575.f, sf::Color::White, 80);   
    Text Arena(fancyFontText, "Arena", 1300.f, 575.f, sf::Color::White, 80);
   
    Text howToPlay2(fancyFontText, "How To Play:", 600.f, 0.f, sf::Color::White, 130);
    Text rules(fancyFontText, "walk: A/D      Jump: Space    Run: walk + LShift   Attack: LClick", 75.f, 400.f, sf::Color::White, 70);
    Text rules2(fancyFontText, "Special: S      Roll: RClick         Climb: Space          Heal: R", 75.f, 800.f, sf::Color::White, 70);

    Text rules3(fancyFontText, "Sprint Attack        Jump Attack", 500.f, 250.f, sf::Color::White, 70);
    Text rules4(fancyFontText, "  Shift + LClick                Space + LClick", 500.f, 700.f, sf::Color::White, 50);

    Text samuraiRules(fancyFontText, "walk: A/D      Jump: Space    Run: walk + LShift   Attack: LClick", 75.f, 400.f, sf::Color::White, 70);
    Text samuraiRules2(fancyFontText, "           Special: S          Climb: Space          Heal: R", 75.f, 800.f, sf::Color::White, 70);

    Text specialInteractions(fancyFontText, "Special Interactions (Deflection):", 200.f, 0.f, sf::Color::White, 130);
    Text deflectionInfoBox(standardFontText, "Note: Reflected arrows deal 1.5x more damage (25 dmg -> 40 dmg)", 375.f, 725.f, sf::Color::White, 50);

    Text specialInteractions2(fancyFontText, "Special Interactions (Parry):", 300.f, 0.f, sf::Color::White, 130);
    Text parryInfoBox(standardFontText, "Note: Any attack hitting a player in a stunned state is 1.5x more damage", 340.f, 725.f, sf::Color::White, 50);

    Text countdownText(fancyFontText, "", 800.f, 100.f, sf::Color::White, 400);

    Text characterSelect(fancyFontText, "Charcater Select", 560.f, 0.f, sf::Color::White, 130);
    Text knightText(fancyFontText, "Paladin", 570.f, 725.f, sf::Color::White,70);
    Text samuraiText(fancyFontText, "Samurai",1100.f, 725.f, sf::Color::White, 70);
    Text howToPlayCharacterSelect(fancyFontText, "How To Play", 1375.f, 925.f, sf::Color::White, 90);

    DebugOverlay debug(font);

    sf::Text names(font);
    names.setOutlineColor(sf::Color::Black);
    names.setOutlineThickness(3);

    sf::Music menuMusic;
    menuMusic.openFromFile("Sounds/Kings_Feast.mp3");
    menuMusic.setLooping(true);
    menuMusic.play();

    bool playSong = true;
    bool playSong2 = true;
    bool playSong3 = true;
    bool playSong4 = true;
    bool playMenuSound = false;
    bool playMenuSound2 = false;
    bool playMenuSound3 = false;
    bool playMenuSound4 = false;
    bool playMenuSound5 = false;
    bool playMenuSound6 = false;
    bool playMenuSound7 = false;
    bool playMenuSound8 = false;
    bool playMenuSound9 = false;
    bool playMenuSound10 = false;
    bool playMenuSound11 = false;
    bool playMenuSound12 = false;
    
    bool wasAttackingLastFrame = false;
    int attackID = 0;
    bool sendDeathPacketOnce = false;
    int myID;
    int myIDCharacterSelect;
    float spawnY;
    int slot = 0;
    bool isAttackingNow = false;
    bool rebuildServerUI = false;
    int lastServerCount = 0;
    int maxPlayers = 2;
    std::atomic<bool> serverReady = false;
    bool serverLaunched = false;
    sf::Clock loadingClock;
    bool characterSelected = false;
    bool countdownStarted = false;

    int entryServerIndex = -1;

    sf::Clock fillerGapCountdown;
    sf::Clock countdownClock;
    int countdown = 3;
    bool showGo = false;
    bool finished = false;

    bool winClockStarted = false;

    float alphaWinner = 0.f;
    float winDelay = 5.f;
    float fadeDuration = 2.f;

    sf::Clock winClock;
    sf::Clock pauseBeforeBackToLobby;
    bool pauseToLobby = false;
    bool fadeStarted = false;

    attackType attackTypes = attackType::none;

    bool playWinnerSound = false;

    bool pressedLobbyCharacterSelectButton = false;

    const auto CursorHand = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand).value();
    const auto CursorArrow = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();

    sf::RectangleShape maxPlayersBox;
    maxPlayersBox.setSize({ 50, 50 });
    maxPlayersBox.setPosition({ 915, 710 });
    maxPlayersBox.setFillColor(sf::Color::White);
    maxPlayersBox.setOutlineColor(sf::Color::Black);
    maxPlayersBox.setOutlineThickness(2);

    sf::Text maxPlayersText(font);
    maxPlayersText.setCharacterSize(50);
    maxPlayersText.setOutlineThickness(5);
    maxPlayersText.setFillColor(sf::Color::White);
    maxPlayersText.setPosition({ 650, 700 });

    sf::Texture inventoryText;
    if (!inventoryText.loadFromFile("GUI/Inventory.png"))
    {
        std::cout << "error loading the knight inventory sprite" << std::endl;
    }

    sf::Sprite inventory(inventoryText);

    inventory.setTextureRect(sf::IntRect({ 3 * 28, 5 * 32 }, { 32, 32 }));
    inventory.setScale({ 5, 7 });


    sf::Sprite multiplayerInventory(inventoryText);

    multiplayerInventory.setTextureRect(sf::IntRect({ 3 * 28, 5 * 32 }, { 32, 32 }));
    multiplayerInventory.setScale({ 3, 5 });

    sf::Texture bannerText;
    if (!bannerText.loadFromFile("GUI/Banners.png"))
    {
        std::cout << "error loading the knight banner sprite" << std::endl;
    }

    sf::Sprite banner(bannerText);
    sf::Sprite banner2(bannerText);
    sf::Sprite banner3(bannerText);
    sf::Sprite banner4(bannerText);
    sf::Sprite banner5(bannerText);
    sf::Sprite banner6(bannerText);
    sf::Sprite banner7(bannerText);
    sf::Sprite banner8(bannerText);

    sf::Sprite serverBanner1(bannerText);
    sf::Sprite serverBanner2(bannerText);
    sf::Sprite serverBanner3(bannerText);
    sf::Sprite serverBanner4(bannerText);
    sf::Sprite serverBanner5(bannerText);
    sf::Sprite serverBanner6(bannerText);

    sf::Sprite serverBanner7(bannerText);
    sf::Sprite serverBanner8(bannerText);
    sf::Sprite serverBanner9(bannerText);
    sf::Sprite serverBanner10(bannerText);
    sf::Sprite serverBanner11(bannerText);

    sf::Sprite statsBanner1(bannerText);
    sf::Sprite statsBanner2(bannerText);
    sf::Sprite statsBanner3(bannerText);
    sf::Sprite statsBanner4(bannerText);
  
    banner2.setTextureRect(sf::IntRect({ 2 * 32, 7 * 32 }, { 32, 32 }));
    banner2.setScale({ 8, 8 });
    banner2.setPosition({ 200,50 });

    banner.setTextureRect(sf::IntRect({ 3 * 27, 7 *32 }, { 32, 32 }));
    banner.setScale({ 8, 8 });
    banner.setPosition({ 300,50 });

    banner3.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    banner3.setScale({ 8, 8 });
    banner3.setPosition({ 550,50 });

    banner4.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    banner4.setScale({ 8, 8 });
    banner4.setPosition({ 800,50 });

    banner5.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    banner5.setScale({ 8, 8 });
    banner5.setPosition({ 1050,50 });

    banner6.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    banner6.setScale({ 8, 8 });
    banner6.setPosition({ 1300,50 });

    banner7.setTextureRect(sf::IntRect({ 3 * 32, 7 * 32 }, { 32, 32 }));
    banner7.setScale({ 8, 8 });
    banner7.setPosition({ 1450, 50 });

    // Create Server Banner
    serverBanner1.setTextureRect(sf::IntRect({ 2 * 32, 7 * 32 }, { 32, 32 }));
    serverBanner1.setScale({ 8, 8 });
    serverBanner1.setPosition({ 500,275 });

    serverBanner2.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    serverBanner2.setScale({ 8, 8 });
    serverBanner2.setPosition({ 750,275 });

    serverBanner3.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    serverBanner3.setScale({ 8, 8 });
    serverBanner3.setPosition({ 950,275 });

    serverBanner4.setTextureRect(sf::IntRect({ 3 * 32, 7 * 32 }, { 32, 32 }));
    serverBanner4.setScale({ 8, 8 });
    serverBanner4.setPosition({ 1200, 275 });

    // Join Server Banner
    serverBanner5.setTextureRect(sf::IntRect({ 2 * 32, 7 * 32 }, { 32, 32 }));
    serverBanner5.setScale({ 8, 8 });
    serverBanner5.setPosition({ 500,550 });

    serverBanner6.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    serverBanner6.setScale({ 8, 8 });
    serverBanner6.setPosition({ 750,550 });

    serverBanner7.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    serverBanner7.setScale({ 8, 8 });
    serverBanner7.setPosition({ 950,550 });

    serverBanner8.setTextureRect(sf::IntRect({ 3 * 32, 7 * 32 }, { 32, 32 }));
    serverBanner8.setScale({ 8, 8 });
    serverBanner8.setPosition({ 1200,550 });

    //// Create Button
    serverBanner9.setTextureRect(sf::IntRect({ 2 * 32, 7 * 32 }, { 32, 32 }));
    serverBanner9.setScale({ 8, 8 });
    serverBanner9.setPosition({ 620, 800 });

    serverBanner10.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    serverBanner10.setScale({ 8, 8 });
    serverBanner10.setPosition({ 820, 800 });

    serverBanner11.setTextureRect(sf::IntRect({ 3 * 32, 7 * 32 }, { 32, 32 }));
    serverBanner11.setScale({ 8, 8 });
    serverBanner11.setPosition({ 1020, 800 });

    // Stats Button
    statsBanner1.setTextureRect(sf::IntRect({ 2 * 32, 7 * 32 }, { 32, 32 }));
    statsBanner1.setScale({ 6, 6 });
    statsBanner1.setPosition({ 1300, 875 });

    statsBanner2.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    statsBanner2.setScale({ 6, 6 });
    statsBanner2.setPosition({ 1420, 875 });

    statsBanner3.setTextureRect(sf::IntRect({ 3 * 27, 7 * 32 }, { 32, 32 }));
    statsBanner3.setScale({ 6, 6 });
    statsBanner3.setPosition({ 1550, 875 });

    statsBanner4.setTextureRect(sf::IntRect({ 3 * 32, 7 * 32 }, { 32, 32 }));
    statsBanner4.setScale({ 6, 6 });
    statsBanner4.setPosition({ 1720, 875 });
    
    sf::Texture heartTexture;
    if (!heartTexture.loadFromFile("Knight_1/heartSprite.png"))
    {
        std::cout << "error loading the knight  sprite" << std::endl;
    }

    sf::Sprite heartSprite(heartTexture);
    heartSprite.setScale({ 2.5,2.5 });

    sf::Texture trees;
    if (!trees.loadFromFile("tileset/tree.png"))
    {
        std::cout << "error loading the knight tree sprite" << std::endl;
    }

    sf::Texture rosebush;
    if (!rosebush.loadFromFile("tileset/bush.png"))
    {
        std::cout << "error loading the knight rosebush sprite" << std::endl;
    }

    sf::Texture bush;
    if (!bush.loadFromFile("tileset/grass.png"))
    {
        std::cout << "error loading the knight bush sprite" << std::endl;
    }

    sf::Texture enemy;
    if (!enemy.loadFromFile("wolf/walk.png"))
    {
        std::cout << "error loading the wolf walk sprite" << std::endl;
    }

    sf::Texture enemyAttack1;
    if (!enemyAttack1.loadFromFile("wolf/attack_1.png"))
    {
        std::cout << "error loading the wolf attack1 sprite" << std::endl;
    }

    sf::Texture enemyAttack2;
    if (!enemyAttack2.loadFromFile("wolf/attack_2.png"))
    {
        std::cout << "error loading the wolf attack2 sprite" << std::endl;
    }

    sf::Texture enemyAttack3;
    if (!enemyAttack3.loadFromFile("wolf/attack_3.png"))
    {
        std::cout << "error loading the wolf attack3 sprite" << std::endl;
    }

    sf::Texture dead;
    if (!dead.loadFromFile("wolf/dead.png"))
    {
        std::cout << "error loading the wolf dead sprite" << std::endl;
    }

    sf::Texture wolfHurt;
    if (!wolfHurt.loadFromFile("wolf/hurt.png"))
    {
        std::cout << "error loading the wolf hurt sprite" << std::endl;
    }

    sf::Texture SkeletonIdle;
    if (!SkeletonIdle.loadFromFile("Skeleton_Archer/Idle.png"))
    {
        std::cout << "error loading the skeleton idle sprite" << std::endl;
    }

    sf::Texture SkeletonShot1;
    if (!SkeletonShot1.loadFromFile("Skeleton_Archer/Shot_2.png"))
    {
        std::cout << "error loading the skeleton shot sprite" << std::endl;
    }

    sf::Texture arrow;
    if (!arrow.loadFromFile("Skeleton_Archer/Arrow.png"))
    {
        std::cout << "error loading the skeleton arrow sprite" << std::endl;
    }

    sf::Texture skeletonHurt;
    if (!skeletonHurt.loadFromFile("Skeleton_Archer/Hurt.png"))
    {
        std::cout << "error loading the skeleton hurt sprite" << std::endl;
    }

    sf::Texture skeletonDead;
    if (!skeletonDead.loadFromFile("Skeleton_Archer/Dead.png"))
    {
        std::cout << "error loading the skeleton dead sprite" << std::endl;
    }

    sf::Texture dragonIdle;
    if (!dragonIdle.loadFromFile("Dragon/Idle.png"))
    {
        std::cout << "error loading the dragon idle sprite" << std::endl;
    }
    
    sf::Texture dragonWalk;
    if (!dragonWalk.loadFromFile("Dragon/Walk.png"))
    {
        std::cout << "error loading the dragon walk sprite" << std::endl;
    }

    sf::Texture dragonAttack2;
    if (!dragonAttack2.loadFromFile("Dragon/Attack_2.png"))
    {
        std::cout << "error loading the dragon attack 2 sprite" << std::endl;
    }

    sf::Texture dragonRise;
    if (!dragonRise.loadFromFile("Dragon/Rise.png"))
    {
        std::cout << "error loading the dragon rise sprite" << std::endl;
    }

    sf::Texture dragonFlight;
    if (!dragonFlight.loadFromFile("Dragon/Flight.png"))
    {
        std::cout << "error loading the dragon flight sprite" << std::endl;
    }

    sf::Texture dragonSpecial;
    if (!dragonSpecial.loadFromFile("Dragon/Special.png"))
    {
        std::cout << "error loading the dragon special sprite" << std::endl;
    }

    sf::Texture dragonLanding;
    if (!dragonLanding.loadFromFile("Dragon/Landing.png"))
    {
        std::cout << "error loading the dragon landing sprite" << std::endl;
    }

    sf::Texture dragonHurt;
    if (!dragonHurt.loadFromFile("Dragon/Hurt.png"))
    {
        std::cout << "error loading the dragon hurt sprite" << std::endl;
    }

    sf::Texture dragonDead;
    if (!dragonDead.loadFromFile("Dragon/Dead.png"))
    {
        std::cout << "error loading the dragon dead sprite" << std::endl;
    }

    sf::Texture potionTexture;
    if (!potionTexture.loadFromFile("Knight_1/potionIcon.png"))
    {
        std::cout << "error loading the knight potion sprite" << std::endl;
    }

    AssetManager assets;
    sf::Music battleMusic;

    if (!assets.load())
    {
        cout << "TEXTURE(S) NOT LOADED" << endl;
        return -1;
    }

    sf::Sprite potionSprite(assets.getKnightTextures().potionTex);
    potionSprite.setScale({ 3,3 });

    sf::Sprite singlePlayerPotionSprite(assets.getKnightTextures().potionTex);
    singlePlayerPotionSprite.setScale({ 4,4 });

    sf::Sprite arenaSprite1(assets.getBackgroundTextures().arenaPictureTex);
    arenaSprite1.setPosition({ 1050,220 });
    arenaSprite1.scale({ 0.35,0.35 });

    sf::SoundBuffer menuSoundBuffer;
    sf::Sound menuSelectSound(menuSoundBuffer);

    sf::SoundBuffer winnerSoundBuffer;
    sf::Sound winnerSound(winnerSoundBuffer);

    sf::SoundBuffer menuClickBuffer;
    sf::Sound menuClickSound(menuClickBuffer);

    sf::SoundBuffer countdownBuffer;
    sf::Sound countdownSound(countdownBuffer);

    menuClickBuffer.loadFromFile("Sounds/menuClickSound.mp3");
    menuSoundBuffer.loadFromFile("Sounds/menuSelectSound.mp3");
    winnerSoundBuffer.loadFromFile("Sounds/hostVanquished.mp3");
    countdownBuffer.loadFromFile("Sounds/countdown.mp3");

    sf::RectangleShape backgroundShape(sf::Vector2f(GAME_WIDTH, GAME_HEIGHT));
    backgroundShape.setTexture(&assets.getBackgroundTextures().singlePlayerBackgroundTex);
    assets.getBackgroundTextures().singlePlayerBackgroundTex.setRepeated(true);

    sf::RectangleShape loadingBackgroundShape(sf::Vector2f(GAME_WIDTH, GAME_HEIGHT));
    loadingBackgroundShape.setTexture(&assets.getBackgroundTextures().loadingBackgroundTex);

    sf::RectangleShape multiplayerBackgroundShape(sf::Vector2f(GAME_WIDTH, GAME_HEIGHT));
    multiplayerBackgroundShape.setTexture(&assets.getBackgroundTextures().singlePlayerBackgroundTex);

    multiplayerBackgroundShape.setSize({ GAME_WIDTH, GAME_HEIGHT });
    multiplayerBackgroundShape.setPosition({ 0.f, 0.f });

    sf::RectangleShape titleBackgroundShape(sf::Vector2f(GAME_WIDTH, GAME_HEIGHT));
    titleBackgroundShape.setTexture(&assets.getBackgroundTextures().titleBackgroundTex);
    assets.getBackgroundTextures().titleBackgroundTex.setRepeated(true);

    sf::RectangleShape forestBackgroundShape(sf::Vector2f(GAME_WIDTH, GAME_HEIGHT));
    forestBackgroundShape.setTexture(&assets.getBackgroundTextures().howToPlayBackrgoundTex);

    sf::RectangleShape serverListBackgroundShape(sf::Vector2f(GAME_WIDTH, GAME_HEIGHT));
    serverListBackgroundShape.setTexture(&assets.getBackgroundTextures().serverListBackgroundTex);

    sf::RectangleShape createServerBackgroundShape(sf::Vector2f(GAME_WIDTH, GAME_HEIGHT));
    createServerBackgroundShape.setTexture(&assets.getBackgroundTextures().createServerBackgroundTex);
    assets.getBackgroundTextures().createServerBackgroundTex.setRepeated(true);
    

    sf::Sprite img1(assets.getBackgroundTextures().titleBackgroundTex);
    sf::Sprite img2(assets.getBackgroundTextures().titleBackgroundTex);
    float bgSpeed = 20.f; // pixels per second
    img1.setPosition({ 0.f,0.f });
    img2.setPosition({ (float)assets.getBackgroundTextures().titleBackgroundTex.getSize().x, 0.f });


   
    sf::RectangleShape player(sf::Vector2f(50, 50));
    player.setFillColor(sf::Color::Red);
    player.setOrigin({ 25, 25 });

    float worldWidth = 50000.f;

    sf::Sprite town(assets.getBackgroundTextures().singlePlayerBackgroundTex);
    town.setScale({ 0.9,0.9 });
    town.setPosition({ 0, -50 });

    sf::Color singlePlayerColor = sf::Color::Red;

    unique_ptr<Player> singlePlayerKnight = std::make_unique<Knight>(400.f, 1825.f, assets.getKnightTextures(), fancyFontText, singlePlayerColor, standardFont);
  
    //----------------------------------------------------------------------------------------------------------------------------------------------//
        // Unique pointer keeps my Knight object on the heap constantly while the only thing getting moved or copied is the pointer itself.
        // Places such as when I use try_emplace or anytime when unordered_map rehashes only moves the pointers of players instead of the actual object.
        // This fix made my sounds work again, because moving the actual object messed up my soundbufferes.

        // Other advantages of unique_ptr:
        // 1.) better preformance (only moving and copying 8 bytes of memory from the pointer)
        // 2.) Prevents memory leaks since it automatically frees memory
        // 3.) No accidental copying. Before i could do players[0] = players[1], thats bad.
        // 4.) Internal pointers (like soundBuffer) in the knight class are now stable memory and dont
        //     moving the object usual;y broke that memory
    //----------------------------------------------------------------------------------------------------------------------------------------------//
    CharacterType characterType;

    sf::Color samuraiColor = sf::Color::Blue;

    std::unique_ptr<Player> testSamurai = std::make_unique<Samurai>(350.f, 200.f, assets.getSamuraiTextures(), samuraiColor, standardFont);

    unordered_map<int, unique_ptr<Player>> players;
    HUDLayout HUDlayout;

    int myPlayerID = -1;
    std::vector<ServerEntryUI> serverEntries;

    Animation knightIdleDemo(assets.getKnightTextures().idleTex, 4, 1, 0.5f, 0, true, true, true);
    Animation knightWalkDemo(assets.getKnightTextures().walkTex, 8, 1, 0.1, 0, true, true,true);
    Animation knightJumpDemo(assets.getKnightTextures().jumpTex, 6, 1, 0.15, 0, true, true,true);
    Animation knightSprintDemo(assets.getKnightTextures().runTex, 7, 1, 0.1, 0, true, true,true);
    Animation knightAttackDemo(assets.getKnightTextures().attack2Tex, 4, 1, 0.2, 0, true, true,true);
    Animation knightGuardDemo(assets.getKnightTextures().shieldTex, 4, 1, 0.2, 0, true, true, true);
    Animation knightRollingDemo(assets.getKnightTextures().rollTex, 6, 1, 0.15, 0, true, true,true);
    Animation knightHangingDemo(assets.getKnightTextures().hangingTex, 6, 1, 0.15, 0, true, true,true);
    Animation knightDrinkingDemo(assets.getKnightTextures().elixirTex, 4, 1, 0.2, 0, true, true,true);
    Animation knightJumpAttackDemo(assets.getKnightTextures().jumpAttackTex, 5, 1, 0.15, 0, false, true,true);
    Animation knightSprintAttackDemo(assets.getKnightTextures().runAttackTex, 6, 1, 0.15, 0, false, true, true);

    knightWalkDemo.setPosition(200, 200);
    knightJumpDemo.setPosition(630, 200);
    knightSprintDemo.setPosition(1160, 200);
    knightAttackDemo.setPosition(1650, 200);
    knightGuardDemo.setPosition(150, 600);
    knightRollingDemo.setPosition(560, 600);
    knightHangingDemo.setPosition(1075, 600);
    knightDrinkingDemo.setPosition(1600, 600);
    knightSprintAttackDemo.setPosition(500, 150);
    knightJumpAttackDemo.setPosition(900, 150);

    knightJumpAttackDemo.setScale({ 4,4 });
    knightSprintAttackDemo.setScale({ 4,4 });

    Animation samuraiIdleDemo(assets.getSamuraiTextures().idleTex, 9, 1, 0.2f, 0, true, true);
    Animation samuraiWalkDemo(assets.getSamuraiTextures().walkTex, 8, 1, 0.1f, 0, true, true);
    Animation samuraiJumpDemo(assets.getSamuraiTextures().jumpTex, 9, 1, 0.1f, 0, true, true);
    Animation samuraiSprintDemo(assets.getSamuraiTextures().runTex, 8, 1, 0.1f, 0, true, true);
    Animation samuraiAttackDemo(assets.getSamuraiTextures().attack1Tex, 5, 1, 0.1f, 0, true, true);
    Animation samuraiSpecialDemo(assets.getSamuraiTextures().specialTex, 14, 1, 0.08f, 0, true, true);
    Animation samuraiDrinkDemo(assets.getSamuraiTextures().elixirTex, 4, 1, 0.2f, 0, true, true);
    Animation samuraiClimbDemo(assets.getSamuraiTextures().climbTex, 5, 1, 0.2f, 0, true, true);

    Animation samuraiDeadDemo(assets.getSamuraiTextures().deadTex, 5, 1, 0.1f, 0, true, true);

    samuraiWalkDemo.setPosition(200, 200);
    samuraiJumpDemo.setPosition(630, 200);
    samuraiSprintDemo.setPosition(1160, 200);
    samuraiAttackDemo.setPosition(1650, 200);
    samuraiSpecialDemo.setPosition(450, 600);
    samuraiClimbDemo.setPosition(1000, 600);
    samuraiDrinkDemo.setPosition(1500, 600);

    Animation samuraiSpecialDeflectionDemo(assets.getSamuraiTextures().specialTex,14, 1, 0.08f, 0, true, false);
    Animation knightGuardDeflectionDemo(assets.getKnightTextures().shieldTex, 4, 1, 0.2f, 0, true, false, true);
    Animation samuraiDeadDeflectionDemo(assets.getSamuraiTextures().deadTex, 5, 1, 0.1f, 0, true, false);

    knightGuardDeflectionDemo.setInitalDirectionToLeft();

   DeflectionDemo deflectionDemo(samuraiSpecialDeflectionDemo, knightIdleDemo, knightGuardDeflectionDemo, samuraiDeadDeflectionDemo, assets.getSamuraiTextures().arrowTex);

   Animation knightAttackParryDemo(assets.getKnightTextures().attack2Tex, 4, 1, 0.1f, 0, true, false,true);
   Animation knightGuardParryDemo(assets.getKnightTextures().shieldTex, 4, 1, 0.1f, 0, true, false, true);
   Animation knightStunnedParryDemo(assets.getKnightTextures().deadTex, 6, 1, 0.1f, 0, true, false,true);

   ParryDemo parryDemo(knightAttackParryDemo, knightIdleDemo, knightGuardParryDemo, knightStunnedParryDemo);

    Animation samuraiIdleDemoCharacterSelect(assets.getSamuraiTextures().idleTex, 9, 1, 0.2f, 0, false, true);
    samuraiIdleDemoCharacterSelect.getSprite().setScale({ 2,2 });

    Animation characterSelectKnightIdleDemo(assets.getKnightTextures().idleTex, 4, 1, 0.3f, 0, false, true,true);
    characterSelectKnightIdleDemo.getSprite().setScale({ 5.f,5.f });
    characterSelectKnightIdleDemo.getSprite().setPosition({ 500.f,90.f });
   
    Animation characterSelectSamuraiIdleDemo(assets.getSamuraiTextures().idleTex, 9, 1, 0.2f, 0, false, true);
    characterSelectSamuraiIdleDemo.getSprite().setScale({ 5.f,5.f });
    characterSelectSamuraiIdleDemo.getSprite().setPosition({900.f,95.f});

    sf::RectangleShape knightButton;
    knightButton.setSize({ 220.f, 400.f });
    knightButton.setPosition({ 550.f, 400.f });

    knightButton.setFillColor(sf::Color(255, 0, 0, 60));
    knightButton.setOutlineColor(sf::Color::Red);
    knightButton.setOutlineThickness(2.f);

    sf::RectangleShape samuraiButton;
    samuraiButton.setSize({ 220.f, 400.f });
    samuraiButton.setPosition({ 1100.f, 400.f });

    samuraiButton.setFillColor(sf::Color(255, 0, 0, 60));
    samuraiButton.setOutlineColor(sf::Color::Red);
    samuraiButton.setOutlineThickness(2.f);


    sf::Texture tileTextureDemo;
    if (!tileTextureDemo.loadFromFile("tileset/tilesetgrass.png")) {
        std::cout << "Failed to load tile texture\n";
    }

    sf::Texture ServerListTexture;
    if (!ServerListTexture.loadFromFile("GUI/freefantasy.png"))
    {
        std::cout << "error loading the serverlist texture" << std::endl;
    }

    sf::Texture ArenaPreviewTexture;
    if (!ArenaPreviewTexture.loadFromFile("GUI/freefantasy.png"))
    {
        std::cout << "error loading the serverlist texture" << std::endl;
    }

    sf::Texture WoodenGUITexture;
    if (!WoodenGUITexture.loadFromFile("GUI/freeversion.png"))
    {
        std::cout << "error loading the WoodenGUI" << std::endl;
    }

    sf::Texture ArenaBanner;
    if (!ArenaBanner.loadFromFile("GUI/banners.png"))
    {
        std::cout << "error loading the DarkWoodenGUI" << std::endl;
    }

    sf::Sprite tileSprite(tileTextureDemo);
    sf::Sprite ServerListSprite(ServerListTexture);
    sf::Sprite characterSelectBackground1(ServerListTexture);
    sf::Sprite characterSelectBackground2(ServerListTexture);
    sf::Sprite WoodenGUISprite(WoodenGUITexture);
    sf::Sprite WoodenGUISprite2(WoodenGUITexture);
    sf::Sprite ArenaPreviewSprite(ArenaPreviewTexture);
    sf::Sprite startGameBackground(ArenaPreviewTexture);
    //sf::Sprite deleteServerBackground(ArenaPreviewTexture);
    sf::Sprite characterSelectBackground(ArenaPreviewTexture);
    sf::Sprite ArenaBannerSprite(ArenaBanner);

    ArenaBannerSprite.setTextureRect(sf::IntRect({ 35 * 5, 0 }, { 55,32 }));
    ArenaBannerSprite.setScale({ 7,7 });
    ArenaBannerSprite.setPosition({ 1210,520 });

    ArenaPreviewSprite.setTextureRect(sf::IntRect({ 4 * 32, 0 }, { 64,32}));
    ArenaPreviewSprite.setScale({ 18,20 });
    ArenaPreviewSprite.setPosition({ 800,75 });

    startGameBackground.setTextureRect(sf::IntRect({ 4 * 32, 0 }, { 64,32 }));
    startGameBackground.setScale({ 9,5 });
    startGameBackground.setPosition({ 210,545 });

   /* deleteServerBackground.setTextureRect(sf::IntRect({ 4 * 32, 0 }, { 64,32 }));
    deleteServerBackground.setScale({ 10,6 });
    deleteServerBackground.setPosition({ 180,400 });*/

    characterSelectBackground.setTextureRect(sf::IntRect({ 4 * 32, 0 }, { 64,32 }));
    characterSelectBackground.setScale({ 11,5 });
    characterSelectBackground.setPosition({ 150,360 });

    tileSprite.setTextureRect(sf::IntRect({ 3 * 32, 3 * 32 }, { 32, 32 }));
    tileSprite.setScale({ 3,3 });

    ServerListSprite.setTextureRect(sf::IntRect({ 75, 40 }, { 85, 120 }));
    ServerListSprite.setScale({ 9,9 });
    ServerListSprite.setPosition({ 1100,-60});

    characterSelectBackground1.setTextureRect(sf::IntRect({ 75, 40 }, { 85, 120 }));
    characterSelectBackground1.setScale({ 6,6 });
    characterSelectBackground1.setPosition({ 400,150 });

    characterSelectBackground2.setTextureRect(sf::IntRect({ 75, 40 }, { 85, 120 }));
    characterSelectBackground2.setScale({ 6,6 });
    characterSelectBackground2.setPosition({ 950,150 });

    WoodenGUISprite.setTextureRect(sf::IntRect({ 0, 32 * 4 }, { 32 * 4, 32 + 20})); // a,b c,d   (c controlls length) (d controlls verticaltiy)
    WoodenGUISprite.setScale({ 15,15 });
    WoodenGUISprite.setPosition({ 0,50 });

    WoodenGUISprite2.setTextureRect(sf::IntRect({ 0, 32 * 4 }, { 32 * 4, 32 + 20 })); // a,b c,d   (c controlls length) (d controlls verticaltiy)
    WoodenGUISprite2.setScale({ 11,11 });
    WoodenGUISprite2.setPosition({ 250,500 });

    float t;
    float eased;
    sf::Color c;
    c = c.Red;
    sf::Color c2;
    c2 = c2.Yellow;
    sf::Color outline;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Arrow> arrows;

    sf::Texture tileSet;

    tileSet.loadFromFile("tileset/tilesetgrass.png");

    sf::Vector2u tileSize(32, 32);

    std::vector<Tile> tiles;
    std::vector<Tile> multiplayerTiles;
    std::vector<Tile> demoTiles;

    std::vector<Tile> decorationTiles;
    std::vector<Tile> multiplayerDecorationTiles;

    //loads in each tile based off the number. -1 is air
    std::vector<std::vector<int>> mapData = {

        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,1,1,1,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                   0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,1,1,1,1,1,2,-1,-1,-1,-1,-1,-1,0,1,1,1,1,35,11,11,11,34,1,1,1,1,1,1,1,1,1,1,                                   2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,25,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11},
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,21,25,11,11,34,1,1,1,1,1,1,1,1,1,1,35,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,34,                  1,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11},
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,2,-1,-1,-1,-1,-1,20,25,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,         12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11},
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,35,34,1,1,2,-1,-1,-1,-1,-1,20,21,21,25,24,21,21,21,21,21,21,21,25,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,         12,-1,-1,-1,-1,-1,-1,0,1,1,2,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11},
         { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,34,1,1,1,1,2,-1,-1,-1,-1,-1,-1,-1,20,22,-1,-1,-1,-1,-1,-1,-1,20,21,25,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,         12,-1,-1,-1,-1,-1,-1,10,11,11,12,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11},//
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,35,11,11,11,11,11,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,21,25,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,       12,-1,-1,-1,-1,-1,-1,10,11,1,1,1,2,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11},//
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,25,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,     12,-1,-1,-1,-1,-1,-1,10,11,11,11,11,12,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11},//
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,34,1,1,1,1,1,1,1,1,1,2,-1,-1,0,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,21,25,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,                   12,-1,-1,-1,-1,-1,-1,10,11,11,11,11,12,-1,-1,-1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,24,21,22,-1,-1,0,1,1,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,21,21,21,25,11,11,11,11,11,11,11,11,11,11,11,11,           12,-1,-1,-1,-1,-1,-1,10,11,11,11,11,12,-1,-1,-1,20,25,11,11,11,11,11,11,11,24,21,21,21,21,21,21,21,21,21,21,21,22},
         { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,12,-1,-1,-1,-1,20,21,21,21,21,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,25,11,11,11,11,11,11,11,11,11,11,    24,22,-1,-1,-1,-1,-1,-1,10,11,11,34,1,1,   2,-1,-1,-1,10,11,11,11,11,24,21,21,22},//
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,1,35,24,21,21,21,21,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,21,25,11,11,11,11,11,11,11,11,             12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,12,-1,-1,-1,-1,10,11,24,21,21,22},
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,24,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,25,11,11,11,11,11,11,11,        12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,12,-1,-1,-1,-1,10,11,12},
           { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,34,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,25,24,22,-1,-1,-1,-1,30,31,32,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,     12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,12,-1,-1,-1,0,1,1,12},
           { -1,-1,-1,0,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,24,21,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,1,1,2,-1,-1,-1,-1,20,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,          12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,12,-1,-1,-1,20,21,21,22},
           { -1,0,1,35,34,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,35,11,11,11,34,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,1,1,11,11,11,11,11,11,               12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
           { 1,35,24,21,21,21,25,34,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,35,11,11,11,11,11,11,34,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,21,25,11,11,11,11,11,11,11,11,      12,-1,-1,-1,-1,-1,-1,-1,10,11,34,1,1,1,    2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    0,1,1,1,2},
           { 24,21,22,-1,-1,-1,20,21,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,1,1,1,1,1,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,2,-1,-1,0,1,1,1,1,1,1,1,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,                             12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    10,11,11,11,12},
           { 12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,1,11,11,11,11,11,11,11,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,12,-1,-1,20,25,11,11,11,11,11,11,11,11,34,2,-1,-1,-1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,11,11,11,11,11,11,                      12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    10,11,11,11,12},
           { 1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,1,1,1,1,11,11,11,11,11,11,11,11,12,-1,-1,-1,-1,-1,-1 -1 -1 -1 -1 -1 -1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,-1,-1,-1,0,1,35,12,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,34,1,1,1,35,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,34,1,1,1,1,2,-1,-1,-1,-1,-1,-1,-1,         0,35,11,11,11,12,-1,-1,-1,-1,-1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
           { 11,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,-1,-1,-1,0,1,1,1,1,2,-1,-1,-1,10,34,1,2,-1,-1,-1,20,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,35,11,11,11,11,11,11,11,11,11,11,11,                                                       12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,12,-1,-1,-1,-1,-1,-1,-1,   10,11,11,11,11,12,-1,-1,-1,-1,-1,20,25,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,24,22},
           { 11,34,1,1,1,1,1,1,1,1,1,1,1,35,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,2,-1,-1,-1,10,1,1,1,1,1,2,-1,-1,10,11,24,22,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,                       12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,34,1,1,1,1,1,1,1,1,1,1,1,1,1,1,35,11,12,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12},
           {11,11,11,11,11,11,11,11,11,11,11,11,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,-1,-1,10,11,11,12,-1,-1,-1,10,11,11,11,11,11,12,-1,-1,10,11,12,-1,-1,-1,-1,0,1,1,1,1,1,1,35,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,                                          12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12},
           {11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,1,1,1,1,2,-1,10,11,11,12,-1,-1,-1,0,1,1,1,1,35,12,-1,-1,0,1,2,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,                12,-1,-1,-1,-1,-1,-1,-1,10,11,11,34,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,12,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12},
           {21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,11,12,-1-1,10,11,11,12,-1,-1,-1,10,11,11,11,11,11,12,-1,-1,10,11,12,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11, 12,-1,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,-1,-1,-1,-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12}
    };

    std::vector<std::vector<int>> decorationData =
    {
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,3,4,1,2,3,5,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,2,3,1,5,3,4,2,1,3,2,3,2,2,3},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,3,2,2,3,3,3,2,-1,-1,-1,-1,-1,-1,-1,-1,3,2,3,3,-1,-1,-1,-1,-1,1,2,4,3,3,0,2,3,3},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,3,3,3,2,2,3,2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,3,1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,4,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,4,3},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0},
        {-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,4,3,5,2,3,3,0,2,3,-1,1,2,3,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,3,1,4,3,2,3,5,3,4,2,2},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,3,3,5,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,3,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},

        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,3,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0},
        {-1,-1,-1,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,3,4,2,3},
        {-1,4,-1,-1,-1,1,2,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,4,3},
        {3,-1,-1,-1,-1,-1,-1,-1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,5,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,5,2},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,3,2,2,3,1,2,3,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,5,3,-1,-1,-1,2,3,2,0,2,3,3,2,3,2},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,2,3,1,3,3,2,2,5,3,2,3},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,4,3,1,2,2,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,3,0,-1,-1,-1,-1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,4,3,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,5,2,3,3,2,1,3,3,4,2,3,2,2,5,3,3,4,1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,2,3,3,2,2,2,3,2,2,2,3,1,3,3,2,4,3,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,2,3,-1,-1,-1,4,1,2,3,2,-1,-1,-1,-1,-1,-1,0,3,-1,-1,-1,-1,2,3,3,0,2,1,2,2,3,3,3,2,3,3,3,2,2,3,3,2,2,2},
        {-1,-1,3,5,-1,2,3,2,2,2,0,-1,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,4,3,2,2,-1,-1,-1,-1,-1,2,2,3,3,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,4,3,2,3,5},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,3,2,3,-1,2,1,3,0,2,-1,3,4,3,2,3,1,2,3,3,2,3,0,2,2,5,3,2,3,2,4},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,3,3,2,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,2,4,3,2,3,3,2,2,3,2,3,2,2,2,3,2,1}


    };

    std::vector<std::vector<int>> multiplayerArenaData = {

           {-1,-1,-1},
           {-1,-1,-1},
           {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
           {-1,-1,-1,-1,-1,-1,-1,30,31,31,32,-1,-1,-1,0,1,1,1,2},
           {-1,0,1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,1,1,1,12},
           {-1,0,1,1,2,-1,-1,-1,-1,-1,-1,-1,-1,20,21,21,25,11,12},
           {-1,10,24,21,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,25,12},
           {-1,10,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,12},
           {-1,10,12,-1,-1,-1,-1,0,1,1,1,2,-1,-1,-1,-1,-1,10,12},
           {-1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
           {-1,20,25,11,11,11,11,11,11,11,11,11,11,11,11,11,11,24,22},
           {-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12},
           {-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,11,12},
           {-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,12},
           {-1,-1,-1,10,11,11,11,11,11,11,11,11,11,11,11,11,12}
    };

    std::vector<std::vector<int>> multiplayerDecorationData = {

           {-1,-1,-1},
           {-1,-1,-1},
           {-1,-1,-1},
           {-1,-1,-1,-1,-1,-1,-1,2,3,3,2,-1,-1,-1,4,3,5,2},
           {-1,3,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,3,2,2},
           {-1,-1,3,2,3},
           {-1,-1,-1},
           {-1,-1,-1},
           {-1,-1,-1,-1,-1,-1,-1,5,2,1},
           {-1,5,2,4,1,2,2,3,3,2,3,3,2,0,2,3,1}
    };

    std::vector<std::vector<int>> demoData = {
        {-1},
        {-1},
        {-1},
        {-1},
        {-1},
        {-1,-1,-1,-1,0,1,1,1,1,1,1,1,1,1,1,2},
        {-1,-1,-1,-1,20,21,21,21,21,21,21,21,21,21,21,22}
    };

    sf::Vector2u treeSize(96, 96);

    Level singlePlayerLevel;
    Level multiplayerArena;
    Level demoPlatform;


    singlePlayerLevel.loadTileMap(tileSet, mapData, tileSize,tiles);
    singlePlayerLevel.loadDecorationMap(trees, bush, rosebush, decorationData, treeSize, tileSize, decorationTiles);

    multiplayerArena.loadTileMap(tileSet, multiplayerArenaData, tileSize, multiplayerTiles);
    multiplayerArena.loadDecorationMap(trees, bush, rosebush, multiplayerDecorationData, treeSize, tileSize, multiplayerDecorationTiles);

    demoPlatform.loadTileMap(tileSet, demoData, tileSize, demoTiles);

    float knightOffset = 300.f;
    float lerpSpeed = 10.f; 
   // sf::View defaultView = window.getDefaultView();
    bool pendingReset = false;

    singlePlayerKnight->deathText.getText().setFillColor(sf::Color(255, 255, 255, 0)); // start invisible
    float alpha = 0.f;
    float fadeSpeed = 150.f;

    while (window.isOpen())
    {
        sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos);
        float deltaTime = clock.restart().asSeconds();
        window.setMouseCursor(CursorArrow);
        window.clear();

        

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                cout << "RESIZED" << endl;
                cout << "--------------------------" << endl;
                cout << "Desktop: "
                    << desktopMode.size.x << " "
                    << desktopMode.size.y << endl;

                cout << "Window: "
                    << window.getSize().x << " "
                    << window.getSize().y << endl;

                cout << "Menu view: "
                    << menuView.getSize().x << " "
                    << menuView.getSize().y << endl;


                gameView.setViewport(
                    getLetterboxView(
                        gameView,
                        resized->size.x,
                        resized->size.y
                    )
                );

                battleView.setViewport(
                    getLetterboxView(
                        battleView,
                        resized->size.x,
                        resized->size.y
                    )
                );


                menuView.setViewport(
                    getLetterboxView(
                        menuView,
                        resized->size.x,
                        resized->size.y
                    )
                );
            }

            bool clickConsumed = false;

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                const auto& mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();

                if (mouseEvent->button == sf::Mouse::Button::Left)
                {
                    if (maxPlayersBox.getGlobalBounds().contains(mousePos))
                    {
                        maxPlayers++;

                        if (maxPlayers > 4)
                            maxPlayers = 2;
                    }

                    if (serverNameBox.box.getGlobalBounds().contains(mousePos) && gameState == GameState::createServer)
                        serverNameBox.selected = true;
                    else
                        serverNameBox.selected = false;

                    if (passwordBox.box.getGlobalBounds().contains(mousePos) && gameState == GameState::createServer)
                        passwordBox.selected = true;
                    else
                        passwordBox.selected = false;

                    if (nametag.box.getGlobalBounds().contains(mousePos) && gameState == GameState::enterUsername)
                        nametag.selected = true;
                    else
                        nametag.selected = false;

                    if (passwordAttemptBox.box.getGlobalBounds().contains(mousePos) && gameState == GameState::enterPassword)
                        passwordAttemptBox.selected = true;
                    else
                        passwordAttemptBox.selected = false;

                    switch (gameState)
                    {
                    case GameState::mainMenu:
                    {
                        if (!clickConsumed)
                        {
                            if (play.getBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                pendingReset = true;
                                gameState = GameState::level1Pause;
                                clickConsumed = true;
                            }
                            else if (exit.getBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                window.close();
                                clickConsumed = true;
                            }
                            else if (howToPlay.getBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                gameState = GameState::howtoPlay;
                                clickConsumed = true;
                            }
                            else if (BattleMode.getBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                gameState = GameState::enterUsername;
                                clickConsumed = true;
                            }
                        }
                        break;
                    }
                    case GameState::howtoPlay:
                    {
                        GameState changePage = pressedLobbyCharacterSelectButton ? GameState::characterSelect : GameState::mainMenu;

                        if (!clickConsumed && Menu.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = changePage;
                            clickConsumed = true;

                            if (changePage != GameState::characterSelect)
                                 pressedLobbyCharacterSelectButton = false;
                        }
                        else if (!clickConsumed && Next.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = GameState::howToPlay2;
                            clickConsumed = true;
                        }
                        break;
                    }
                    case GameState::howToPlay2:
                    {
                        GameState changePage = pressedLobbyCharacterSelectButton ? GameState::characterSelect : GameState::mainMenu;

                        if (!clickConsumed && Menu.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = changePage;
                            clickConsumed = true;
                           
                            if (changePage != GameState::characterSelect)
                                pressedLobbyCharacterSelectButton = false;
                        }
                        else if (!clickConsumed && Next.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = GameState::howToPlay3;
                            clickConsumed = true;
                        }
                        else if (!clickConsumed && previousHowToPlayPage.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = GameState::howtoPlay;
                            clickConsumed = true;
                        }
                        break;
                    }
                    case GameState::howToPlay3:
                    {
                        GameState changePage = pressedLobbyCharacterSelectButton ? GameState::characterSelect : GameState::mainMenu;

                        if (!clickConsumed && Menu.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = changePage;
                            clickConsumed = true;
                            
                            if (changePage != GameState::characterSelect)
                                pressedLobbyCharacterSelectButton = false;
                        }
                        else if (!clickConsumed && Next.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            deflectionDemo.reset();
                            gameState = GameState::specialInteractions;
                            clickConsumed = true;
                        }
                        else if (!clickConsumed && previousHowToPlayPage.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = GameState::howToPlay2;
                            clickConsumed = true;
                        }
                        break;
                    }
                    case GameState::specialInteractions:
                    {
                        GameState changePage = pressedLobbyCharacterSelectButton ? GameState::characterSelect : GameState::mainMenu;

                        if (!clickConsumed && Menu.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = changePage;
                            clickConsumed = true;
                            
                            if (changePage != GameState::characterSelect)
                                pressedLobbyCharacterSelectButton = false;
                        }
                        else if (!clickConsumed && Next.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();                           
                            parryDemo.reset();
                            gameState = GameState::specialInteractions2;
                            clickConsumed = true;
                        }
                        else if (!clickConsumed && previousHowToPlayPage.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = GameState::howToPlay3;
                            clickConsumed = true;
                        }
                        break;
                    }
                    case GameState::specialInteractions2:
                    {
                        GameState changePage = pressedLobbyCharacterSelectButton ? GameState::characterSelect : GameState::mainMenu;

                        if (!clickConsumed && Menu.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = changePage;
                            clickConsumed = true;

                            if (changePage != GameState::characterSelect)
                                pressedLobbyCharacterSelectButton = false;
                        }                      
                        else if (!clickConsumed && previousHowToPlayPage.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            deflectionDemo.reset();
                            gameState = GameState::specialInteractions;
                            clickConsumed = true;
                        }
                        break;
                    }
                    case GameState::enterUsername:
                    {
                        if (!clickConsumed)
                        {
                            if (Continue.getBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                client.connectToLobby(Ip, lobbyPort); // Connect to lobby first to be able to use lobbysocket for requestServerList and other things
                                client.setPlayerName(nametag.str); // Set your player name to the one from the username textbox
                                gameState = GameState::characterSelect;
                                clickConsumed = true;
                            }
                        }

                        break;
                    }
                    case GameState::characterSelect:
                    {
                        if (!clickConsumed)
                        {
                            GameState changePage = pressedLobbyCharacterSelectButton ? GameState::preGameLobby : GameState::serverPage;

                            if (Back.getBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                gameState = pressedLobbyCharacterSelectButton ? GameState::preGameLobby : GameState::mainMenu;
                                clickConsumed = true;
                                pressedLobbyCharacterSelectButton = false;
                            }
                            else if (knightButton.getGlobalBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                characterType = CharacterType::Knight;
                                gameState = changePage;
                                clickConsumed = true;
                                characterSelected = true;
                            }
                            else if (samuraiButton.getGlobalBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                characterType = CharacterType::Samurai;
                                gameState = changePage;
                                clickConsumed = true;
                                characterSelected = true;
                            }
                            else if (howToPlayCharacterSelect.getBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                gameState = changePage;
                                clickConsumed = true;
                                gameState = GameState::howtoPlay;
                            }
                        }

                        break;
                    }
                    case GameState::serverPage:
                    {
                        if (!clickConsumed)
                        {
                            if (CreateServer.getBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                gameState = GameState::createServer;
                                clickConsumed = true;
                            }
                            else if (JoinServer.getBounds().contains(mousePos))
                            {
                                menuClickSound.play();

                                client.requestServerList();

                                rebuildServerUI = true;
                                gameState = GameState::joinServer;
                                clickConsumed = true;
                            }
                            else if (Back.getBounds().contains(mousePos))
                            {
                                menuClickSound.play();
                                gameState = GameState::mainMenu;
                                clickConsumed = true;
                            }
                        }
                        break;
                    }
                    case GameState::joinServer:
                    {                     
                        for (auto& entry : serverEntries)
                        {
                            const auto& server = client.getServerList()[entry.serverIndex];

                            // Only join server if currentPlayers is less than the maximum amount of players
                            bool joinSpecificServer = entry.text.getGlobalBounds().contains(mousePos) && 
                                server.currentPlayers < server.maxPlayers;                          

                            if (joinSpecificServer)
                            {
                                if (server.passwordProtected)
                                {
                                    gameState = GameState::enterPassword;
                                    passwordAttemptBox.str.clear();
                                    passwordAttemptBox.text.setString("");
                                    showErrorMessage = false;
                                    errorMessageCount = 0;
                                    entryServerIndex = entry.serverIndex;
                                    menuClickSound.play();
                                }
                                else
                                {
                                    menuClickSound.play();                                  

                                    client.maxLobbySize = server.maxPlayers;

                                    client.connectToGameServer(Ip, 54000); // Connect to game server when lobby is clicked on
                                    client.rebuildLobbyPlayers();
                                    gameState = GameState::preGameLobby;
                                    break;
                                }
                            }                                               
                        }

                        if (Back.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = GameState::serverPage;
                        }

                        if (Refresh.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            rebuildServerUI = true;
                            client.requestServerList();
                        }

                        break;
                    }
                    case GameState::enterPassword:
                    {
                        if (!clickConsumed)
                        {
                            if (Continue.getBounds().contains(mousePos))
                            {
                                cout << "----------------------------------------" << endl;
                                cout << "CONTINUE PRESSED" << endl;
                                cout << "ENTRY INDEX: " << entryServerIndex << endl;

                                const auto& server = client.getServerList()[entryServerIndex];

                                cout << "SERVER NAME: " << server.name << endl;
                                cout << "SERVER PASSWORD: [" << server.password << "]" << endl;
                                cout << "ATTEMPTED PASSWORD: [" << passwordAttemptBox.str << "]" << endl;

                                if (passwordAttemptBox.str == server.password)
                                {
                                    cout << "PASSWORD CORRECT" << endl;

                                    menuClickSound.play();

                                    client.maxLobbySize = server.maxPlayers;                                   

                                    client.connectToGameServer(Ip, 54000); // Connect to game server when lobby is clicked on
                                    client.rebuildLobbyPlayers();

                                    cout << "CONNECTING TO "
                                        << server.ip.toString()
                                        << ":" << server.port << endl;
                                    cout << "----------------------------------------" << endl;

                                    gameState = GameState::preGameLobby;                                   
                                    showErrorMessage = false;
                                    errorMessageCount = 0;
                                }
                                else
                                {
                                    menuClickSound.play();
                                    showErrorMessage = true;  
                                    errorMessageCount++;
                                }

                                clickConsumed = true;
                            }
                                                      
                            if (Back.getBounds().contains(mousePos))
                            {
                                menuClickSound.play(); 
                                gameState = GameState::joinServer;
                                clickConsumed = true;
                            }
                        }

                        break;
                    }
                    case GameState::createServer:
                    {
                        if (Back.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();
                            gameState = GameState::serverPage;
                            clickConsumed = true;
                        }

                        if (!serverLaunched && Create.getBounds().contains(mousePos) && serverNameBox.letterCount != 0)
                        {
                            menuClickSound.play();
                            serverLaunched = true;

                            ServerInfo info = extractServerInfoFromUI(serverNameBox, passwordBox, maxPlayers, gamePort, Ip);
                            client.createServer(info);

                            //std::thread([Ip, lobbyPort, gamePort, info, &serverReady]() // Allows the computer to run the network loop indenpendently of the main loop
                            //    {
                            //        Server server(gamePort);
                            //        server.connectToLobby(Ip, lobbyPort);                                 
                            //        server.createServer(info);
                            //        serverReady = true;
                            //        server.runRelayServer();
                            //    }).detach(); // deatch allows the thread to execute without depending on any other loop or thread. It continues
                            //// working in the background forever      

                           /* client.connectToGameServer(Ip, 54000);

                            client.maxLobbySize = 2;*/

                            gameState = GameState::preGameLobby;
                        }

                        break;
                    }
                    case GameState::preGameLobby:
                    {

                        if (StartGame.getBounds().contains(mousePos) && client.getLobbyPlayers().size() == client.maxLobbySize)
                        {
                            menuClickSound.play();

                            sf::Packet packet;

                            packet << static_cast<int>(PacketType::StartLoading);

                            client.gameSocket.send(packet);

                        }

                        if (Leave.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();

                            client.leaveLobby();
                            client.rebuildLobbyPlayers();

                            gameState = GameState::serverPage;
                        }

                        if (characterSelectLobby.getBounds().contains(mousePos))
                        {
                            menuClickSound.play();

                            pressedLobbyCharacterSelectButton = true;

                            gameState = GameState::characterSelect;
                        }                      
                        break;
                    }
                    default:
                        break;
                    }


                }

                
            }

            if (event->is<sf::Event::TextEntered>())
            {
                if (auto* textEvent = event->getIf<sf::Event::TextEntered>())
                {
                    unsigned int unicode = textEvent->unicode; // safe now

                    if (serverNameBox.selected)
                    {
                        if (unicode == 8) // Backspace
                            serverNameBox.backspace();
                        else
                            serverNameBox.addCharacter(unicode,16);
                    }
                    else if (passwordBox.selected)
                    {
                        if (unicode == 8)
                            passwordBox.backspace();
                        else
                            passwordBox.addCharacter(unicode,16);
                    }
                    else if (nametag.selected)
                    {
                        if (unicode == 8)
                            nametag.backspace();
                        else
                            nametag.addCharacter(unicode,8);
                    }
                    else if (passwordAttemptBox.selected)
                    {
                        if (unicode == 8)
                            passwordAttemptBox.backspace();
                        else
                            passwordAttemptBox.addCharacter(unicode, 16);
                    }
                }
            }

        }

        if (pendingReset) 
        {
            enemies.clear();
            arrows.clear();
         
            singlePlayerKnight->resetPlayer({ 400.f, 1825.f }, true);

            enemies.emplace_back(std::make_unique<Skeleton>(SkeletonIdle, SkeletonShot1, arrow, skeletonHurt, skeletonDead, 5560.f, 1825.f, 2));
            enemies.emplace_back(std::make_unique<wolf>(enemy, enemyAttack1, enemyAttack2, enemyAttack3, dead, wolfHurt, 3500.f, 2115.f));
            enemies.emplace_back(std::make_unique<wolf>(enemy, enemyAttack1, enemyAttack2, enemyAttack3, dead, wolfHurt, 2500.f, 1635.f));
            enemies.emplace_back(std::make_unique<Skeleton>(SkeletonIdle, SkeletonShot1, arrow, skeletonHurt, skeletonDead, 7892.f, 1060.f, 1));
            enemies.emplace_back(std::make_unique<wolf>(enemy, enemyAttack1, enemyAttack2, enemyAttack3, dead, wolfHurt, 7300.f, 1346.f));
            enemies.emplace_back(std::make_unique<wolf>(enemy, enemyAttack1, enemyAttack2, enemyAttack3, dead, wolfHurt, 7200.f, 100.f));
            enemies.emplace_back(std::make_unique<wolf>(enemy, enemyAttack1, enemyAttack2, enemyAttack3, dead, wolfHurt,  9000.f, 100.f));
            enemies.emplace_back(std::make_unique<Dragon>(dragonIdle, dragonWalk, dragonAttack2, dragonRise, dragonFlight, dragonSpecial, dragonLanding, dragonHurt, dragonDead, fancyFontText, 14000.f, 100.f,singlePlayerColor));
           
            pendingReset = false;

        }

        switch (gameState)
        {
            case GameState::mainMenu:
            {         
                window.setView(menuView);

                if (playSong)
                {
                     menuMusic.play();
                    singlePlayerKnight->music.stop();
                    for (auto& enemy : enemies) {
                        Dragon* dragon = dynamic_cast<Dragon*>(enemy.get());
                        if (dragon)
                        {
                            dragon->bossMusic.stop();
                            dragon->dragonFlapSound.stop();
                            dragon->dragonFlameSound.stop();
                            dragon->dragonFireSpecialSound.stop();
                            dragon->bossMusic.setVolume(100.f);
                        }
                    }

                    levelDelay.reset();
                    deathDelay.reset();
                    backToMenuDelay.reset();
                    endDelayToMenu.reset();
                    endDelay.reset();
                    alpha = 0.f;
                    playSong3 = true;
                    playSong2 = true;
                    playSong = false;
                    playSong4 = true;
                    window.setMouseCursorVisible(true);
                }
                  //MOVING BACKGROUND
                  ///////////////////////////////////////////////////////////////////////////////////
                img1.move({ -bgSpeed * deltaTime, 0.f });
                img2.move({ -bgSpeed * deltaTime, 0.f });

                //  Reset when one sprite goes off-screen
                if (img1.getPosition().x + assets.getBackgroundTextures().titleBackgroundTex.getSize().x < 0)
                    img1.setPosition({ img2.getPosition().x + assets.getBackgroundTextures().titleBackgroundTex.getSize().x, 0.f });

                if (img2.getPosition().x + assets.getBackgroundTextures().titleBackgroundTex.getSize().x < 0)
                    img2.setPosition({ img1.getPosition().x + assets.getBackgroundTextures().titleBackgroundTex.getSize().x, 0.f });
                /////////////////////////////////////////////////////////////////////////////////////////////////////////////

                window.draw(img1);
                window.draw(img2);
                window.draw(banner);
                window.draw(banner2);
                window.draw(banner3);
                window.draw(banner4);
                window.draw(banner5);
                window.draw(banner6);
                window.draw(banner7);

                play.draw(window);
                exit.draw(window);
                title.draw(window);
                howToPlay.draw(window);
                BattleMode.draw(window);

                if (play.getBounds().contains(mousePos))
                {
                    play.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound)
                    {
                        menuSelectSound.play();
                        playMenuSound = true;
                    }
                }
                else
                {
                    play.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound = false;
                }

                if (exit.getBounds().contains(mousePos))
                {
                    exit.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound2)
                    {
                        menuSelectSound.play();
                        playMenuSound2 = true;
                    }
                }
                else
                {
                    exit.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound2 = false;
                }

                if (howToPlay.getBounds().contains(mousePos))
                {
                    howToPlay.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound3)
                    {
                        menuSelectSound.play();
                        playMenuSound3 = true;
                    }
                }
                else
                {
                    howToPlay.getText().setFillColor(sf::Color::White); // Normal

                    playMenuSound3 = false;
                }

                if (BattleMode.getBounds().contains(mousePos))
                {
                    BattleMode.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound5)
                    {
                        menuSelectSound.play();
                        playMenuSound5 = true;
                    }
                }
                else
                {
                    BattleMode.getText().setFillColor(sf::Color::White); // Normal

                    playMenuSound5 = false;
                }                

                break;
            }
            case GameState::playing:
            {            
                if (playSong2)
                {
                    singlePlayerKnight->music.setVolume(100.f);
                    singlePlayerKnight->music.play();
                    menuMusic.stop();
                    playSong = true;
                    playSong2 = false;
                    window.setMouseCursorVisible(false);

                }

                window.draw(backgroundShape);

                singlePlayerLevel.draw(window,tiles,decorationTiles);

                singlePlayerKnight->draw(window);         

                for (auto& enemy : enemies)
                {
                    enemy->draw(window);
                }

                for (const auto& arrow2 : arrows)
                {
                    window.draw(arrow2.sprite);

                }

                window.draw(inventory);
                window.draw(singlePlayerPotionSprite);

                singlePlayerKnight->potionNumText.draw(window);

                singlePlayerKnight->updateSinglePlayer(deltaTime, tiles, enemies, arrows, standardFont);
           

                if (singlePlayerKnight->isDeadBool())
                {
                    deathDelay.start();

                    if (deathDelay.getElapsedTime() > deathTime)
                    {
                        gameState = GameState::paused;
                    }


                }
          
                for (auto it = arrows.begin(); it != arrows.end(); )
                {
                    it->update(deltaTime, arrows, tiles);
                
                    for (const auto& tile : tiles)
                    {
                        if (tile.isCollidableTile() && it->getBounds().findIntersection(tile.getBounds()))
                        {
                            it->velocity = { 0.f, 0.f };
                            it->setArrowOnTile(true); 
                        }
                    }

                    if (it->isExpired())
                        it = arrows.erase(it);
                    else
                        ++it;
                }

                for (auto& enemy : enemies)
                {
                    Skeleton* skeleton = dynamic_cast<Skeleton*>(enemy.get());
                    if (skeleton)
                    {
                        if (!enemy->isDead())
                            skeleton->shootArrow(arrows, tiles, deltaTime); // Shoots arrow if needed
                    }


                    enemy->update(deltaTime, tiles, enemies);

                }


                sf::Vector2f knightPosition = singlePlayerKnight->getPlayerBox().getPosition();

                // Get current camera center
                sf::Vector2f center = gameView.getCenter();
                sf::Vector2f target = center; // Start with current center

                if (knightPosition.x > center.x + margin) {
                    target.x = knightPosition.x - knightOffset;
                }
                // Knight moves left beyond margin
                else if (knightPosition.x < center.x - margin) {
                    target.x = knightPosition.x + knightOffset;
                }

          
                float halfWindowWidth = windowWidth / 2.f;
                if (target.x < halfWindowWidth)
                    target.x = halfWindowWidth;
                if (target.x > worldWidth - halfWindowWidth)
                    target.x = worldWidth - halfWindowWidth;

                // Smoothly move the current center toward the target
                center.x += (target.x - center.x) * lerpSpeed * deltaTime;


                float marginY = 100.f; // vertical margin buffer

                if (singlePlayerKnight->getPlayerBox().getGlobalBounds().position.y < 2000)
                {

                    if (knightPosition.y > center.y + marginY) {
                        center.y = knightPosition.y - marginY;
                    }
                    else if (knightPosition.y < center.y - marginY) {
                        center.y = knightPosition.y + marginY;
                    }

                }

                center.x = std::floor(center.x + 0.5f);
                center.y = std::floor(center.y + 0.5f);

                gameView.setCenter(center);
                window.setView(gameView);

                // Parallax background offset 
                int textureWidth = assets.getBackgroundTextures().singlePlayerBackgroundTex.getSize().x;
                float parallaxFactor = 0.1f;

                int offsetX = static_cast<int>(center.x * parallaxFactor) % textureWidth;
                backgroundShape.setTextureRect(sf::IntRect({ offsetX, 0 },
                    { static_cast<int>(GAME_WIDTH),static_cast<int>(GAME_HEIGHT) }));

                backgroundShape.setPosition({
                    center.x - GAME_WIDTH / 2.f,
                    center.y - GAME_HEIGHT / 2.f
                    });

                for (auto& enemy : enemies) {
                    Dragon* dragon = dynamic_cast<Dragon*>(enemy.get());
                    if (dragon)
                    {
                        dragon->setBackgroundShape(&backgroundShape);

                        if (dragon->isDead())
                        {
                            endDelay.start();

                            dragon->bossMusic.setVolume(30.f);

                            if (endDelay.getElapsedTime() > endTime)
                            {
                                gameState = GameState::end;
                            }

                       
                        }
                    }

                }
           
                singlePlayerKnight->getHealthBar().update(deltaTime, singlePlayerKnight->getHealth(), 100.f, backgroundShape.getPosition().x + 50, backgroundShape.getPosition().y + 30);                
                heartSprite.setPosition({ backgroundShape.getPosition().x + 15, backgroundShape.getPosition().y + 8 });
                singlePlayerPotionSprite.setPosition({backgroundShape.getPosition().x + 15, backgroundShape.getPosition().y + 900});
                singlePlayerKnight->potionNumText.setPosition( backgroundShape.getPosition().x + 100, backgroundShape.getPosition().y + 975 );

                inventory.setPosition({ backgroundShape.getPosition().x + 20, backgroundShape.getPosition().y + 850 });

                singlePlayerKnight->getHealthBar().draw(window);
                window.draw(heartSprite);
           
                break;
            }
            case GameState::paused:
            {
                if (playSong3)
                {
                    singlePlayerKnight->deathMusic.play();
                    playSong3 = false;
                }
                singlePlayerKnight->music.setVolume(30.f);


                window.draw(backgroundShape);
                singlePlayerLevel.draw(window, tiles, decorationTiles);

                singlePlayerKnight->draw(window);

                for (auto& enemy : enemies)
                {
                    enemy->draw(window);
                }

                for (const auto& arrow2 : arrows)
                {
                    window.draw(arrow2.sprite);

                }


                outline = singlePlayerKnight->deathText.getText().getOutlineColor();
                outline.a = static_cast<unsigned char>(alpha);
                singlePlayerKnight->deathText.getText().setOutlineColor(outline);

                alpha += fadeSpeed * deltaTime;
                if (alpha > 255.f) alpha = 255.f;
                t = alpha / 255.f;
                eased = t * t;
                c.a = static_cast<unsigned char>(255.f * eased);
                singlePlayerKnight->deathText.getText().setFillColor(c);

                singlePlayerKnight->deathText.setPosition(backgroundShape.getPosition().x + 500, backgroundShape.getPosition().y + 300);
                singlePlayerKnight->deathText.draw(window);


                backToMenuDelay.start();
                if (backToMenuDelay.getElapsedTime() > backToMenuTime)
                {
                    gameState = GameState::mainMenu;
                }


                break;
            }
            case GameState::level1Pause:
            {

                levelOne.draw(window);
                menuMusic.stop();
                levelDelay.start();
                window.setMouseCursorVisible(false);

                if (levelDelay.getElapsedTime() > levelTime)
                {
                    gameState = GameState::playing;
                }


                break;
            }
            case GameState::end:
            {

                if (playSong4)
                {
                    singlePlayerKnight->enemeyFelledSound.play();
                    playSong4 = false;
                }
                window.draw(backgroundShape);
                singlePlayerLevel.draw(window, tiles, decorationTiles);

                singlePlayerKnight->draw(window);

                for (auto& enemy : enemies)
                {
                    enemy->draw(window);
                }

                for (const auto& arrow2 : arrows)
                {
                    window.draw(arrow2.sprite);

                }

                outline = singlePlayerKnight->ending.getText().getOutlineColor();
                outline.a = static_cast<unsigned char>(alpha);
                singlePlayerKnight->ending.getText().setOutlineColor(outline);

                alpha += fadeSpeed * deltaTime;
                if (alpha > 255.f) alpha = 255.f;
                t = alpha / 255.f;
                eased = t * t;
                c2.a = static_cast<unsigned char>(255.f * eased);
                singlePlayerKnight->ending.getText().setFillColor(c2);

                singlePlayerKnight->ending.setPosition(backgroundShape.getPosition().x + 300, backgroundShape.getPosition().y + 300);
                singlePlayerKnight->ending.draw(window);

                endDelayToMenu.start();

                if (endDelayToMenu.getElapsedTime() > endTimeToMenu)
                {
                    gameState = GameState::mainMenu;
                }

                break;
            }
            case GameState::howtoPlay:
            {

                if (Menu.getBounds().contains(mousePos))
                {
                    Menu.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound4)
                    {
                        menuSelectSound.play();
                        playMenuSound4 = true;
                    }
                }
                else
                {
                    Menu.getText().setFillColor(sf::Color::White); // Normal

                    playMenuSound4 = false;
                }

                if (Next.getBounds().contains(mousePos))
                {
                    Next.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound5)
                    {
                        menuSelectSound.play();
                        playMenuSound5 = true;
                    }
                }
                else
                {
                    Next.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound5 = false;
                }

                window.draw(forestBackgroundShape);
                rules.draw(window);
                rules2.draw(window);
                howToPlay2.draw(window);
                Menu.draw(window);
                Next.draw(window);

                knightWalkDemo.update(deltaTime);
                knightJumpDemo.update(deltaTime);    
                knightSprintDemo.update(deltaTime);            
                knightAttackDemo.update(deltaTime);            
                knightGuardDemo.update(deltaTime);               
                knightRollingDemo.update(deltaTime);        
                knightHangingDemo.update(deltaTime);      
                knightDrinkingDemo.update(deltaTime);              

                tileSprite.setPosition({ 1125.f, 600.f });

                window.draw(tileSprite);
                window.draw(knightWalkDemo.getSprite());
                window.draw(knightJumpDemo.getSprite());
                window.draw(knightSprintDemo.getSprite());
                window.draw(knightAttackDemo.getSprite());
                window.draw(knightGuardDemo.getSprite());
                window.draw(knightRollingDemo.getSprite());
                window.draw(knightHangingDemo.getSprite());
                window.draw(knightDrinkingDemo.getSprite());


                break;
            }
            case GameState::howToPlay2:
            {
                if (Menu.getBounds().contains(mousePos))
                {
                    Menu.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound4)
                    {
                        menuSelectSound.play();
                        playMenuSound4 = true;
                    }
                }
                else
                {
                    Menu.getText().setFillColor(sf::Color::White); // Normal

                    playMenuSound4 = false;
                }

                if (previousHowToPlayPage.getBounds().contains(mousePos))
                {
                    previousHowToPlayPage.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound5)
                    {
                        menuSelectSound.play();
                        playMenuSound5 = true;
                    }
                }
                else
                {
                    previousHowToPlayPage.getText().setFillColor(sf::Color::White); // Normal

                    playMenuSound5 = false;
                }

                if (Next.getBounds().contains(mousePos))
                {
                    Next.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound6)
                    {
                        menuSelectSound.play();
                        playMenuSound6 = true;
                    }
                }
                else
                {
                    Next.getText().setFillColor(sf::Color::White); // Normal

                    playMenuSound6 = false;
                }

                window.draw(forestBackgroundShape);
                howToPlay2.draw(window);
                Menu.draw(window);
                Next.draw(window);
                previousHowToPlayPage.draw(window);
                rules3.draw(window);
                rules4.draw(window);

                knightSprintAttackDemo.update(deltaTime);                
                knightJumpAttackDemo.update(deltaTime);              

                window.draw(knightJumpAttackDemo.getSprite());
                window.draw(knightSprintAttackDemo.getSprite());


                break;
            }
            case GameState::howToPlay3:
            {
                if (Menu.getBounds().contains(mousePos))
                {
                    Menu.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound4)
                    {
                        menuSelectSound.play();
                        playMenuSound4 = true;
                    }
                }
                else
                {
                    Menu.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound4 = false;
                }

                if (previousHowToPlayPage.getBounds().contains(mousePos))
                {
                    previousHowToPlayPage.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound5)
                    {
                        menuSelectSound.play();
                        playMenuSound5 = true;
                    }
                }
                else
                {
                    previousHowToPlayPage.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound5 = false;
                }           

                if (Next.getBounds().contains(mousePos))
                {
                    Next.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound6)
                    {
                        menuSelectSound.play();
                        playMenuSound6 = true;
                    }
                }
                else
                {
                    Next.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound6 = false;
                }

                window.draw(forestBackgroundShape);
                Menu.draw(window);               
                previousHowToPlayPage.draw(window);
                Next.draw(window);

                samuraiWalkDemo.update(deltaTime);
                samuraiSprintDemo.update(deltaTime);
                samuraiJumpDemo.update(deltaTime);
                samuraiAttackDemo.update(deltaTime);
                samuraiSpecialDemo.update(deltaTime);
                samuraiDrinkDemo.update(deltaTime);
                samuraiClimbDemo.update(deltaTime);

                window.draw(samuraiWalkDemo.getSprite());
                window.draw(samuraiSprintDemo.getSprite());
                window.draw(samuraiJumpDemo.getSprite());
                window.draw(samuraiAttackDemo.getSprite());
                window.draw(samuraiSpecialDemo.getSprite());
                window.draw(samuraiDrinkDemo.getSprite());
                window.draw(samuraiClimbDemo.getSprite());
                
                howToPlay2.draw(window);
                samuraiRules.draw(window);
                samuraiRules2.draw(window);

                break;
            }
            case GameState::specialInteractions:
            {
                if (Menu.getBounds().contains(mousePos))
                {
                    Menu.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound4)
                    {
                        menuSelectSound.play();
                        playMenuSound4 = true;
                    }
                }
                else
                {
                    Menu.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound4 = false;
                }

                if (previousHowToPlayPage.getBounds().contains(mousePos))
                {
                    previousHowToPlayPage.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound5)
                    {
                        menuSelectSound.play();
                        playMenuSound5 = true;
                    }
                }
                else
                {
                    previousHowToPlayPage.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound5 = false;
                }

                if (Next.getBounds().contains(mousePos))
                {
                    Next.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound6)
                    {
                        menuSelectSound.play();
                        playMenuSound6 = true;
                    }
                }
                else
                {
                    Next.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound6 = false;
                }

                deflectionDemo.update(deltaTime);
          
                window.draw(forestBackgroundShape);
                Menu.draw(window);
                Next.draw(window);
                previousHowToPlayPage.draw(window);             
                specialInteractions.draw(window);             
                deflectionInfoBox.draw(window);
                demoPlatform.drawOnlyTiles(window, demoTiles);
                deflectionDemo.draw(window);
                break;
            }
            case GameState::specialInteractions2:
            {
                if (Menu.getBounds().contains(mousePos))
                {
                    Menu.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound4)
                    {
                        menuSelectSound.play();
                        playMenuSound4 = true;
                    }
                }
                else
                {
                    Menu.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound4 = false;
                }

                if (previousHowToPlayPage.getBounds().contains(mousePos))
                {
                    previousHowToPlayPage.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound5)
                    {
                        menuSelectSound.play();
                        playMenuSound5 = true;
                    }
                }
                else
                {
                    previousHowToPlayPage.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound5 = false;
                }
                parryDemo.update(deltaTime);
                window.draw(forestBackgroundShape);
                specialInteractions2.draw(window);
                Menu.draw(window);              
                previousHowToPlayPage.draw(window);
                demoPlatform.drawOnlyTiles(window, demoTiles);
                parryInfoBox.draw(window);
                parryDemo.draw(window);
                break;
            }
            case GameState::battleMode:
            {            

                debug.clear();

                debug.addLine("State: ", players.at(myID)->stateToString(players.at(myID)->getState()));
                debug.addLine("Attack Timer: ", std::to_string(players.at(myID)->getAttackTimer()));
                debug.addLine("Frame: ", std::to_string(players.at(myID)->getAnimation()->getCurrentFrame()));
                debug.addBool("Hurt: ", players.at(myID)->isHurtBool());
                debug.addBool("Attacking: ", players.at(myID)->isAttackingBool());
                debug.addBool("Hitbox Active: ", players.at(myID)->isHitboxActive());
                debug.addBool("Dead: ", players.at(myID)->isDeadBool());
                debug.addBool("Grounded: ", players.at(myID)->isGrounded());             
                debug.addBool("Parry Window: ", players.at(myID)->isParryWindow());
              
                spawnY = 100.f;              

                client.receiveNetworkEvent(players, deltaTime, playerColors, spawnpoints, assets,fancyFontText, characterType, standardFont);

                myID = client.getPlayerID();

                if (client.getPlayerID() != -1 && !players.count(myID))
                {
                    sf::Color myColor = playerColors[myID % playerColors.size()]; // Gives each player a unique color health bar

                    float rightX = 1725.f;
                    float leftX = 225.f;
                    float arenaLength = rightX - leftX;

                    int playerCount = players.size() + 1; // including the new one
                    float spacing = arenaLength / (playerCount + 4);

                    float spawnX = leftX + spacing * (myID + 1);
                    float spawnY = 550.f;

                    players.try_emplace(myID, PlayerFactory::create(characterType, spawnpoints[myID].x, spawnpoints[myID].y, assets, fancyFontText, myColor, standardFont));

                }

                if (myID != -1)
                {                    
                    players.at(myID)->updateMultiplayer(deltaTime, multiplayerTiles, standardFont); // update my player

                    if (players.at(myID)->getIsHealed())
                    {
                        players.at(myID)->setHealBool(false);
                        client.sendHeal(myID, players.at(myID)->getHealAmount());
                    }
                }

                for (auto& [id, attackers] : players) // update all other players
                {
                    if (id == myID) continue;                                                              

                    attackers->updateRemotePlayers(deltaTime, multiplayerTiles);

                }
              
                for (auto& [id, attacker] : players) // Attacker is all players except for me
                {                                                           
                    int damage = 0;
                
                        attacker->determineCharacterHitbox();

                        for (auto& [victimID, victim] : players) // Victim is YOUR player
                        {                                            
                           // Arrow Deflection Logic
                           attacker->processProjectileDeflection(*victim, client, id, victimID, standardFont);

                           // Samurai Arrow Logic
                           attacker->processProjectileCollisions(*victim, id, victimID, client, standardFont);                               
                          
                            if (id == victimID) continue;

                            if (deflectionLogic(victim,victimID,attacker,id,client)) continue;

                            bool wasStunned = victim->getStunned();               

                            // Only triggers when vuulnerabilty wears off and when my player is not rolling
                            if (!victim->isInvulnerableBool() && attacker->getAttackBox().getGlobalBounds().findIntersection(victim->getPlayerBox().getGlobalBounds()) && !victim->isRollingBool() && attacker->isHitboxActive())
                            {                                                                  
                                damage = calculatePlayerDamage(attacker->getCurrentAttack().damage, wasStunned);
                                cout << "CURRENT ATTACK DAMAGE: "
                                    << attacker->getCurrentAttack().damage
                                    << endl;

                                victim->applyDamage(damage, attacker->getDirection(), standardFont);                           

                                victim->setInvul(true);
                                victim->setInvulTimer(0.5f);

                                client.sendDamage(id, victimID, damage, attacker->getDirection());
                            }                                                                          
                        }
                }
         
                // send my position to server
                if (myID != -1 && networkClock.getElapsedTime().asMilliseconds() > 33)
                {
                    auto& myPlayer = players.at(myID);

                    client.sendPlayerState(myID, myPlayer->getPlayerBox().getPosition().x, myPlayer->getPlayerBox().getPosition().y,
                        myPlayer->getState(), myPlayer->getDirection());

                    if ((players.at(myID)->getPlayerBox().getPosition().y > 1500.f || players.at(myID)->getHealth() <= 0) && !sendDeathPacketOnce) // Detects if player fell off map
                    {
                        cout << "-----------------------------" << endl;
                        cout << "SENDING DEATH: " << myID << endl;
                        cout << "-----------------------------" << endl;

                        client.sendDeath(myID); // Send health update to all players
                        sendDeathPacketOnce = true;
                    }

                    isAttackingNow = myPlayer->isAttackingBool();

                    // Only send ONE attack packet
                    if (isAttackingNow && !wasAttackingLastFrame)
                    {
                        attackID = myPlayer->determineAttackID();
                        
                        client.sendAttack(myID, myPlayer->getPlayerBox().getPosition().x, myPlayer->getPlayerBox().getPosition().y,
                            myPlayer->getDirection(), attackID);
                    }

                    wasAttackingLastFrame = isAttackingNow;

                    if (auto* samurai = dynamic_cast<Samurai*>(myPlayer.get()))
                    {
                        if (samurai->didArrowSpawnThisFrame())
                        {
                            client.sendArrow(
                                myID,
                                samurai->getPlayerBox().getPosition().x,
                                samurai->getPlayerBox().getPosition().y,
                                samurai->getDirection(),
                                samurai->getlastArrowID(),
                                samurai->getArrowGravity()
                            );

                            samurai->clearArrowSpawnFlag();
                        }

                        if (samurai->isBowPulledBack() && samurai->isSpecialAttacking() &&
                            samurai->getAnimation()->getCurrentFrame() == 3)
                        {
                            client.sendBowPullbackSound(myID);
                        }
                    }

                    networkClock.restart();
                }

                window.draw(multiplayerBackgroundShape);

                multiplayerArena.draw(window, multiplayerTiles, multiplayerDecorationTiles);

                slot = 0;

                 // Draw all other Players
                drawMultiplayerOpponentsAndHUD(
                    window,
                    players,
                    client,
                    font,
                    HUDlayout,
                    potionSprite,
                    multiplayerInventory,
                    playerColors,
                    deltaTime,
                    multiplayerBackgroundShape.getPosition().y + 1000.f
                );

                if (client.gameEnded() && !winClockStarted)
                {
                    winClock.restart();
                    winClockStarted = true;
                }


                if (client.gameEnded())
                {                   
                    float t = winClock.getElapsedTime().asSeconds();

                    // pause before fade
                    if (t < winDelay)
                    {
                        alpha = 0;
                    }
                    else
                    {
                        if (!playWinnerSound)
                        {
                            winnerSound.play();
                            playWinnerSound = true;
                        }

                        float fadeProgress = (t - winDelay) / fadeDuration;

                        if (fadeProgress > 1)
                        {                          
                            fadeProgress = 1;

                            if (!pauseToLobby)
                            {
                                pauseBeforeBackToLobby.restart();
                                pauseToLobby = true;
                            }
                        }

                        alpha = fadeProgress * 255;
                    }

                    if (pauseToLobby && pauseBeforeBackToLobby.getElapsedTime().asSeconds() >= 3)
                    {
                        battleMusic.stop();
                        menuMusic.play();

                        gameState = GameState::preGameLobby;
                        
                        // winner screen
                        alpha = 0;
                        winClockStarted = false;                                            

                        // timers
                        winClock.restart();
                        pauseBeforeBackToLobby.restart();

                        // gameplay flags
                        client.setGameEnded(false);

                        // optional but usually needed
                        players.clear();                        
                        pauseToLobby = false;
                    }

                    sf::Text winner(standardFont);

                   
                    winner.setString(client.getWinnerName() + " WINS");
                    winner.setCharacterSize(300);
                    sf::Color color = sf::Color::Yellow;
                    color.a = static_cast<std::uint8_t>(alpha);

                    winner.setFillColor(color);
                    winner.setOutlineColor(sf::Color(0, 0, 0, alpha));
                    winner.setOutlineThickness(5);

                    sf::FloatRect bounds = winner.getLocalBounds();
                    winner.setOrigin({ bounds.size.x / 2, bounds.size.y / 2 });
                    winner.setPosition({ 960, 300 });

                    window.draw(winner);                   
                }

                debug.draw(window);
            
                break;
            }
            case GameState::enterUsername:
            {           
                nametag.update();

                if (Continue.getBounds().contains(mousePos))
                {
                    Continue.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound6)
                    {
                        menuSelectSound.play();
                        playMenuSound6 = true;
                    }
                }
                else
                {
                    Continue.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound6 = false;
                }

                window.draw(loadingBackgroundShape);              
                Username.draw(window);
                nametag.draw(window);

                if (nametag.str.length() > 0)
                {
                    Continue.draw(window);
                }
              
               
                break;
            }
            case GameState::characterSelect:
            {             
                if (knightButton.getGlobalBounds().contains(mousePos))
                {
                    knightText.getText().setFillColor(sf::Color::Yellow); // Highlight                   
                 
                    characterSelectKnightIdleDemo.getSprite().setPosition({ 500.f, 90.f - 10.f});

                    if (!playMenuSound11)
                    {
                        menuSelectSound.play();
                        playMenuSound11 = true;
                    }
                }
                else
                {
                    knightText.getText().setFillColor(sf::Color::White); // Normal

                    characterSelectKnightIdleDemo.getSprite().setPosition({ 500.f, 90.f});

                    playMenuSound11 = false;
                }

                if (samuraiButton.getGlobalBounds().contains(mousePos))
                {
                    samuraiText.getText().setFillColor(sf::Color::Yellow); // Highlight                   

                    characterSelectSamuraiIdleDemo.getSprite().setPosition({ 900.f, 95.f - 10.f });

                    if (!playMenuSound12)
                    {
                        menuSelectSound.play();
                        playMenuSound12 = true;
                    }
                }
                else
                {
                    samuraiText.getText().setFillColor(sf::Color::White); // Normal

                    characterSelectSamuraiIdleDemo.getSprite().setPosition({ 900.f, 95.f });

                    playMenuSound12 = false;
                }

                if (Back.getBounds().contains(mousePos))
                {
                    Back.getText().setFillColor(sf::Color::Yellow); // Highlight                   
               
                    if (!playMenuSound)
                    {
                        menuSelectSound.play();
                        playMenuSound = true;
                    }
                }
                else
                {
                    Back.getText().setFillColor(sf::Color::White); // Normal                 

                    playMenuSound = false;
                }
             
                if (howToPlayCharacterSelect.getBounds().contains(mousePos))
                {
                    howToPlayCharacterSelect.getText().setFillColor(sf::Color::Yellow); // Highlight                   

                    if (!playMenuSound2)
                    {
                        menuSelectSound.play();
                        playMenuSound2 = true;
                    }
                }
                else
                {
                    howToPlayCharacterSelect.getText().setFillColor(sf::Color::White); // Normal                 

                    playMenuSound2 = false;
                }


                characterSelectKnightIdleDemo.update(deltaTime);    
                characterSelectSamuraiIdleDemo.update(deltaTime);

                window.draw(loadingBackgroundShape);           
                characterSelect.draw(window);
                window.draw(characterSelectBackground1);
                window.draw(characterSelectBackground2);
                knightText.draw(window);
                samuraiText.draw(window);                        
                Back.draw(window);
                window.draw(characterSelectKnightIdleDemo.getSprite());
                window.draw(characterSelectSamuraiIdleDemo.getSprite());           
                           
                if (pressedLobbyCharacterSelectButton)
                {
                    window.draw(statsBanner1);
                    window.draw(statsBanner2);
                    window.draw(statsBanner3);
                    window.draw(statsBanner4);
                    howToPlayCharacterSelect.draw(window);
                }

                break;
            }       
            case GameState::serverPage:
            {

                if (Back.getBounds().contains(mousePos))
                {
                    Back.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound8)
                    {
                        menuSelectSound.play();
                        playMenuSound8 = true;
                    }
                }
                else
                {
                    Back.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound8 = false;
                }

                if (JoinServer.getBounds().contains(mousePos))
                {
                    JoinServer.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound6)
                    {
                        menuSelectSound.play();
                        playMenuSound6 = true;
                    }
                }
                else
                {
                    JoinServer.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound6 = false;
                }

                if (CreateServer.getBounds().contains(mousePos))
                {
                    CreateServer.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound7)
                    {
                        menuSelectSound.play();
                        playMenuSound7 = true;
                    }
                }
                else
                {
                    CreateServer.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound7 = false;
                }



                window.draw(multiplayerBackgroundShape);
                window.draw(serverBanner1);
                window.draw(serverBanner2);
                window.draw(serverBanner3);
                window.draw(serverBanner4);
                window.draw(serverBanner5);
                window.draw(serverBanner6);
                window.draw(serverBanner7);
                window.draw(serverBanner8);
                JoinServer.draw(window);
                CreateServer.draw(window);
                Back.draw(window);

                break;
            }
            case GameState::joinServer:
            {


                if (Back.getBounds().contains(mousePos))
                {
                    window.setMouseCursor(CursorHand);
                    Back.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound8)
                    {
                        menuSelectSound.play();
                        playMenuSound8 = true;
                    }
                }
                else
                {
                    Back.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound8 = false;
                }

                if (Refresh.getBounds().contains(mousePos))
                {
                    window.setMouseCursor(CursorHand);
                    Refresh.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound10)
                    {
                        menuSelectSound.play();
                        playMenuSound10 = true;
                    }
                }
                else
                {
                    Refresh.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound10 = false;
                }

                client.updateLobby();

                window.draw(serverListBackgroundShape);
                window.draw(ServerListSprite);

                if (client.getServerList().size() != lastServerCount)
                {
                    rebuildServerUI = true;
                    lastServerCount = client.getServerList().size();
                }

                if (rebuildServerUI)
                {
                    serverEntries.clear();

                    for (int i = 0; i < client.getServerList().size(); i++)
                    {
                        float x = 1300;
                        float y = 300.f + i * 100;

                        ServerEntryUI entry(font);

                        entry.serverIndex = i;

                        entry.text.setFont(font);
                        entry.text.setCharacterSize(40);
                        entry.text.setFillColor(sf::Color::White);
                        entry.text.setPosition({ x, y });
                        entry.text.setOutlineThickness({ 5 });
                        entry.text.setOutlineColor(sf::Color::Black);

                        entry.text.setString(
                            client.getServerList()[i].name + "  " + "(" +
                            std::to_string(client.getServerList()[i].currentPlayers) + "/" +
                            std::to_string(client.getServerList()[i].maxPlayers) + ")"
                        );

                        serverEntries.emplace_back(entry);


                    }

                    rebuildServerUI = false;
                }

                for (auto& entry : serverEntries)
                {
                    if (entry.text.getGlobalBounds().contains(mousePos))
                    {
                        window.setMouseCursor(CursorHand);
                        if (!entry.hovered)
                        {
                            menuSelectSound.play();
                            entry.hovered = true;
                        }

                        entry.text.setFillColor(sf::Color::Yellow);
                    }
                    else
                    {
                        entry.hovered = false;
                        entry.text.setFillColor(sf::Color::White);
                    }
                }


                for (auto& entry : serverEntries)
                {
                    window.draw(entry.text);
                }


                Back.draw(window);
                Refresh.draw(window);

                break;
            }
            case GameState::enterPassword:
            {
                passwordAttemptBox.update();

                if (Continue.getBounds().contains(mousePos))
                {
                    window.setMouseCursor(CursorHand);
                    Continue.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound10)
                    {
                        menuSelectSound.play();
                        playMenuSound10 = true;
                    }
                }
                else
                {
                    Continue.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound10 = false;
                }

                if (Back.getBounds().contains(mousePos))
                {
                    window.setMouseCursor(CursorHand);
                    Back.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound9)
                    {
                        menuSelectSound.play();
                        playMenuSound9 = true;
                    }
                }
                else
                {
                    Back.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound9 = false;
                }


                window.draw(multiplayerBackgroundShape);
                passwordAttemptBox.draw(window);
                EnterPassword.draw(window);
                Back.draw(window);

                if (passwordAttemptBox.letterCount > 0) Continue.draw(window);
                if (showErrorMessage)
                {
                    errorMessage.setString("Incorrect Password (" + std::to_string(errorMessageCount) + ")");
                    errorMessage.draw(window);
                }
                   

                break;
            }
            case GameState::createServer:
            {
                if (Create.getBounds().contains(mousePos))
                {
                    window.setMouseCursor(CursorHand);
                    Create.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound7)
                    {
                        menuSelectSound.play();
                        playMenuSound7 = true;
                    }
                }
                else
                {
                    Create.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound7 = false;
                }

                if (Back.getBounds().contains(mousePos))
                {
                    window.setMouseCursor(CursorHand);
                    Back.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound8)
                    {
                        menuSelectSound.play();
                        playMenuSound8 = true;
                    }
                }
                else
                {
                    Back.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound8 = false;
                }

                if (serverNameBox.box.getGlobalBounds().contains(mousePos) || passwordBox.box.getGlobalBounds().contains(mousePos) ||
                    maxPlayersBox.getGlobalBounds().contains(mousePos))
                {
                    window.setMouseCursor(CursorHand);
                }

                maxPlayersText.setString("Max Players:  " + std::to_string(maxPlayers));

                serverNameBox.update();
                passwordBox.update();
                nametag.update();


                window.draw(createServerBackgroundShape);

                window.draw(maxPlayersBox);
                window.draw(maxPlayersText);

                serverNameBox.draw(window);
                passwordBox.draw(window);
                
                Back.draw(window);
                ServerName.draw(window);
                Password.draw(window);
               
                window.draw(serverBanner9);
                window.draw(serverBanner10);
                window.draw(serverBanner11);

                if (serverNameBox.letterCount > 0)
                  Create.draw(window);

                break;

            }
            case GameState::loading:
            {              
                menuMusic.stop();

                if (loadingClock.getElapsedTime().asSeconds() >= 2.f)
                {         
                   
                   HUDlayout = calculateHealthbarPositioning(client.getLobbyPlayers().size());
                   gameState = GameState::countdown;                    
                }

                window.draw(loadingBackgroundShape);
                Loading.draw(window);

                break;
            }
            case GameState::preGameLobby:
            {               
                
                if (client.loadingStarted())
                {
                    menuMusic.stop();
                    gameState = GameState::loading;
                    loadingClock.restart();
                }               

                sendDeathPacketOnce = false;
                playWinnerSound = false;

                if (client.moveServerPageStarted())
                {
                    gameState = GameState::serverPage;
                }

                int playerID = client.getPlayerID();

                if (playerID != -1 && characterSelected && pressedLobbyCharacterSelectButton)
                {
                    std::cout << "CHANGE CHARACTER SENT"
                        << " | ID: " << playerID
                        << " | Character: " << static_cast<int>(characterType)
                        << std::endl;

                    client.sendCharacterChange(playerID, characterType);
                    characterSelected = false;
                    pressedLobbyCharacterSelectButton = false;
                }

                client.receiveNetworkEvent(players, deltaTime, playerColors, spawnpoints, assets, fancyFontText, characterType, standardFont);             

                float x = 500.f;                
                int i = 0;
                
                sf::Text text2(standardFont);
                text2.setString("Players: (" + std::to_string(client.getLobbyPlayers().size()) + "/" + std::to_string(client.maxLobbySize) + ")");

                text2.setCharacterSize(80);
                text2.setOutlineColor(sf::Color::Black);
                text2.setOutlineThickness(4);
                text2.setPosition({ 300.f, 75.f });

                sf::Text text3(standardFont);
                text3.setString("Host:  " + client.getLobbyPlayers()[client.getHostID()].name);

                text3.setCharacterSize(80);
                text3.setOutlineColor(sf::Color::Black);
                text3.setOutlineThickness(4);
                text3.setPosition({ 300.f, 155.f });

                sf::Text text4(standardFont);

                if (client.isHost())
                {
                    for (auto& server : client.getServerList())
                    {
                        if (server.id == client.getServerID())
                        {
                            text4.setString("Password: " + server.password);
                        }
                    }
                }
        
                text4.setCharacterSize(80);
                text4.setOutlineColor(sf::Color::Black);
                text4.setOutlineThickness(4);
                text4.setPosition({ 300.f, 235.f });
                
                drawWaitingForPlayersGameState(window, multiplayerBackgroundShape, WoodenGUISprite, WoodenGUISprite2);
                                             
                 knightIdleDemo.update(deltaTime);
                                                             
                 samuraiIdleDemoCharacterSelect.update(deltaTime);                 
           
                for (auto& [id, player] : client.getLobbyPlayers())
                {   
                    sf::Color myColor = playerColors[id % playerColors.size()];

                   // cout << "LOBBY SIZE: " << client.getLobbyPlayers().size() << endl;

                    sf::Text text(standardFont);
                    text.setString(player.name);
                    text.setPosition({ x, 950.f });
                    text.setCharacterSize(40);
                    text.setOutlineColor(sf::Color::Black);
                    text.setOutlineThickness(2);
                    text.setFillColor(myColor);                  
                                  
                    if (player.character == CharacterType::Knight)
                    {
                        knightIdleDemo.setPosition(x + (text.getLocalBounds().size.x / 2), 780);                        
                        window.draw(knightIdleDemo.getSprite());
                    }
                    else if (player.character == CharacterType::Samurai)
                    {
                        samuraiIdleDemoCharacterSelect.setPosition(x - 130.f + (text.getLocalBounds().size.x / 2), 700);
                        window.draw(samuraiIdleDemoCharacterSelect.getSprite());
                    }

                    float width = text.getLocalBounds().size.x;
                    x += width + 150;

                    window.draw(text);
                                                  
                    i++;
                }

                window.draw(text2);
                window.draw(text3);
                window.draw(text4);
                window.draw(characterSelectBackground);
                characterSelectLobby.draw(window);
                window.draw(ArenaPreviewSprite);         
                window.draw(arenaSprite1);
                window.draw(ArenaBannerSprite);           
                Arena.draw(window);

                if (client.isHost())
                {
                    if (client.getLobbyPlayers().size() == client.maxLobbySize)
                    {
                        if (StartGame.getBounds().contains(mousePos))
                        {
                            window.setMouseCursor(CursorHand);
                            StartGame.getText().setFillColor(sf::Color::Yellow); // Highlight

                            if (!playMenuSound8)
                            {
                                menuSelectSound.play();
                                playMenuSound8 = true;
                            }
                        }
                        else
                        {
                            StartGame.getText().setFillColor(sf::Color::White); // Normal
                            playMenuSound8 = false;
                        }

                        window.draw(startGameBackground);
                        StartGame.draw(window);                     
                    }             
                }
               

                    if (Leave.getBounds().contains(mousePos))
                    {
                        window.setMouseCursor(CursorHand);
                        Leave.getText().setFillColor(sf::Color::Red); // Highlight

                        if (!playMenuSound8)
                        {
                            menuSelectSound.play();
                            playMenuSound8 = true;
                        }
                    }
                    else
                    {
                        Leave.getText().setFillColor(sf::Color::White); // Normal
                        playMenuSound8 = false;
                    }

                    Leave.draw(window);
                

                if (characterSelectLobby.getBounds().contains(mousePos))
                {
                    window.setMouseCursor(CursorHand);
                    characterSelectLobby.getText().setFillColor(sf::Color::Yellow); // Highlight

                    if (!playMenuSound)
                    {
                        menuSelectSound.play();
                        playMenuSound = true;
                    }
                }
                else
                {
                    characterSelectLobby.getText().setFillColor(sf::Color::White); // Normal
                    playMenuSound = false;
                }
                                                   
                break;
            }
            case GameState::countdown:
            {           
                window.setView(battleView);

                client.receiveNetworkEvent(players, deltaTime, playerColors, spawnpoints, assets, fancyFontText, characterType, standardFont);

                myID = client.getPlayerID();

                if (client.getPlayerID() != -1 && !players.count(myID))
                {
                    sf::Color myColor = playerColors[myID % playerColors.size()]; // Gives each player a unique color health bar

                    float rightX = 1725.f;
                    float leftX = 225.f;
                    float arenaLength = rightX - leftX;

                    int playerCount = players.size() + 1; // including the new one
                    float spacing = arenaLength / (playerCount + 4);

                    float spawnX = leftX + spacing * (myID + 1);
                    float spawnY = 597.f;

                    players.try_emplace(myID, PlayerFactory::create(characterType, spawnpoints[myID].x, spawnpoints[myID].y, assets, fancyFontText, myColor, standardFont));

                }               

                for (auto& [id, player] : players) 
                {        
                    player->getAnimation()->update(deltaTime);
                }
            
                // send my position to server
                if (myID != -1 && networkClock.getElapsedTime().asMilliseconds() > 5)
                {
                    auto& myKnight = players.at(myID);

                    client.sendPlayerState(myID, myKnight->getPlayerBox().getPosition().x, myKnight->getPlayerBox().getPosition().y,
                        myKnight->getState(), myKnight->getDirection());

                }
                    

                //cout << localKnight.stateToString(localKnight.getState()) << endl;

                window.draw(multiplayerBackgroundShape);

                multiplayerArena.draw(window, multiplayerTiles, multiplayerDecorationTiles);

                slot = 0;               

                 // Draw all other Players
                   // Draw all other Players
                drawMultiplayerOpponentsAndHUD(
                    window,
                    players,
                    client,
                    font,
                    HUDlayout,
                    potionSprite,
                    multiplayerInventory,
                    playerColors,
                    deltaTime,
                    multiplayerBackgroundShape.getPosition().y + 1000.f
                );

                sf::FloatRect bounds = countdownText.getText().getLocalBounds();
                countdownText.getText().setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });              
                countdownText.setPosition(GAME_WIDTH / 2.f, GAME_HEIGHT / 2.f - 300.f);

                if (client.goTimer > 0)
                {
                    client.goTimer -= deltaTime;  // dt = delta time
                }

                if (!client.gameStarted && client.countdownTimer > 0)
                {       
                    if (!countdownStarted)
                    {
                        countdownSound.play();
                        countdownStarted = true;
                    }
                    
                    countdownText.setString(std::to_string((int)ceil(client.countdownTimer)));
                    countdownText.draw(window);
                }
                else if (client.goTimer > 0)
                {                   
                    countdownText.setString("FIGHT");
                    countdownText.draw(window);
                }

                

                if (client.gameStarted && client.goTimer == 0) 
                {   
                    battleSongLogic(battleMusic);
                    gameState = GameState::battleMode; 
                    countdownStarted = false;
                }
    
                break;
            

            }
            case GameState::testingGrounds:
            {

                testSamurai->updateMultiplayer(deltaTime, multiplayerTiles, standardFont);
             
                testSamurai->determineCharacterHitbox();

                window.draw(backgroundShape);
                multiplayerArena.draw(window, multiplayerTiles, multiplayerDecorationTiles);
               // testSamurai->draw(window);
                break;
            }

        }
               
       //singlePlayerKnight.drawCollisionBox(window);

        window.display();

       
    }
}