#pragma once
#include "Animation.hpp"
#include "PlayerState.hpp"
#include "attackData.h"
#include "InputState.hpp"
#include "Tile.hpp"
#include <vector>
#include "Healthbar.hpp"
#include "Enemy.hpp"
#include "Arrow.hpp"
#include "text.hpp"

using namespace std;

struct DamageNumber
{
    sf::Text text;
    sf::Vector2f velocity;
    float lifetime = 0.8f;
    float maxLifetime = 0.8f;

    DamageNumber(sf::Font& standardFont) :
        text(standardFont)
    {
    }
};


class Client;

class Player
{
public:

    Player(int maxHealth, int healAmount, float walkSpeed, float runSpeed, float jumpStrength, float gravity, float knockbackDistance, sf::Color& color, sf::Font& standardFont) :
        maxHealth(maxHealth), 
        healAmount(healAmount),
        health(maxHealth), 
        walkSpeed(walkSpeed),
        runSpeed(runSpeed), 
        jumpStrength(jumpStrength),
        gravity(gravity), 
        knockbackDistance(knockbackDistance),
        healthbar(300,color, maxHealth),
        walkSound1(walkBuffer1),
        walkSound2(walkBuffer2),
        jumpSound1(jumpBuffer1),
        jumpSound2(jumpBuffer2),
        hurtSound1(hurtBuffer1),
        hurtSound2(hurtBuffer2),
        hurtSound3(hurtBuffer3),
        attackSound1(attackBuffer1),
        attackSound2(attackBuffer2),
        attackSound3(attackBuffer3),
        parrySound(parrySoundBuffer),
        deathSound1(deathBuffer1),
        deathSound2(deathBuffer2),
        shieldImpactSound1(shieldImpactBuffer1),
        shieldImpactSound2(shieldImpactBuffer2),
        potionSprite(potionTexture),
        swordHitSound1(swordHit1Buffer),
        swordHitSound2(swordHit2Buffer),
        swordHitSound3(swordHit3Buffer),
        bowShotSound(bowShotBuffer),
        bowPullbackSound(bowPullbackBuffer),
        rollSound1(rollSound1Buffer),
        rollSound2(rollSound2Buffer),
        hangSound1(hangBuffer1),
        potionSound(potionSoundBuffer),
        deathText("Fonts/OldLondon.ttf", "You Died", 500.f, 1000.f, sf::Color::Red, 250),
        potionNumText("Fonts/OldLondon.ttf", "", 500.f, 1000.f, sf::Color::White, 40),
        ending("Fonts/OldLondon.ttf", "Dragon Slain", 840, 800.f, sf::Color::White, 250),
        enemeyFelledSound(enemeyFelledBuffer)
    {
        initializeSounds();

        int randomNumber = std::rand() % 3 + 1;

        if (randomNumber == 1)
           music.openFromFile("Sounds/music.mp3");  
        else if (randomNumber == 2)
           music.openFromFile("Sounds/music2.mp3");  
        else
           music.openFromFile("Sounds/Woodland Fantasy.mp3");
        
        deathMusic.openFromFile("Sounds/YouDied.mp3");
    }

    virtual void draw(sf::RenderWindow& window) = 0;

    virtual void updateMultiplayer(float dt, vector<Tile>& tiles, sf::Font& standardFont) = 0;
    virtual void updateSinglePlayer(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<Arrow>& arrows, sf::Font& standardFont);
    
    virtual void handleActions(InputState& input, float dt) = 0;

    virtual InputState readInput();

    virtual void handleMovement(InputState& input) = 0;

    virtual void switchAnimationByState(PlayerState& state) = 0;

    void updateGravity(float dt, std::vector<Tile>& tiles);

    virtual bool tryStartJump(InputState& input) = 0;
    virtual void jumpLogic() = 0;

    virtual bool tryStartNormalAttacks(InputState& input, float dt) = 0;
    virtual void attackLogicMultiplayer() = 0;
    virtual void updateAttack(float dt) = 0;
    virtual void updateAttackCooldown(float dt) = 0;

