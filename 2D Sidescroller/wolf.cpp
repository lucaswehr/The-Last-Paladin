#include "wolf.hpp"

void wolf::draw(sf::RenderWindow& window)
{
    //window.draw(wolfBox);
	//window.draw(wolfAttackBox);

	window.draw(currentAnimation2->getSprite());

	if (!isDead() && health < maxHealth)
	   healthbar.draw(window);

	//window.draw(rightLureBox);
	//window.draw(leftLureBox);
}

void wolf::update(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>& enemies)
{
	wolfBox.setPosition(currentAnimation2->getSprite().getPosition());
	rightLureBox.setPosition({ currentAnimation2->getSprite().getPosition() });
	leftLureBox.setPosition({ currentAnimation2->getSprite().getPosition() });

	healthbar.update(dt, health, maxHealth, wolfBox.getPosition().x - 40, wolfBox.getPosition().y - 150);

	if (health <= 0)
	{
		if (!playAnimationOnce)
		{

			if (health == 0)
			{
				deathSound.play();
			}

			switchAnimation2(death.get());
	
			currentAnimation2->getSprite().setScale({ lastDir == enemyDirection::Right ? 1.5f : -1.5f, 1.5f });

			velocity.x = 0.f;

			playAnimationOnce = true;

			dead = true;

		}

		currentAnimation2->update(dt); // Let the animation play
		return;		
	}
	

		if (lastDir == enemyDirection::Left)
		{
			wolfAttackBox.setPosition({ currentAnimation2->getSprite().getPosition().x + 50.f * -1, currentAnimation2->getSprite().getPosition().y });
		}
		else
		{
			wolfAttackBox.setPosition({ currentAnimation2->getSprite().getPosition().x + 50.f, currentAnimation2->getSprite().getPosition().y });
		}

		if (attackCooldown > 0.f)
		{
			attackCooldown -= dt;
		}

		//std::cout << velocity.x << std::endl;

		if ((knightDamaged || this->wolfHitsShield) && !isKnockedBack)
		{

			if (attackCooldown <= 0.f)
			{
				int randomNumber = std::rand() % 3 + 1;

				if (randomNumber == 1)
				{			
					switchAnimation2(attack1.get());
				}
				else if (randomNumber == 2)
				{				
					switchAnimation2(attack2.get());
				}
				else
				{				
					switchAnimation2(attack3.get());
				}

				currentAnimation2->reset();

				velocity.x = 0.f;

				attackCooldown = attackDelay;
	
				if (wolfHitsShield)
				pendingShieldKnockback = true;  // Defer knockback until animation finishes

				isAttacking = true;			
			}
			
			if (pendingShieldKnockback && currentAnimation2->isFinished()) {

				wolfHitsShield = true;
				isKnockedBack = true;
				knockbackTimer = 0.5f;
				pendingShieldKnockback = false;
				isAttacking = false;

				std::cout << "WOLF KNOCKED BACK AFTER ATTACK ANIMATION" << std::endl;
			}

			if (lastDir == enemyDirection::Right)
			{
				currentAnimation2->getSprite().setScale({ 1.5,1.5 });
			}
			else
			{
				currentAnimation2->getSprite().setScale({ -1.5,1.5 });
			}
		}
		else
		{
			switchAnimation2(walk.get());

			if (lastDir == enemyDirection::Left)
			{
				velocity.x = -3.f;

			}
			else
			{
				velocity.x = 3.f;
			}
		}

		
		/*if (isAttacking && currentAnimation2->isFinished())
		{
			isAttacking = false;
			std::cout << "WOLF DONE ATTACKING (no shield knockback)" << std::endl;
		}*/
		

		if (isKnockedBack && lastDir == enemyDirection::Left)
		{
			
			velocity.x = 5.f;

			knockbackTimer -= dt;
			if (knockbackTimer <= 0.f)
			{
				isKnockedBack = false;
				velocity.x = -3.f;
				wolfHurt = false;
				playOnce = false;
				wolfHitsShield = false;
				
			}
		}
		else if (isKnockedBack && lastDir == enemyDirection::Right)
		{
			
				velocity.x = -5.f;
		
			knockbackTimer -= dt;
			if (knockbackTimer <= 0.f)
			{
				isKnockedBack = false;
				velocity.x = 3.f;
				wolfHurt = false;
				playOnce = false;
				wolfHitsShield = false;
			}

		}

		for (auto& wolf : enemies)
		{
			if (wolfHurt)
			{
				switchAnimation2(hurt.get());

				currentAnimation2->getSprite().setScale({ lastDir == enemyDirection::Right ? 1.5f : -1.5f, 1.5f });

				//velocity.x = 0.f;
			}


			if (wolfHurt && !playOnce)
			{

				int randomNumber = std::rand() % 2 + 1;

				if (randomNumber == 1)
				{
					hurtSound1.play();
				}
				else
				{
                   hurtSound2.play();
				}

				playOnce = true;


				this->health -= 20;
				
			}
		}


		checkCollision(tiles);
		checkForCliff(tiles);

		/*if (isAttacking)
		std::cout << "TRUE" << std::endl;
		else
			std::cout << "FALSE" << std::endl;*/

		
		if (knightDamaged)
		{
			isAttacking = false;
		}

		if (rightLure && !isKnockedBack)
		{
		//	velocity.x = 3.f;
			currentAnimation2->getSprite().setScale({ 1.5,1.5 });
		}

		if (leftLure && !isKnockedBack)
		{
			currentAnimation2->getSprite().setScale({ -1.5,1.5 });
			//velocity.x = -3.f;
		}

		if (lastDir == enemyDirection::Left)
			currentAnimation2->getSprite().setScale({ -1.5,1.5 });

		if (!isAttacking)
			currentAnimation2->getSprite().move({ velocity.x, 0.f });
		else
			currentAnimation2->getSprite().move({ 0.f, 0.f });

		currentAnimation2->update(dt);
}

