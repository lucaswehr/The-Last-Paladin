#include "Enemy.hpp"

DamageNumber Enemy::createDamageNumberText(sf::Font& standardFont, std::string message, int scaleX, int scaleY, sf::Color color, sf::Vector2f position)
{
	DamageNumber damageNumber(standardFont);
	damageNumber.text.setString(message);
	damageNumber.text.setScale({ (float)scaleX, (float)scaleY });

	sf::FloatRect textBounds =
		damageNumber.text.getGlobalBounds();

	damageNumber.text.setPosition({
		position.x - textBounds.size.x / 2.f,
		position.y - textBounds.size.y - 50.f
		});

	damageNumber.text.setFillColor(color);
	damageNumber.text.setOutlineColor(sf::Color::Black);
	damageNumber.text.setOutlineThickness(2.f);
	damageNumber.velocity = { 0.f, -50.f };
	damageNumber.lifetime = 0.8f;

	return damageNumber;
}

void Enemy::updateDamageText(float dt)
{
	for (auto it = damageNumbers.begin(); it != damageNumbers.end(); )
	{
		it->lifetime -= dt;

		// Move upward
		it->text.move(it->velocity * dt);

		// Fade fill
		float alpha = (it->lifetime / it->maxLifetime) * 255.f;

		sf::Color fillColor = it->text.getFillColor();
		fillColor.a = static_cast<std::uint8_t>(alpha);
		it->text.setFillColor(fillColor);

		// Fade outline
		sf::Color outlineColor = it->text.getOutlineColor();
		outlineColor.a = static_cast<std::uint8_t>(alpha);
		it->text.setOutlineColor(outlineColor);

		if (it->lifetime <= 0.f)
		{
			it = damageNumbers.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Enemy::initializeSounds()
{
	this->swordHitFleshBuffer1.loadFromFile("Sounds/swordHitFlesh1.mp3");
	this->swordHitFleshSound1.setBuffer(swordHitFleshBuffer1);

	this->swordHitFleshBuffer2.loadFromFile("Sounds/swordHitFlesh2.mp3");
	this->swordHitFleshSound2.setBuffer(swordHitFleshBuffer2);

	swordHitFleshSound1.setVolume(50.f);
	swordHitFleshSound2.setVolume(50.f);
}
