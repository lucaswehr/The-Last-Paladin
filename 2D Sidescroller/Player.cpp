#include "Player.hpp"
#include "Client.hpp"

void Player::updateStun(float dt)
{
	if (!isStunned) return;

	stunTimer -= dt;
	stunFrameTimer += dt;

	float frameDuration = 0.08f;

	// Phase 0: go to frame 2
	if (isHurt)
	{
		isStunned = false;
		state = PlayerState::Hurt;
		//switchAnimationByState(PlayerState::Hurt);
	}
	else if (stunPhase == 0)
	{
		if (stunFrameTimer >= frameDuration)
		{
			stunFrameTimer = 0.f;

			if (currentAnimation->getCurrentFrame() < stunHoldFrame)
			{
				currentAnimation->nextFrame();
			}
			else
			{
				stunPhase = 1;
			}
		}
	}
	// Phase 1: hold frame 2
	else if (stunPhase == 1)
	{
		currentAnimation->setFrame(stunHoldFrame); // HOLD frame 2

		if (stunTimer <= 0.f)
		{
			stunPhase = 2;
			stunFrameTimer = 0.f;
		}
	}
	// Phase 2: reverse back to frame 1
	else if (stunPhase == 2)
	{
		if (stunFrameTimer >= frameDuration)
		{
			stunFrameTimer = 0.f;

			if (currentAnimation->getCurrentFrame() > 0)
			{
				currentAnimation->setFrame(currentAnimation->getCurrentFrame() - 1);
			}
			else
			{
				isStunned = false;
				isHurt = false;
				currentAnimation->reset();
				state = PlayerState::Idle;
				switchAnimationByState(state);
			}
		}
	}
}

void Player::processProjectileCollisions(Player& victim, int attackerID, int victimID, Client& client, sf::Font& standardFont)
{
	return;
}

void Player::processProjectileDeflection(Player& victim, Client& client, int attackerID, int victimID, sf::Font& standardFont)
{
	return;
}

bool Player::isDeflectionWindow()
{
	return false;
}

void Player::playDeathSound()
{
	int random = std::rand() % 2;
	(random == 0 ? deathSound1 : deathSound2).play();
}

void Player::playBowPullBackSound()
{
	if (!bowPullback && currentAnimation->getCurrentFrame() == 3)
	{
		this->bowPullbackSound.play();
		bowPullback = true;
	}
}

void Player::playBowShotSound()
{
	bowShotSound.play();
}

void Player::playPotionSound()
{
	potionSound.play();
}

void Player::playOtherSounds(PlayerState state)
{
	switch (state)
	{
		case PlayerState::Jump:
			playJumpSound();
			break;
		case PlayerState::Roll:
			playRollSound();
			break;
		case PlayerState::Climb:
			playLedgeSound();
			break;
	}
}

void Player::playJumpSound()
{
	int randomNumber = std::rand() % 2 + 1;

	if (randomNumber == 1)
		jumpSound1.play();
	else
		jumpSound2.play();
}

void Player::playRollSound()
{
	int randomNumber = std::rand() % 2 + 1;

	if (randomNumber == 1)
		rollSound1.play();
	else
		rollSound2.play();
}

void Player::playLedgeSound()
{
	hangSound1.play();
}

bool Player::isBowPulledBack()
{
	return bowPullback;
}

void Player::clearBowPullbackBool()
{
	this->bowPullback = false;
}

void Player::initializeSounds()
{
	this->shieldImpactBuffer1.loadFromFile("Sounds/bing1.wav");
	this->shieldImpactSound1.setBuffer(shieldImpactBuffer1);

	this->shieldImpactBuffer2.loadFromFile("Sounds/metal3.wav");
	this->shieldImpactSound2.setBuffer(shieldImpactBuffer2);

	this->bowShotBuffer.loadFromFile("Sounds/BowShotSound.mp3");
	this->bowShotSound.setBuffer(bowShotBuffer);

	this->bowPullbackBuffer.loadFromFile("Sounds/BowPullback.mp3");
	this->bowPullbackSound.setBuffer(bowPullbackBuffer);

	this->hangBuffer1.loadFromFile("Sounds/grass/0.ogg");
	this->hangSound1.setBuffer(hangBuffer1);
}

PlayerState Player::getState()
{
	return state;
}

