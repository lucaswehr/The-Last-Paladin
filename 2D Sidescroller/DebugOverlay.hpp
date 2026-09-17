 #pragma once
#include "Animation.hpp"
using namespace std;

class DebugOverlay {

public:

	DebugOverlay(sf::Font& font) : text(font)
	{
		text.setFont(font);
		text.setCharacterSize(50);
		text.setFillColor(sf::Color::White);
        text.setOutlineThickness(5.f);
		text.setPosition({ 10.f, 10.f });
	}

    void clear()
    {
        debugString.clear();
    }

    void addLine(const std::string& name, const std::string& value)
    {
        debugString += name + ": " + value + "\n\n";
    }

    void addBool(const std::string& name, bool value)
    {
        debugString += name + ": ";
        debugString += value ? "true" : "false";
        debugString += "\n\n";
    }

    void draw(sf::RenderWindow& window)
    {
        text.setString(debugString);
        window.draw(text);
    }

private:
	sf::Text text;
	string debugString;
};
