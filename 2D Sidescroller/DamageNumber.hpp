#pragma once
#include "Animation.hpp"

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