Direction Player::getDirection()
{
	return lastDir;
}

sf::RectangleShape Player::getPlayerBox()
{
	return playerBox;
}

sf::RectangleShape Player::getAttackBox()
{
	return attackBox;
}

bool Player::isHitboxActive()
{
	return hitboxActive;
}

bool Player::getStunned()
{
	return isStunned;
}

attackData Player::getCurrentAttack()
{
	return currentAttack;
}

bool Player::isHurtBool()
{
	return isHurt;
}

void Player::switchAnimation(Animation* newAnimation)
{
	if (currentAnimation != newAnimation) {
		sf::Vector2f pos = currentAnimation->getSprite().getPosition();
		currentAnimation = newAnimation;
		currentAnimation->getSprite().setPosition(pos);
	}

	currentAnimation->setDirection(lastDir);
}

void Player::setNetworkState(float x, float y, PlayerState netState, Direction netDir)
{
	currentAnimation->getSprite().setScale({ netDir == Direction::Right ? 2.f : -2.f,2.f });

	lastDir = netDir;

	networkTargetPos = { x, y };


	// ONLY change animation if state changed
	if (netState != previousNetworkState)
	{
		state = netState;
		switchAnimationByState(netState);
		currentAnimation->reset();

		playOtherSounds(netState);
		
		previousNetworkState = netState;
	}

	currentAnimation->setDirection(netDir);
}

void Player::applyDamage(int damage, Direction attackerDir, sf::Font& standardFont)
{
	if (isDead) return;

	if (invulnerable)
		return;

	health -= damage;

	int scaleX = 2, scaleY = 2;
	sf::Vector2f pos = playerBox.getPosition();
	DamageNumber damageNumber = createDamageNumberText(standardFont, "-" + std::to_string(damage), scaleX, scaleY, sf::Color::Red, pos);
	damageNumbers.push_back(std::move(damageNumber));

	startKnockback(attackerDir);
	std::cout << "HEALTH: " << health << std::endl;

	if (health <= 0)
		health = 0;

	currentAnimation->reset();
	state = PlayerState::Hurt;

	invulnerable = true;
	invulTimer = 0.3f;

	if (health == 0)
	{
		state = PlayerState::Dead;
	}

}

void Player::startKnockback(Direction dir)
{
	// Always allow knockback, even if invulnerable
	invulnerable = true;
	invulTimer = 0.4f;

	state = PlayerState::Hurt;

	int randomNumber = std::rand() % 3 + 1;

	if (hurtSoundClock.getElapsedTime().asSeconds() > 0.2f)
	{
		if (randomNumber == 1) hurtSound1.play();
		else if (randomNumber == 2) hurtSound2.play();
		else hurtSound3.play();
		
		hurtSoundClock.restart();
	}

	isHurt = true;
	isKnockedback = true;

	cancelAttack();

	knockbackTimer = 0.55f;

	velocity.x = (dir == Direction::Right) ? knockbackDistance : -knockbackDistance;
	velocity.y = -300.f;
}

void Player::resetPlayer(sf::Vector2f spawnPos, bool isSinglePlayer)
{
	health = maxHealth;

	playerBox.setPosition(spawnPos);
	currentAnimation->getSprite().setPosition(spawnPos);

	velocity = { 0.f, 0.f };

	isNormalAttacking = false;
	isJumping = false;
	isDead = false;
	isHurt = false;
	isClimbing = false;
	isSpecialAttack = false;
	isRolling = false;
	isOnGround = false;
	blockLeft = false;
	blockRight = false;

	playDeathOnce = false;

	knockbackTimer = 0.f;

	lastDir = spawnPos.x > 1770.f / 2 ? Direction::Left : Direction::Right;
	
	resetCharacter(isSinglePlayer);
}

string Player::stateToString(PlayerState state)
{
	switch (state)
	{
		case PlayerState::Idle: return "Idle";
		case PlayerState::Walk: return "Walk";
		case PlayerState::Run: return "Run";
		case PlayerState::Jump: return "Jump";
		case PlayerState::Roll: return "Roll";
		case PlayerState::Attack1: return "Attack1";
		case PlayerState::Attack2: return "Attack2";
		case PlayerState::Attack3: return "Attack3";
		case PlayerState::SprintAttack: return "SprintAttack";
		case PlayerState::SpecialAttack: return "Special";
		case PlayerState::Drink: return "Drink";
		case PlayerState::Hurt: return "Hurt";
		case PlayerState::Dead: return "Dead";
		case PlayerState::Hang: return "Hang";
		case PlayerState::Climb: return "Climb";
		case PlayerState::Stunned: return "Stun";
		case PlayerState::JumpAttack: return "JumpAttack";
		default: return "Unknown";
	}
	
}

