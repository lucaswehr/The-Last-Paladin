#include "Dragon.hpp"

void Dragon::update(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>& enemies)
{

	if (currentAnimation == Special.get() || currentAnimation == Landing.get())
	{
		dragonBox.setPosition({ currentAnimation->getSprite().getPosition().x, currentAnimation->getSprite().getPosition().y - 100.f });
	}
	else
	{
		dragonBox.setPosition(currentAnimation->getSprite().getPosition());
	}


	if (currentAnimation != Flight.get() && currentAnimation != Takeoff.get() && currentAnimation != Special.get() && currentAnimation != Hurt.get())
		helper.setPosition(currentAnimation->getSprite().getPosition());

	if (currentAnimation == Flight.get() || currentAnimation == Takeoff.get() || currentAnimation == Special.get() || currentAnimation == Landing.get())
	{
		if (lastDir == enemyDirection::Left)
			placeHolder.setPosition({ currentAnimation->getSprite().getPosition().x,  currentAnimation->getSprite().getPosition().y + 250 });
		else if (lastDir == enemyDirection::Right)
			placeHolder.setPosition({ currentAnimation->getSprite().getPosition().x,  currentAnimation->getSprite().getPosition().y + 250 });
	}
	else
	{
		placeHolder.setPosition(currentAnimation->getSprite().getPosition());
	}

	leftLureBox.setPosition(currentAnimation->getSprite().getPosition());

	rightLureBox.setPosition(currentAnimation->getSprite().getPosition());

	if (currentAnimation == Flight.get())
	{
		attackBox.setPosition({ currentAnimation->getSprite().getPosition().x, currentAnimation->getSprite().getPosition().y });
	}
	else if (!this->initalizer && !isMoving)
		attackBox.setPosition({ currentAnimation->getSprite().getPosition().x, currentAnimation->getSprite().getPosition().y });

	attackInitializer.setPosition(currentAnimation->getSprite().getPosition());

	if (lastDir == enemyDirection::Left && currentAnimation != Flight.get() || lastDir == enemyDirection::Left && currentAnimation != Special.get())
	{
		placeHolder.setOrigin({ 180,-500 });

		attackInitializer.setOrigin({ 100,-175 });
	}
	else if (lastDir == enemyDirection::Right && currentAnimation != Flight.get() || lastDir == enemyDirection::Right && currentAnimation != Special.get())
	{
		placeHolder.setOrigin({ -160,-500 });

		attackInitializer.setOrigin({ -100,-175 });
	}

	if (currentAnimation == Walk.get() && lastDir == enemyDirection::Right)
	{
		velocity.x = 2.f;

	}
	else if (currentAnimation == Walk.get() && lastDir == enemyDirection::Left)
	{
		velocity.x = -2.f;
	}
	else if (currentAnimation == Idle.get() && lastDir == enemyDirection::Right)
	{
		velocity.x = 0.f;

		placeHolder.setOrigin({ -160,-500 });
	}
	else if ((currentAnimation == Idle.get() && lastDir == enemyDirection::Left))
	{
		velocity.x = 0.f;

		placeHolder.setOrigin({ 180,-500 });
	}

	if (currentAnimation == Flight.get() || currentAnimation == Takeoff.get())
	{
		dragonBox.setPosition({ currentAnimation->getSprite().getPosition().x, currentAnimation->getSprite().getPosition().y - 100.f });

	}

	if (leftLure || rightLure)
	{
		if (playOnce16)
		{
			bossMusic.play();
			fightStarted = true;
			playOnce16 = false;
		}
	}

	if (currentAnimation == Takeoff.get())
	{
		std::cout
			<< "TAKEOFF FRAME: "
			<< currentAnimation->getCurrentFrame()
			<< " Y: "
			<< currentAnimation->getSprite().getPosition().y
			<< " VY: "
			<< velocity.y
			<< std::endl;

		if (currentAnimation->getCurrentFrame() >= 3)
			velocity.y = -7.f;

		if (currentAnimation->isFinished())
		{
			velocity.y = 0.f;
		}
	}

	if (!dead)
	{

		if (currentAnimation != Special.get())
		{
			checkCollision(tiles);

			checkForCliff(tiles);
		}


		attackLogic(tiles,dt);

		landingLogic(tiles);

	}

	if (isHurt)
	{
		if (currentAnimation != Takeoff.get())
		{
			if (currentAnimation != Attack2.get() && currentAnimation != Takeoff.get() && currentAnimation != Flight.get() && currentAnimation != Special.get() && currentAnimation != Landing.get())
				switchAnimation(Hurt.get());

			if (playOnce2)
			{
				health -= 20;

				if (health > 0)
				{
					int random = std::rand() % 3;

					if (random == 0)
						dragonHurtSound.play();
					else if (random == 1)
						dragonHurtSound2.play();
					else if (random == 2)
						dragonHurtSound3.play();
				}

				playOnce2 = false;
			}


			if (currentAnimation == Hurt.get())
			{

				if (health == 100 || health == 40)
					velocity.x = lastDir == enemyDirection::Right ? -7.f : 7.f;
			}
		}

		if (currentAnimation->isFinished())
		{
			velocity.x = 0.f;
			Hurt->reset();
			std::cout << "HURT" << std::endl;
			isHurt = false;
			playOnce2 = true;
		}
	}

	if (health <= 0)
	{

		if (!playOnce4)
		{

			dead = true;
			dragonDeathSound.play();
			playOnce4 = true;
			fightStarted = false;

		}

		switchAnimation(Dead.get());
		gravityLogic(tiles, dt);
		currentAnimation->update(dt);
		dragonBar.update(dt, health, maxHealth, background->getPosition().x + 400.f, background->getPosition().y + 900.f);
		return;

	}

	if (currentAnimation == Special.get())
	{

		if (playOnce14)
		{
			dragonFireSpecialSound.play();
			playOnce14 = false;
		}



		attackBox.setSize({ 35.f, 100.f });

		if (lastDir == enemyDirection::Right)
			attackBox.setOrigin({ -40.f, -80.f });
		else
			attackBox.setOrigin({ 80.f, -80.f });

		isMoving = true;


		sf::Vector2f basePos = currentAnimation->getSprite().getPosition();

		// Base position: always follow the dragon
		attackBox.setPosition(basePos);

		
		if (currentAnimation == Special.get())
		{
			// move downward over time while offset from dragon
			float downwardSpeed = 10.f;
			specialOffset.y += downwardSpeed; 



			if (Special->getCurrentFrame() >= 8)
			{
				specialOffset.y = 0.f;
			}

			if (Special->getCurrentFrame() == 12)
			{
				attackBox.setOrigin({ -80.f, -80.f });
			}

			attackBox.move(specialOffset);
		}
		else
		{
			// Reset offset when not in Special
			specialOffset = { 0.f, 0.f };
		}




	}
	else
	{
		attackBox.setSize({ 100.f, 35.f });
		attackBox.setScale({ 2,2 });
		attackBox.setOrigin({ 50.f, -170.f });
		attackBox.setFillColor(sf::Color(255, 0, 0, 128));

		isMoving = false;
	}

	gravityLogic(tiles, dt);

	if (currentAnimation == Flight.get())
	{
		if (playOnce10)
		{
			dragonFlapSound.play();
			playOnce10 = false;
		}
	}
	if (currentAnimation == Landing.get() || currentAnimation == Special.get())
	{
		playOnce10 = true;
		dragonFlapSound.pause();
	}

	currentAnimation->getSprite().move({ velocity.x, 0 });

	currentAnimation->getSprite().move({ 0, velocity.y });

	currentAnimation->update(dt);

	if (background) {
		dragonBar.update(dt, health, maxHealth, background->getPosition().x + 400.f, background->getPosition().y + 900.f);
		name.setPosition(background->getPosition().x + 1230.f, background->getPosition().y + 950.f);
	}

	
}

