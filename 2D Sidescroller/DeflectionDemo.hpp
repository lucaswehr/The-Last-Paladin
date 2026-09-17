#pragma once
#include "Animation.hpp"

class DeflectionDemo
{
public:
    DeflectionDemo(Animation& samuraiSpecial, Animation& knightIdle, Animation& knightSpecial, Animation& samuraiDead, const sf::Texture& arrowTex) :
        arrow(arrowTex),
        samuraiSpecial(samuraiSpecial),
        knightIdle(knightIdle),
        knightSpecial(knightSpecial),
        samuraiDead(samuraiDead)
    {
        setup();

        knight.currentAnimation->getSprite().setScale({ -2.f, 2.f });
        arrow.sprite.setScale({ 2,2 });
    };

    void update(float dt);
    void draw(sf::RenderWindow& window);
    void reset();

private:
    void setup();
    void spawnArrow();
    void updateArrow(float dt);
    void processDeflection();
    void processArrowHit();

    enum class DeflectionDemoState
    {
        SamuraiShooting,
        ArrowFlying,
        ArrowDeflected,
        SamuraiDead,
        Reset
    };

    struct DemoArrow
    {
        sf::Sprite sprite;
        sf::RectangleShape hitbox;

        sf::Vector2f velocity;

        float gravity = 400.f;

        bool active = false;
        bool reflected = false;

        DemoArrow(const sf::Texture& arrowTex) :
            sprite(arrowTex)
        {
        }

    };

    struct DemoCharacter
    {
        Animation* currentAnimation = nullptr;

        sf::RectangleShape bodyBox;
        sf::RectangleShape deflectionBox;

        bool dead = false;
    };

    Animation samuraiSpecial;
    Animation knightIdle;
    Animation knightSpecial;
    Animation samuraiDead;

    DemoCharacter samurai;
    DemoCharacter knight;
    DemoArrow arrow;

    DeflectionDemoState state;

    float timer = 0.f;
};