void Player::multiplayerDeath()
{
	health = 0;
	state = PlayerState::Dead;

	if (currentAnimation->isFinished()) isDead = true;

	currentAnimation->getSprite().setScale({ 2.5f,2.5f });
}

void Player::updateDamageText(float dt)
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

DamageNumber Player::createDamageNumberText(sf::Font& standardFont, string message, int scaleX, int scaleY, sf::Color color, sf::Vector2f position)
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

void Player::handleWalkSounds()
{
	if (walkSoundClock.getElapsedTime() >= walkInterval && isOnGround)
	{
		if (playFirstWalkSound)
			walkSound1.play();
		else
			walkSound2.play();

		playFirstWalkSound = !playFirstWalkSound;
		walkSoundClock.restart();
	}
}

void Player::playParrySound()
{
	parrySound.play();
}

void Player::playDeflectionSounds()
{
	int randomNum = std::rand() % 2 + 1;

	randomNum == 1 ? shieldImpactSound1.play() : shieldImpactSound2.play();
}

float Player::getGuardTimer()
{
	return 0.0f;
}

sf::Sprite& Player::getPotionSprite()
{
	return potionSprite;
}

int Player::getPotionNumber()
{
	return potionNumber;
}

void Player::setPotionNumber(int num)
{
	this->potionNumber = num;
}

bool Player::isGrounded()
{
	return isOnGround;
}

float Player::getInvulTimer()
{
	return invulTimer;
}

bool Player::isSpecialAttacking()
{
	return isSpecialAttack;
}

vector<DamageNumber>& Player::getDamageNumber()
{
	return damageNumbers;
}

void Player::determineHitSound()
{
	int random = std::rand() % 3 + 1;

	switch (random)
	{
		case 1:
			swordHitSound1.play();
			break;
		case 2:
			swordHitSound2.play();
			break;
		default:
			swordHitSound3.play();
			break;
	}
}

bool Player::isRollingBool()
{
	return isRolling;
}

bool Player::isInvulnerableBool()
{
	return invulnerable;
}

void Player::setInvul(bool x)
{
	invulnerable = x;
}

void Player::setInvulTimer(float timer)
{
	invulTimer = timer;
}

int Player::getHealth()
{
	return health;
}

Healthbar& Player::getHealthBar()
{
	return healthbar;
}

int Player::getMaxHealth()
{
	return maxHealth;
}

Animation* Player::getAnimation()
{
	return currentAnimation;
}

bool Player::getIsHealed()
{
	return healed;
}

int Player::getHealAmount()
{
	return healAmount;
}

bool Player::isDeadBool()
{
	return this->isDead;
}

float Player::getAttackTimer()
{
	return attackTimer;
}

void Player::setHealBool(bool healed)
{
	this->healed = healed;
}

void Player::setHealth(int health)
{
	if (health > maxHealth)
	{
		this->health = maxHealth;
	}
	else
	{
		this->health = health;
	}
}

void Player::updateSinglePlayer(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<Arrow>& arrows, sf::Font& standardFont)
{
	return;
}

InputState Player::readInput()
{
	InputState input;

	input.left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
	input.right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
	input.jumpPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
	input.attackPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
	input.rollPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
	input.sprintHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);
	input.specialPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
	input.elixirPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R);

	input.attackJustPressed = input.attackPressed && !lastInput.attackPressed;
	input.jumpJustPressed = input.jumpPressed && !lastInput.jumpPressed;
	input.specialJustPressed = input.specialPressed && !lastInput.specialPressed;

	return input;
}