    virtual bool tryStartSpecial(InputState& input) = 0;
    virtual void specialAttackLogic() = 0;
    virtual void updateSpecialAttack(float dt, std::vector<Tile>& tiles) = 0;

    bool tryStartElixir(InputState& input);
    void elixirLogic();
    virtual bool cancelElixir() = 0;
    void updatePotionLogic(sf::Font& standardFont);

    virtual void climbingLogic(std::vector<Tile>& tiles) = 0;

    virtual void updateBoundryBoxes() = 0;

    virtual void determineCharacterHitbox() = 0;

    virtual void updateHitBoxWindow(float dt) = 0;

    virtual AttackHitbox* getAttackBoxTable() = 0;

    virtual const attackData* getAttackTable() const = 0;

    virtual void startAttack(const attackData& attack, Direction dir) = 0;

    virtual void cancelAttack() = 0;

    virtual bool isAttackingBool() = 0;

    virtual void updateRemotePlayers(float dt, vector<Tile>& tiles) = 0;

    virtual void applyStun(float duration) = 0;

    virtual bool isParryWindow() = 0;

    virtual int determineAttackID() = 0;

    virtual bool handleDeathLogic(float dt, std::vector<Tile>& tiles) = 0;

    virtual sf::Vector2f animationOffset() = 0;

    virtual void updateStun(float dt);

    virtual void processProjectileCollisions(Player& victim, int attackerID, int victimID, Client& client, sf::Font& standardFont);

    virtual void processProjectileDeflection(Player& victim, Client& client, int attackerID, int victimID, sf::Font& standardFont);

    virtual bool isDeflectionWindow();

    PlayerState getState();
    Direction getDirection();
    sf::RectangleShape getPlayerBox();
    sf::RectangleShape getAttackBox();
    bool isHitboxActive();
    bool getStunned();
    attackData getCurrentAttack();
    bool isHurtBool();
    bool isRollingBool();
    bool isInvulnerableBool();
    void setInvul(bool x);
    void setInvulTimer(float timer);
    int getHealth();
    Healthbar& getHealthBar();
    int getMaxHealth();
    Animation* getAnimation();
    bool getIsHealed();
    int getHealAmount();
    bool isDeadBool();
    float getAttackTimer();
    void setHealBool(bool healed);
    void setHealth(int health);
    sf::Sprite& getPotionSprite();
    int getPotionNumber();
    void setPotionNumber(int num);
    bool isGrounded();
    float getInvulTimer();
    bool isSpecialAttacking();
    vector<DamageNumber>& getDamageNumber();
  
    void setNetworkState(float x, float y, PlayerState netState, Direction netDir);

    virtual float getGuardTimer();

    void applyDamage(int damage, Direction attackerDir, sf::Font& standardFont);

    void startKnockback(Direction dir);

    void resetPlayer(sf::Vector2f spawnPos, bool isSinglePlayer = false);

    string stateToString(PlayerState state);

    void multiplayerDeath();

    void updateDamageText(float dt);
    DamageNumber createDamageNumberText(sf::Font& standardFont, string message, int scaleX, int scaleY, sf::Color color, sf::Vector2f position);

    void handleWalkSounds();
    void playParrySound();
    void playDeflectionSounds();
    void determineHitSound();
    void playDeathSound();
    void playBowPullBackSound();
    void playBowShotSound();
    void playPotionSound();

    void playOtherSounds(PlayerState state);
    void playJumpSound();
    void playRollSound();
    void playLedgeSound();

    bool isBowPulledBack();
    void clearBowPullbackBool();

    Text deathText;
    sf::Music music;
    sf::Music deathMusic;

  //  Text healthText;
    Text potionNumText;
    Text ending;

    sf::SoundBuffer enemeyFelledBuffer;
    sf::Sound enemeyFelledSound;

protected:

    int health;
    int maxHealth;

    float walkSpeed;
    float runSpeed;
    float jumpStrength;
    float gravity;
    float knockbackDistance;

    sf::Vector2f velocity;

    Direction lastDir;
    PlayerState state;

    sf::Vector2f networkTargetPos;
    PlayerState previousNetworkState;

