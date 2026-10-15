#pragma once
#include "Animation.hpp"

struct MenuHelper {
    sf::Window& window;
    sf::Sound sound;
    std::optional<sf::Cursor> cursorHand;

    MenuHelper(sf::Window& win, const sf::SoundBuffer& buffer)
        : window(win),
        sound(buffer),
        cursorHand(sf::Cursor::createFromSystem(sf::Cursor::Type::Hand)) {
    }
};