void Dragon::draw(sf::RenderWindow& window)
{
	window.draw(currentAnimation->getSprite());

	if (fightStarted)
	{
		name.draw(window);
		dragonBar.draw(window);
	}
	
	//window.draw(placeHolder);

	//window.draw(dragonBox);

//	window.draw(this->rightLureBox);

	//window.draw(this->leftLureBox);

	//window.draw(attackBox);

	//window.draw(attackInitializer);

	//window.draw(helper);
}

sf::FloatRect Dragon::getBounds()
{
	return this->dragonBox.getGlobalBounds();
}

sf::FloatRect Dragon::getAttackBoxBounds()
{
	return this->attackBox.getGlobalBounds();
}

bool Dragon::isDead()
{
	return this->dead;
}

int Dragon::getHealth()
{
	return health;
}

void Dragon::checkCollision(std::vector<Tile>& tiles)
{
	for (auto& tile : tiles)
	{
		if (dragonBox.getGlobalBounds().findIntersection(tile.getBounds()))
		{
			isColliding = true;

		}


	}
		if (isColliding)
		{
			switchAnimation(Idle.get());

			isColliding = false;
		}


		if (dragonIdleClock.getElapsedTime() >= idleInterval && currentAnimation == Idle.get() && lastDir == enemyDirection::Right && !isColliding)
		{
			lastDir = enemyDirection::Left;

			currentAnimation->getSprite().setPosition({ currentAnimation->getSprite().getPosition().x - 20.f, currentAnimation->getSprite().getPosition().y });

			switchAnimation(Walk.get());

			dragonIdleClock.restart();

		}
		else if (dragonIdleClock.getElapsedTime() >= idleInterval && currentAnimation == Idle.get() && lastDir == enemyDirection::Left && !isColliding)
		{
			lastDir = enemyDirection::Right;

			currentAnimation->getSprite().setPosition({ currentAnimation->getSprite().getPosition().x + 20.f, currentAnimation->getSprite().getPosition().y });

			switchAnimation(Walk.get());

			dragonIdleClock.restart();

		}
	
	
}

