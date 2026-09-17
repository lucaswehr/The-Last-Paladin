#pragma once
#include "Player.hpp"
#include "CharacterType.h"
#include "Knight.hpp"
#include "Samurai.hpp"
#include "AssetManager.hpp"

using namespace std;

class PlayerFactory {

public:
	unique_ptr<Player> static create(CharacterType type, float xPos, float yPos, AssetManager& assets, std::string& fonttext, sf::Color& color, sf::Font& standardFont)
	{
		switch (type)
		{
			case CharacterType::Knight:
				return make_unique<Knight>(xPos, yPos, assets.getKnightTextures(), fonttext, color, standardFont);

			case CharacterType::Samurai:
				return make_unique<Samurai>(xPos, yPos, assets.getSamuraiTextures(), color, standardFont);
			default:
				return nullptr;
		}
		
	}
};
