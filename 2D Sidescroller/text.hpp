#pragma once
#include "Animation.hpp"
#include "MenuHelper.hpp"

class Text 
{
public:

	Text(const std::string& fontPath, const std::string& title, float x, float y, sf::Color color, int size);

	void draw(sf::RenderWindow& window);

	sf::FloatRect getBounds();

	sf::Text& getText();

	void setPosition(float x, float y);

	void setString(const std::string& str);

	void update(const sf::Vector2f& mousePos, MenuHelper& helper);

private:

	sf::Font font;
	sf::Text text;

	bool isHovered = false;
};