void Dragon::checkForCliff(std::vector<Tile>& tiles)
{

	isColliding = false;

	
		

	for (const auto& tile : tiles) {
		
		if (placeHolder.getGlobalBounds().findIntersection(tile.getBounds()))
		{
			isColliding = true;

			switchAnimation(Walk.get());

			///std::cout << "TRUE";
		}
		

	}

	if (!isColliding)
	{
		switchAnimation(Idle.get());
	}

	if (!isColliding && lastDir == enemyDirection::Right) {
		

		if (dragonIdleClock.getElapsedTime() >= idleInterval && currentAnimation == Idle.get() && lastDir == enemyDirection::Right && !isColliding)
		{
			lastDir = enemyDirection::Left;

			currentAnimation->getSprite().setPosition({ currentAnimation->getSprite().getPosition().x - 20.f, currentAnimation->getSprite().getPosition().y });

			switchAnimation(Walk.get());

			dragonIdleClock.restart();

		}

	}
	else if (!isColliding && lastDir == enemyDirection::Left)
	{

	    if (dragonIdleClock.getElapsedTime() >= idleInterval && currentAnimation == Idle.get() && lastDir == enemyDirection::Left && !isColliding)
	    {
		   lastDir = enemyDirection::Right;

		   currentAnimation->getSprite().setPosition({ currentAnimation->getSprite().getPosition().x + 20.f, currentAnimation->getSprite().getPosition().y });

		   switchAnimation(Walk.get());

		   dragonIdleClock.restart();

	    }

	}

}

void Dragon::isHurtTrue()
{
	isHurt = true;
}

void Dragon::isHurtFalse()
{
	isHurt = false;
}

void Dragon::hitsShieldTrue()
{

}

Animation* Dragon::getCurrentEnemyAnimation()
{
	return currentAnimation;
}

void Dragon::knightDamagedTrue()
{
	knightDamaged = true;
}

void Dragon::knightDamagedFalse()
{
	knightDamaged = false;
}

enemyDirection Dragon::getDirection()
{
	return lastDir;
}

void Dragon::setDirection(enemyDirection newDir)
{
   lastDir = newDir;
}

bool Dragon::setEnemyLeftLure(bool value)
{
	return leftLure = value;
}

bool Dragon::setEnemyRightLure(bool value)
{
	return rightLure = value;
}

sf::FloatRect Dragon::getEnemyRightLure()
{
	return rightLureBox.getGlobalBounds();
}

sf::FloatRect Dragon::getEnemyLeftLure()
{
	return leftLureBox.getGlobalBounds();
}

bool Dragon::setInitializerBox(bool value)
{
	return this->initalizer = value;
}

sf::FloatRect Dragon::getIninitializerBox()
{
	return this->attackInitializer.getGlobalBounds();
}

