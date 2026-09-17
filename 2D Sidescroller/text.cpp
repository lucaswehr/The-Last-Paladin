#include "text.hpp"

Text::Text(const std::string& fontPath, const std::string& title, float x, float y, sf::Color color, int size) : text(font)
{
	if (!font.openFromFile(fontPath))
	{
		std::cout << "Error in text constructor" << std::endl;
	}

	text.setString(title);
	text.setCharacterSize(size);
	text.setFillColor(color);
	text.setPosition(sf::Vector2f(x, y));
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness({ 5 });
}

void Text::draw(sf::RenderWindow& window)
{
	window.draw(text);
}

sf::FloatRect Text::getBounds()
{
	return text.getGlobalBounds();

}

sf::Text& Text::getText()
{
	return this->text;
}

void Text::setPosition(float x, float y)
{
	text.setPosition({ x,y });
}

void Text::setString(const std::string& str) {
	text.setString(str);  // forward to sf::Text
}
