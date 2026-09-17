#include "Animation.hpp"

struct HUDLayout
{
    float startX;
    float barWidth;
    float spacing;

    float potionXOffset;
    float potionCountXOffset;
    float inventoryXOffset;
};

void drawWaitingForPlayersGameState(sf::RenderWindow& window, sf::RectangleShape& background, sf::Sprite& woodenGUI, sf::Sprite& woodenGUI2)
{
	window.draw(background);
	window.draw(woodenGUI2);
	window.draw(woodenGUI);
	
}

bool deflectionLogic(unique_ptr<Player>& victim, int victimID, unique_ptr<Player>& attacker, int attackerID,Client& client)
{
    bool attackerIsRealAttack =
        attacker->getState() == PlayerState::Attack1 ||
        attacker->getState() == PlayerState::Attack2 ||
        attacker->getState() == PlayerState::Attack3 ||
        attacker->getState() == PlayerState::SprintAttack ||
        attacker->getState() == PlayerState::JumpAttack;

  /*  if (victim->isParryWindow())
    {
        std::cout << "----- DEFLECT CHECK -----\n";
        std::cout << "Parry window: " << victim->isParryWindow() << '\n';
        std::cout << "Real attack: " << attackerIsRealAttack << '\n';
        std::cout << "Hitbox active: " << attacker->isHitboxActive() << '\n';
        std::cout << "Attack box collision: " << attacker->getAttackBox().getGlobalBounds().findIntersection(
            victim->getAttackBox().getGlobalBounds()).has_value() << '\n';
    }*/

    bool hitsParryBox =
        attacker->getAttackBox().getGlobalBounds()
        .findIntersection(
            victim->getAttackBox().getGlobalBounds()
        ).has_value();

    bool hitsBody =
        attacker->getAttackBox().getGlobalBounds()
        .findIntersection(
            victim->getPlayerBox().getGlobalBounds()
        ).has_value();



    if (victim->isParryWindow())
    {
        std::cout << "Parry: " << hitsParryBox
            << " | Body: " << hitsBody
            << " | ParryWindow: " << victim->isParryWindow()
            << " | HitboxActive: " << attacker->isHitboxActive()
            << '\n';
    }
    
    if (victim->isParryWindow() &&
       attackerIsRealAttack &&
        attacker->isHitboxActive() &&
        attacker->getAttackBox().getGlobalBounds().findIntersection(
            victim->getAttackBox().getGlobalBounds()))
    {
        cout << "DEFLECTED" << endl;
        attacker->cancelAttack();
        client.sendDeflect(victimID, attackerID);
        return true;
    }



    return false;
}

int calculatePlayerDamage(int baseDamage, bool wasStunned)
{
    if (wasStunned) baseDamage += static_cast<int>(std::floor((baseDamage / 2)));

    return baseDamage;
}

void battleSongLogic(sf::Music& music)
{
    int randomNum = rand() % 3 + 1;

    if (randomNum == 1) music.openFromFile("Sounds/battleSong1.mp3");
    else if (randomNum == 2) music.openFromFile("Sounds/battleSong2.wav");
    else music.openFromFile("Sounds/battleSong3.mp3");

    music.setVolume(70.f);

   music.play();
}

sf::FloatRect getLetterboxView(sf::View view, int windowWidth, int windowHeight)
{
    float windowRatio = (float)windowWidth / windowHeight;
    float viewRatio = view.getSize().x / view.getSize().y;

    float sizeX = 1.f;
    float sizeY = 1.f;
    float posX = 0.f;
    float posY = 0.f;

    bool horizontalSpacing = windowRatio > viewRatio;

    if (horizontalSpacing)
    {
        sizeX = viewRatio / windowRatio;
        posX = (1.f - sizeX) / 2.f;
    }
    else
    {
        sizeY = windowRatio / viewRatio;
        posY = (1.f - sizeY) / 2.f;
    }

    view.setViewport(sf::FloatRect({ posX, posY }, { sizeX, sizeY }));

    return view.getViewport();
}

HUDLayout calculateHealthbarPositioning(int numPlayers)
{
    HUDLayout layout;

    switch (numPlayers)
    {
    case 2:
        layout.barWidth = 470.f;
        layout.spacing = 700.f;

        layout.potionXOffset = 410.f;
        layout.potionCountXOffset = 465.f;
        layout.inventoryXOffset = 420.f;

        break;

    case 3:
        layout.barWidth = 380.f;
        layout.spacing = 500.f;

        layout.potionXOffset = 315.f;
        layout.potionCountXOffset = 370.f;
        layout.inventoryXOffset = 325.f;

        break;

    default: // 4+
        layout.barWidth = 300.f;
        layout.spacing = 400.f;

        layout.potionXOffset = 265.f;
        layout.potionCountXOffset = 320.f;
        layout.inventoryXOffset = 275.f;

        break;
    }

    float totalWidth =
        numPlayers * layout.barWidth +
        (numPlayers - 1) * (layout.spacing - layout.barWidth);

    layout.startX = (1920.f - totalWidth) / 2.f;

    return layout;
}

void drawMultiplayerOpponentsAndHUD(
    sf::RenderWindow& window,
    std::unordered_map<int, std::unique_ptr<Player>>& players,
    Client& client,
    sf::Font& font,
    const HUDLayout& HUDlayout,
    sf::Sprite& potionSprite,
    sf::Sprite& multiplayerInventory,
    const std::vector<sf::Color>& playerColors,
    float deltaTime,
    float yPos)
{
    int slot = 0;

    for (auto& [id, player] : players)
    {
        sf::Text potionCount(font);

        float xPos = HUDlayout.startX + slot * HUDlayout.spacing;

        player->getHealthBar().setBarWidth(HUDlayout.barWidth);
        player->getHealthBar().update(
            deltaTime,
            player->getHealth(),
            player->getMaxHealth(),
            xPos,
            yPos
        );

        potionSprite.setPosition({ xPos + HUDlayout.potionXOffset, yPos - 37.f });
        potionCount.setPosition({ xPos + HUDlayout.potionCountXOffset, yPos + 10.f });
        multiplayerInventory.setPosition({ xPos + HUDlayout.inventoryXOffset, yPos - 70.f });

        player->draw(window);

        player->getHealthBar().draw(window);
        window.draw(multiplayerInventory);
        window.draw(potionSprite);

        potionCount.setOutlineThickness(2.f);
        potionCount.setString(std::to_string(player->getPotionNumber()));
        window.draw(potionCount);

        if (client.getLobbyPlayers().count(id))
        {
            sf::Text nameText(font);
            nameText.setString(client.getLobbyPlayers()[id].name);
            nameText.setCharacterSize(32);
            nameText.setOutlineThickness(3);
            nameText.setFillColor(playerColors[id]);

            // Above character
            nameText.setPosition({
                player->getPlayerBox().getPosition().x - nameText.getLocalBounds().size.x / 2.f,
                player->getPlayerBox().getPosition().y - 10.f
                });
            window.draw(nameText);

            // HUD
            nameText.setPosition({ xPos, yPos - 45.f });
            window.draw(nameText);

            // Health text above health bar
            sf::Text healthText(font);
            healthText.setString("(" + std::to_string(player->getHealth()) + "/" + std::to_string(player->getMaxHealth()) + ")");
            healthText.setCharacterSize(32);
            healthText.setOutlineThickness(3);
            healthText.setFillColor(playerColors[id]);
            healthText.setPosition({ xPos + nameText.getGlobalBounds().size.x + 15.f, yPos - 45.f});

            window.draw(healthText);
       
        }

        slot++;
    }
}