void Dragon::attackLogic(std::vector<Tile>& tiles, float dt)
{
	if (this->initalizer)
	{
		switchAnimation(Attack2.get());

		if (playOnce9 )
		{
			if (currentAnimation->getCurrentFrame() > 3)
			{
				dragonFlameSound.play();
				playOnce9 = false;
			}
		}
		
		if (Attack2->getCurrentFrame() >= 8)
		{
			attackBox.move({ 0,0 });
		}
		else
		{
			if (lastDir == enemyDirection::Left)
			attackBox.move({ -5,0 });
		else
			attackBox.move({ 5,0 });

		}

		velocity.x = 0.f;

		if (currentAnimation->isFinished())
		{
			playOnce9 = true;

			currentAnimation->reset();

			counter++;
		}


	}


	if (counter >= 2)
	{
	
		if (playOnce)
			dragonFlightClock.restart();

		if (leftLure || rightLure)
			dragonFlightClock.start();

		playOnce = false;

		std::cout << dragonFlightClock.getElapsedTime().asMilliseconds() << std::endl;

		if (random == 3)
		{
			flightAttackLogic(tiles,dt);

		}
		else
		{
			if (this->dragonFlightClock.getElapsedTime() > flightInterval)
			{
				if (currentAnimation != Takeoff.get())
				{
					if (playOnce11 == true)
					{
                        dragonTakeoffSound.play();
						playOnce11 = false;
					}
					
					switchAnimation(Takeoff.get());
				}
					

				if (currentAnimation->isFinished() && currentAnimation == Takeoff.get())
				{
					switchAnimation(Flight.get());

					if (playOnce7)
					{
						currentAnimation->setFrameDuration(0.1f);

						playOnce7 = false;
					}

				}
				if (currentAnimation->getCurrentFrame() == 11 && currentAnimation == Flight.get())
				{
					switchAnimation(Special.get());

				}

			}

		}
		

	}


	if (!leftLure || !rightLure)
	{
		dragonFlightClock.stop();
	}

	if (currentAnimation == Special.get())
	{
		if (lastDir == enemyDirection::Left)
			velocity.x = -7.f;
		else if (lastDir == enemyDirection::Right)
			velocity.x = 7.f;

	}

}

void Dragon::landingLogic(std::vector<Tile>& tiles)
{
	bool isColliding = false;

	if (currentAnimation == Special.get())
	{

		for (auto& tile : tiles)
		{

			if (placeHolder.getGlobalBounds().findIntersection(tile.getBounds()) && !isLanding)
			{
				isColliding = true;

				switchAnimation(Special.get());
			}


			if (!isColliding)
			{
				switchAnimation(Landing.get());

			}
		}
	}

	if (currentAnimation == Landing.get())
	{
		velocity.x = 0.f;

		//velocity.y = 5.8f;

		isLanding = true;

		if (playOnce15)
		{
			dragonFireSpecialSound.stop();
			dragonLandSound.play();
			playOnce15 = false;
		}

		if (currentAnimation->isFinished())
		{
			//velocity.y = 0.f;
			isLanding = false;
			switchAnimation(Idle.get());
			counter = 0;
			leftLure = false;
			rightLure = false;
			initalizer = false;
			playOnce = true;
			playOnce11 = true;
			playOnce14 = true;
			playOnce17 = true;
			playOnce15 = true;
			playOnce5 = true;
			playOnce6 = true;
			playOnce12 = true;
			Takeoff->reset();
			Flight->reset();	
			Landing->reset();
			Special->reset();
			Attack2->reset();			
			random = std::rand() % 4;			
		}
	}
}

void Dragon::gravityLogic(std::vector<Tile>& tiles, float dt)
{

	bool onGround = false;
	sf::FloatRect intersection;

	if (currentAnimation != Flight.get() && currentAnimation != Special.get() && currentAnimation != Takeoff.get() && currentAnimation != Hurt.get())
	{
		
		velocity.y += gravity * dt;
	}
	
	for (auto& tile : tiles)
	{
		if (auto intersection = dragonBox.getGlobalBounds().findIntersection(tile.getBounds()))
		{
			if (velocity.y > 0.f)
			{
				// Place dragon on top of the tile
				currentAnimation->getSprite().setPosition(
					{ currentAnimation->getSprite().getPosition().x,
					currentAnimation->getSprite().getPosition().y - intersection->size.y}
				);

				velocity.y = 0.f;
				onGround = true;
			}

		}

	}

	if (onGround)
	{
		velocity.y = 0.f;
	}

	
}

