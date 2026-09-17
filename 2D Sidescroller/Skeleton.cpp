#include "Skeleton.hpp"

void Skeleton::update(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>& enemies)
{
	skeletonBox.setPosition(currentAnimation->getSprite().getPosition());
	leftLureBox.setPosition(currentAnimation->getSprite().getPosition());
	rightLureBox.setPosition(currentAnimation->getSprite().getPosition());
	
	if (health <= 0)
	{
		if (!playOnce)
		{
			DeathSound.play();
			switchAnimation(Dead.get());

			dead = true;
			std::cout << "DEAD";
			playOnce = true;

		}

		currentAnimation->getSprite().setScale({ lastDir == enemyDirection::Right ? 2.f : -2.f, 2.f });
		currentAnimation->update(dt);

		return;
	}

	if (isHurt && !dead)
	{	
		switchAnimation(Hurt.get());

		this->boneCrackSound.play();

		if (currentAnimation->isFinished())
		{
			Hurt->reset();
			std::cout << "HURT" << std::endl;
			health -= 20;
			isHurt = false;
		}	
	}

	if (lastDir == enemyDirection::Right)
	{
		currentAnimation->getSprite().setScale({ 2,2 });	
	}
	else if (lastDir == enemyDirection::Left)
	{
		currentAnimation->getSprite().setScale({ -2,2 });
	
	}

	currentAnimation->update(dt);
}

void Skeleton::draw(sf::RenderWindow& window)
{

	//currentAnimation->getSprite().setScale({ -2,2 });

	window.draw(currentAnimation->getSprite());


	//window.draw(leftLureBox);

	//window.draw(rightLureBox);

	//window.draw(skeletonBox);
}

sf::FloatRect Skeleton::getBounds()
{
	return skeletonBox.getGlobalBounds();
}

sf::FloatRect Skeleton::getAttackBoxBounds()
{
	return sf::FloatRect();
}

bool Skeleton::isDead()
{
	return dead;
}

int Skeleton::getHealth()
{
	return health;
}

void Skeleton::checkCollision(std::vector<Tile>& tiles)
{
}

void Skeleton::checkForCliff(std::vector<Tile>& tiles)
{
}

void Skeleton::isHurtTrue()
{
	this->isHurt = true;
}

void Skeleton::isHurtFalse()
{
	this->isHurt = false;
}

void Skeleton::hitsShieldTrue()
{
}

Animation* Skeleton::getCurrentEnemyAnimation()
{
	return currentAnimation;
}

void Skeleton::knightDamagedTrue()
{
}

void Skeleton::knightDamagedFalse()
{
}

enemyDirection Skeleton::getDirection()
{
	return lastDir;
}

void Skeleton::setDirection(enemyDirection newDir)
{
	lastDir = newDir;
}

bool Skeleton::setEnemyLeftLure(bool value)
{
	return  this->leftLure = value;
}

bool Skeleton::setEnemyRightLure(bool value)
{
	return this->rightLure = value;
}

sf::FloatRect Skeleton::getEnemyRightLure()
{
	return this->rightLureBox.getGlobalBounds();
}

sf::FloatRect Skeleton::getEnemyLeftLure()
{
	return this->leftLureBox.getGlobalBounds();
}

bool Skeleton::setInitializerBox(bool value)
{
	return false;
}

sf::FloatRect Skeleton::getIninitializerBox()
{
	return sf::FloatRect();
}

void Skeleton::shootArrow(std::vector<Arrow>& arrows, std::vector<Tile>& tiles, float dt)
{

	if (leftLure || rightLure)
	{
		switchAnimation(Shot1.get());

		if (!playSoundOnce)
		{
			this->bowPullbackSound.play();

			playSoundOnce = true;
			
		}

		if (currentAnimation->getCurrentFrame() == 12 && !arrowFired)
		{
			std::cout << "SHOT DONE";

			sf::Vector2f pos = currentAnimation->getSprite().getPosition();

			this->bowShotSound.play();

			sf::Vector2f direction = (lastDir == enemyDirection::Right) ? sf::Vector2f(1, 0) : sf::Vector2f(-1, 0);
			
			float randomGravity = 0;

			randomGravity = changedGravity == 2 ? 500 + std::rand() % 400 : std::rand() % 1000 + 2000;

			arrows.emplace_back(arrowTex, pos, direction, randomGravity);

			arrowFired = true;
		}

		if (currentAnimation->isFinished())
		{
			currentAnimation->reset();

			arrowFired = false;

			playSoundOnce = false;
		}


	}
	else
	{
		switchAnimation(Idle.get());

		
	}

	
	for (auto& arrow : arrows)
	{
		for (auto& tile : tiles)
		{

			if (arrow.getBounds().findIntersection(tile.getBounds()) && tile.isCollidableTile())
			{

				arrow.velocity.x = 0.f;

				arrow.setArrowOnTile(true);
			}
		}

	}

}

