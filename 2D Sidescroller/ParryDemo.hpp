#pragma once
#include "Animation.hpp"

class ParryDemo
{
public:
    ParryDemo(Animation& knightAttack, Animation& knightIdle, Animation& knightSpecial, Animation& knightStunned) :      
        knightAttack(knightAttack),
        attackingKnightIdle(knightIdle),
        knightSpecial(knightSpecial),
        knightStunned(knightStunned),
        parryingKnightIdle(knightIdle)
    {
        attackingKnight.bodyBox.setSize({ 32.f, 48.f });
        attackingKnight.bodyBox.setScale({ 2,2.5 });
        attackingKnight.bodyBox.setOrigin({ 17.f, -20.f });
        attackingKnight.bodyBox.setFillColor(sf::Color(255, 0, 0, 128));

        attackingKnight.attackBox.setSize({ 32.f, 48.f });
        attackingKnight.attackBox.setScale({ 4,2.5 });
        attackingKnight.attackBox.setFillColor(sf::Color(255, 0, 0, 128));
        attackingKnight.attackBox.setOrigin({ 17.f, -20.f });

        parryingKnight.bodyBox.setSize({ 32.f, 48.f });
        parryingKnight.bodyBox.setScale({ 2,2.5 });
        parryingKnight.bodyBox.setOrigin({ 17.f, -20.f });
        parryingKnight.bodyBox.setFillColor(sf::Color(255, 0, 0, 128));

        parryingKnight.parryBox.setSize({ 35.f, 40.f });
        parryingKnight.parryBox.setScale({ 2,2.5 });
        parryingKnight.parryBox.setOrigin({ -10.f, -20.f });
        parryingKnight.parryBox.setFillColor(sf::Color(255, 0, 0, 128));

        setup();

        parryingKnight.currentAnimation->getSprite().setScale({ -2.f, 2.f });
    };

    void update(float dt);
    void draw(sf::RenderWindow& window);
    void reset();

private:
    void setup();
    void startFight();

    enum class ParryDemoState
    {
        Setup,
        Idle,
        Attacking,
        Parried,
        Reset
    };

    struct DemoCharacter
    {
        Animation* currentAnimation = nullptr;

        sf::RectangleShape attackBox;
        sf::RectangleShape parryBox;
        sf::RectangleShape bodyBox;

        bool isStunned = false;        

        int stunPhase = 0;
        float stunFrameTimer = 0.f;
        float stunTimer = 0.f;

        int stunHoldFrame = 1;
    };

    Animation knightAttack;
    Animation attackingKnightIdle;
    Animation parryingKnightIdle;
    Animation knightSpecial;
    Animation knightStunned;

    DemoCharacter attackingKnight;
    DemoCharacter parryingKnight;
   
    ParryDemoState state;

    float timer = 0.f;
    float idleTimer = 0.f;

    void updateStun(float dt);
};