void Dragon::flightAttackLogic(std::vector<Tile>& tiles, float dt)
{

	if (!isLanding)
	{
		if (this->dragonFlightClock.getElapsedTime() > flightInterval)
		{
			if (currentAnimation != Takeoff.get())
				switchAnimation(Takeoff.get());

			if (currentAnimation->isFinished() && currentAnimation == Takeoff.get())
			{
				switchAnimation(Flight.get());

				if (playOnce8)
				{
					currentAnimation->setFrameDuration(0.04f);

					playOnce8 = false;
				}
			}
			velocity.y = -8.f;

			velocity.x = lastDir == enemyDirection::Right ? 12.f : -12.f;

		}


	}

	if (playOnce5)
	{
		std::cout << "RESTARTING" << std::endl;
		dragonWaitClock.restart();
		playOnce5 = false;
	}
	


	//std::cout << dragonWaitClock.getElapsedTime().asMilliseconds() << std::endl;

	if (this->dragonWaitClock.getElapsedTime() > waitInterval)
	{
		if (!teleported)
		{
			std::cout << "TELEPORTED" << std::endl;
			currentAnimation->getSprite().setPosition({
			helper.getGlobalBounds().getCenter().x + 3000.f, helper.getGlobalBounds().getCenter().y - 270.f});

			teleported = true;   
		}
			
		velocity.y = 0.f;
		velocity.x =  -27.f;
		currentAnimation->getSprite().setScale({ -1.7,1.7 });

		if (playOnce6)
		{
			dragonWaitClock2.restart();		
			playOnce6 = false;
		}

		if (playOnce13)
		{
			if (this->dragonWaitClock.getElapsedTime() > waitInterval + sf::milliseconds(300))
			{
				dragonWhooshSound.play();
				playOnce13 = false;
			}

		}

		if (this->dragonWaitClock2.getElapsedTime() > waitInterval2 )
		{
			velocity.x = 27.f;
			currentAnimation->getSprite().setScale({ 1.7,1.7 });

			if (playOnce12 && this->dragonWaitClock2.getElapsedTime() > waitInterval2 + sf::milliseconds(600))
			{
				dragonWhooshSound.play();
				playOnce12 = false;

			}

		}

		if (dragonWaitClock2.getElapsedTime() > waitInterval2 + sf::milliseconds(2500))
		{

			velocity.y = -8.f;
			velocity.x = 20.f;

		}

		if (dragonWaitClock2.getElapsedTime() > waitInterval2 + sf::milliseconds(5000))
		{

			if (!teleported2)
			{
				currentAnimation->getSprite().setPosition({
				helper.getGlobalBounds().getCenter().x + 3000.f, helper.getGlobalBounds().getCenter().y - 1700.f});

				teleported2 = true;
			}

			velocity.y = 7.f;
			velocity.x = -20.f;


			currentAnimation->getSprite().setScale({ -1.7,1.7 });
			

			bool isColliding = false;

				for (auto& tile : tiles)
				{

					if (placeHolder.getGlobalBounds().findIntersection(tile.getBounds()))
					{
						isColliding = true;

					}


					if (isColliding)
					{
						switchAnimation(Landing.get());
					}

				}

			

			if (currentAnimation == Landing.get())
			{
				//velocity.x = 0.f;

				//velocity.y = 5.8f;

				velocity.y += (gravity * 5 ) * dt;

				if (playOnce17)
				{
					dragonFireSpecialSound.stop();
					dragonLandSound.play();
					playOnce17 = false;
				}

				isLanding = true;

				if (currentAnimation->isFinished())
				{
					isLanding = false;
					counter = 0;
					leftLure = false;
					rightLure = false;
					initalizer = false;
					playOnce = true;
					teleported = false;
					teleported2 = false;
					playOnce5 = true;
					playOnce6 = true;
					playOnce12 = true;
					playOnce13 = true;
					playOnce14 = true;
					playOnce15 = true;
					playOnce17 = true;
					dragonFlightClock.restart();
					dragonWaitClock2.restart(); 
					switchAnimation(Idle.get());
					random = std::rand() % 4;
					Takeoff->reset();
					Flight->reset();
					Landing->reset();
					Special->reset();					
					Attack2->reset();
					dragonLandSound.play();
				}
			}


		}
	}


}

void Dragon::setBackgroundShape(sf::RectangleShape* bg)
{
	background = bg;
}