void wolf::checkCollision(std::vector<Tile>& tiles)
{
	bool collisionDetected = false;

	for (const auto& tile : tiles)
	{
		if (!tile.isCollidableTile()) continue;

		if (wolfBox.getGlobalBounds().findIntersection(tile.getBounds()))
		{
			if (wolfBox.getGlobalBounds().position.x + wolfBox.getGlobalBounds().size.x - 50 <= tile.getBounds().position.x + 50.f) //collision from right side
			{
				//std::cout << "WOLF COLLISION FROM RIGHT" << std::endl;

				currentAnimation2->getSprite().setScale({ -1.5,1.5 });

				collisionDetected = true;

				lastDir = enemyDirection::Left;

				break;
			}
			else
			{
				//std::cout << "WOLF COLLISION FROM LEFT" << std::endl;

				currentAnimation2->getSprite().setScale({ 1.5,1.5 });

				collisionDetected = true;

				lastDir = enemyDirection::Right;

				break;
			}
			
		}
	}

	if (collisionDetected)
	{
		velocity.x = -velocity.x; // Flip direction
		//std::cout << "WOLF COLLISION" << std::endl;
	}
}

void wolf::checkForCliff(std::vector<Tile>& tiles)
{

	sf::Sprite& s = currentAnimation2->getSprite();
	sf::Vector2f position = s.getPosition();

	float offsetX = (velocity.x > 0) ? 20.f : -20.f;  // look ahead to the right or left
	float checkX = position.x + offsetX;
	float checkY = position.y + 10.f; // slightly below the sprite's feet

	bool groundFound = false;

	for (const auto& tile : tiles) {
		if (!tile.isCollidableTile()) continue;

		sf::FloatRect tileBounds = tile.getBounds();

		// Does the tile contain the ground check point?
		if (tileBounds.contains({ checkX, checkY })) {
			groundFound = true;

			break;
		}

	}

	if (!groundFound && lastDir == enemyDirection::Right) {
		// No ground below — turn around
		velocity.x = -velocity.x;

		lastDir = enemyDirection::Left;

		float baseScale = 1.5f;
		s.setScale({ (velocity.x > 0) ? baseScale : -baseScale, baseScale });
	}
	else if (!groundFound && lastDir == enemyDirection::Left)
	{
		velocity.x = -velocity.x;

		lastDir = enemyDirection::Right;

		float baseScale = 1.5f;
		s.setScale({ (velocity.x > 0) ? baseScale : -baseScale, baseScale });


	}

}

sf::FloatRect wolf::getBounds()
{
	return wolfBox.getGlobalBounds();
}

sf::FloatRect wolf::getAttackBoxBounds()
{
	return this->wolfAttackBox.getGlobalBounds();
}

void wolf::setWolfVelocity(float x)
{



}

void wolf::knightDamagedTrue()
{
	this->knightDamaged = true;

	this->knockbackTimer = 0.3f;
}

void wolf::knightDamagedFalse()
{
	this->knightDamaged = false;

}

bool wolf::isDead()
{
	return dead;
}

void wolf::isHurtTrue()
{
	this->wolfHurt = true;

	this->isKnockedBack = true;

	this->knockbackTimer = 0.3f;
}

void wolf::isHurtFalse()
{
	this->wolfHurt = false;
}

enemyDirection wolf::getDirection()
{
	return lastDir;
}

void wolf::setDirection(enemyDirection newDir)
{
	this->lastDir = newDir;
}


void wolf::hitsShieldTrue()
{

	wolfHitsShield = true;
}

Animation* wolf::getCurrentEnemyAnimation()
{
	return this->currentAnimation2;
}

bool wolf::setEnemyRightLure(bool value)
{
	return this->rightLure = value;
}

bool wolf::setEnemyLeftLure(bool value)
{
	return this->leftLure = value;
}



int wolf::getHealth()
{
	return this->health;
}

sf::FloatRect wolf::getEnemyRightLure()
{
	return this->rightLureBox.getGlobalBounds();
}

sf::FloatRect wolf::getEnemyLeftLure()
{
	return this->leftLureBox.getGlobalBounds();
}

bool wolf::setInitializerBox(bool value)
{
	return false;
}

sf::FloatRect wolf::getIninitializerBox()
{
	return sf::FloatRect();
}