    bool isDead = false;
    bool isJumping = false;
    bool isOnGround = false;
    bool isNormalAttacking = false;
    bool isRolling = false;
    bool isSpecialAttack = false;
    bool isStunned = false;
    bool isDrinking = false;
    bool isSprinting = false;
    bool isHurt = false;
    bool isClimbing = false;

    bool playDeathOnce = false;

    int potionNumber;
    bool healed = false;

    int healAmount;

    bool blockLeft = false;
	bool blockRight = false;

    bool hitboxActive = false;

    bool postAttackCooldown = false;
    float postAttackTimer = 0.f;

    float attackTimer = 0.0f;
    bool invulnerable = false;
    float invulTimer = 0.0f;

    bool isKnockedback = false;
    float knockbackDuration = 0.3f;
    float knockbackTimer = 0.f;

    float stunFrameTimer = 0.f;
    int stunPhase = 0;
    float stunTimer;
    int stunHoldFrame;

    int comboStep = 0;           // 0 = none, 1 = attack1, 2 = attack2, 3 = attack3
    bool comboQueued = false;    // Player pressed attack during the combo
    sf::Clock comboClock;
    float comboWindow = 0.35f;   // Seconds allowed to continue combo

    sf::RectangleShape attackBox;
    sf::RectangleShape playerBox;
    sf::RectangleShape climbingBox;
    sf::RectangleShape specialAttackBox;

    attackData currentAttack;
    InputState lastInput;

    Animation* currentAnimation;

    Healthbar healthbar;

    sf::Texture potionTexture;
    sf::Sprite potionSprite;

    sf::SoundBuffer walkBuffer1;
    sf::Sound walkSound1;

    sf::SoundBuffer walkBuffer2;
    sf::Sound walkSound2;

    sf::Clock walkSoundClock;
    bool playFirstWalkSound = true;
    sf::Time walkInterval = sf::milliseconds(400);


    sf::SoundBuffer jumpBuffer1;
    sf::Sound jumpSound1;

    sf::SoundBuffer jumpBuffer2;
    sf::Sound jumpSound2;

    sf::SoundBuffer hurtBuffer1;
    sf::Sound hurtSound1;

    sf::SoundBuffer hurtBuffer2;
    sf::Sound hurtSound2;

    sf::SoundBuffer hurtBuffer3;
    sf::Sound hurtSound3;

    sf::Clock hurtSoundClock;
    sf::Time hurtInterval = sf::milliseconds(500);


    sf::SoundBuffer attackBuffer1;
    sf::Sound attackSound1;

    sf::SoundBuffer attackBuffer2;
    sf::Sound attackSound2;

    sf::SoundBuffer attackBuffer3;
    sf::Sound attackSound3;

    sf::SoundBuffer parrySoundBuffer;
    sf::Sound parrySound;
    
    sf::SoundBuffer deathBuffer1;
    sf::Sound deathSound1;

    sf::SoundBuffer deathBuffer2;
    sf::Sound deathSound2;

    sf::SoundBuffer shieldImpactBuffer1;
    sf::Sound shieldImpactSound1;

    sf::SoundBuffer shieldImpactBuffer2;
    sf::Sound shieldImpactSound2;

    sf::SoundBuffer swordHit1Buffer;
    sf::Sound swordHitSound1;

    sf::SoundBuffer swordHit2Buffer;
    sf::Sound swordHitSound2;

    sf::SoundBuffer swordHit3Buffer;
    sf::Sound swordHitSound3;

    sf::SoundBuffer bowShotBuffer;
    sf::Sound bowShotSound;

    sf::SoundBuffer bowPullbackBuffer;
    sf::Sound bowPullbackSound;

    sf::SoundBuffer rollSound1Buffer;
    sf::Sound rollSound1;

    sf::SoundBuffer rollSound2Buffer;
    sf::Sound rollSound2;

    sf::SoundBuffer hangBuffer1;
    sf::Sound hangSound1;

    sf::SoundBuffer potionSoundBuffer;
    sf::Sound potionSound;

    bool bowPullback = false;

    virtual void resetCharacter(bool isSinglePlayer) = 0;
    void switchAnimation(Animation* newAnimation);
    void initializeSounds();

    std::vector<DamageNumber> damageNumbers;
};