void Player::updateGravity(float dt, std::vector<Tile>& tiles)
{
	isOnGround = false;

	// 1. Apply gravity
	velocity.y += gravity * dt;

	// 2. Predict next position
	sf::FloatRect playerBounds = playerBox.getGlobalBounds();

	sf::FloatRect nextPlayerBoundsX = playerBounds;
	nextPlayerBoundsX.position.x += velocity.x * dt + 5;

	sf::FloatRect nextPlayerBoundsXLeft = playerBounds;
	nextPlayerBoundsXLeft.position.x += velocity.x * dt - 5;

	sf::FloatRect nextPlayerBounds = playerBounds;
	nextPlayerBounds.position.y += velocity.y * dt;

	blockLeft = false;
	blockRight = false;

	// 3. Check against tiles
	for (auto& tile : tiles)
	{
		// LEFT AND RIGHT COLLISION
		if (nextPlayerBoundsX.findIntersection(tile.getBounds()) && tile.isCollidableTile())
		{
			if (playerBounds.position.x + playerBounds.size.x - 50 <= tile.getBounds().position.x + 50.f)
			{
				blockRight = true;
				if (velocity.x > 0.f)
					velocity.x = 0.f;
			}
		}

		if (nextPlayerBoundsXLeft.findIntersection(tile.getBounds()) && tile.isCollidableTile())
		{
			if (playerBounds.position.x + 50 >= tile.getBounds().position.x + tile.getBounds().size.x - 50.f)
			{
				blockLeft = true;
				if (velocity.x < 0.f)
					velocity.x = 0.f;
			}
		}

		// LANDING COLLISION
		sf::FloatRect tileBounds = tile.getBounds();
		if (nextPlayerBounds.findIntersection(tileBounds))
		{
			if (tile.getID() == 11 || tile.getID() == 12)
				continue;

			float currentBottom = playerBounds.position.y + playerBounds.size.y;
			float nextBottom = nextPlayerBounds.position.y + nextPlayerBounds.size.y;
			float knightHeight = playerBounds.size.y;
			float tileTop = tile.getBounds().position.y;

			if (currentBottom <= tileTop && nextBottom >= tileTop)
			{
				playerBox.setPosition({
					playerBox.getPosition().x,
					tileTop - knightHeight - 50.f
					});

				velocity.y = 0.f;
				isOnGround = true;
				isJumping = false;
				break; // stop after first collision
			}
		}

		// CEILING COLLISION (hitting underside of tile)
		float currentTop = playerBounds.position.y;
		float nextTop = nextPlayerBounds.position.y;
		float tileBottom = tileBounds.position.y + tileBounds.size.y;
		bool overlapX =
			playerBounds.position.x + playerBounds.size.x > tileBounds.position.x &&
			playerBounds.position.x < tileBounds.position.x + tileBounds.size.x;

		if (velocity.y < 0.f && overlapX && currentTop >= tileBottom && nextTop <= tileBottom)
		{
			float penetration = tileBottom - nextTop;

			if (penetration > 0.f)
				playerBox.move({ 0.f, penetration + 0.5f });

			// soften instead of kill
			velocity.y = 10.f; // small downward push
		}
	}

	if (isClimbing)
		velocity = { 0.f, 0.f };

	playerBox.move({ velocity.x, velocity.y * dt });
}

bool Player::tryStartElixir(InputState& input)
{
	if (health == maxHealth) return false;
	if (!input.elixirPressed) return false;
	if (isSpecialAttack) return false;
	if (isDrinking) return false;
	if (potionNumber <= 0) return false;
	if (cancelElixir()) return false;

	elixirLogic();

	return true;
}

void Player::elixirLogic()
{
	currentAnimation->reset();
	isDrinking = true;
	state = PlayerState::Drink;
}

void Player::updatePotionLogic(sf::Font& standardFont)
{
	if (!isDrinking) return;
	if (isStunned) return;

	if (isDrinking && isHurt)
	{
		isDrinking = false;

	}
	else
	{
		if (!isHurt)
			velocity.x = 0.f;

		if (currentAnimation->getCurrentFrame() == 2)
		{
			//potionSound.play();
		}

		if (currentAnimation->isFinished())
		{
			currentAnimation->reset();

			health += healAmount;
			potionNumber--;

			int scaleX = 2, scaleY = 2;
			sf::Vector2f pos = playerBox.getPosition();
			DamageNumber damageNumber = createDamageNumberText(standardFont, "+" + std::to_string(healAmount), scaleX, scaleY, sf::Color::Green, pos);

			damageNumbers.push_back(std::move(damageNumber));

			if (health > maxHealth) health = maxHealth;

			healed = true;
			isDrinking = false;

		}
	}
}
