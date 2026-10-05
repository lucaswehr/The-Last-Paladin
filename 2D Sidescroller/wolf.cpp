#include "wolf.hpp"
#include <cstdlib>
#include <algorithm>

void wolf::draw(sf::RenderWindow& window)
{
	window.draw(currentAnimation->getSprite());

	if (!isDead() && health < maxHealth)
		healthbar.draw(window);

	for (const auto& damageNumber : damageNumbers)
	{
		window.draw(damageNumber.text);
	}

	//window.draw(wolfBox);
	//window.draw(wolfAttackBox);
	//window.draw(rightLureBox);
	//window.draw(leftLureBox);
}

// ---------------------------------------------------------------------
// Flow:
//   Idle (spawn) --knight enters lure--> engageKnight() --50%--> Approach (walk up, attack)
//                                                       --50%--> Windup -> Sprint -> sprint Attack -> Recover
//   Approach / Windup --knight leaves lure--> Patrol
//   Patrol --knight enters lure--> engageKnight()
//   Attack / Recover / Knockback finish --> lure still occupied ? Idle (re-engage) : Patrol
// ---------------------------------------------------------------------
void wolf::update(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>&)
{
	updateBoundryBoxes();
	updateDamageText(dt);
	healthbar.update(dt, health, maxHealth, wolfBox.getPosition().x - 40.f, wolfBox.getPosition().y - 150.f);

	if (handleDeathLogic(dt)) return;

	if (attackCooldown > 0.f) attackCooldown -= dt;

	handleHurtLogic();

	float animSpeed = 1.f;

	switch (state)
	{
	case State::Idle:
		velocity.x = 0.f;
		switchAnimation(idleAnim.get());
		if (inLure())
			engageKnight();
		break;

	case State::Patrol:
		switchAnimation(walk.get());
		velocity.x = dirSign() * kWalkSpeed;
		if (inLure())
			engageKnight();
		break;

	case State::Approach:
		if (!inLure() && !inContact())
		{
			state = State::Patrol;
			break;
		}

		if (knightBehind()) { engageKnight(); break; }

		faceKnight();

		if (inContact())
		{
			velocity.x = 0.f;
			if (attackCooldown <= 0.f)
				startAttack(false);
			else
				switchAnimation(idleAnim.get()); // waiting for the next attack
		}
		else
		{
			switchAnimation(walk.get());
			velocity.x = dirSign() * kWalkSpeed;
		}
		break;

	case State::Windup:
		velocity.x = 0.f;
		switchAnimation(idleAnim.get());
		windupTimer -= dt;

		if (inContact())
			startAttack(false);
		else if (!inLure())
			state = State::Patrol;
		else if (knightBehind())
			engageKnight();
		else if (windupTimer <= 0.f)
			startSprint();
		break;

	case State::Sprint:
		if (!sprintOnce)
		{
			velocity.x = dirSign() * kSprintSpeed;
			sprintOnce = true;
		}
		sprintTimer -= dt;

		if (inContact())
			startAttack(true);
		else if (sprintTimer <= 0.f)
			startRecover(kRecoverTime);
		break;

	case State::Attack:
		if (wolfHitsShield)
			pendingShieldKnockback = true;

		lungeSpeed = std::max(0.f, lungeSpeed - kLungeDecay * dt);
		velocity.x = dirSign() * lungeSpeed;

		if (currentAnimation->isFinished())
			finishAttack();
		break;

	case State::Recover:
		velocity.x = 0.f;
		sprintOnce = false;
		switchAnimation(idleAnim.get());
		recoverTimer -= dt;
		if (recoverTimer <= 0.f)
		{
			if (inLure())
				engageKnight();
			else          
				state = State::Patrol;
		}
		break;

	case State::Knockback:
		switchAnimation(wolfHurt ? hurt.get() : walk.get());
		velocity.x = -dirSign() * kKnockbackSpeed;
		knockbackTimer -= dt;
		sprintOnce = false;
		if (knockbackTimer <= 0.f)
		{
			velocity.x = 0.f;
			wolfHurt = false;
			playOnce = false;
			wolfHitsShield = false;
			state = inLure() ? State::Idle : State::Patrol;
		}
		break;
	case State::Dead:
		break;
	}

	if (velocity.x != 0.f)
	{
		checkCollision(tiles);
		checkForCliff(tiles);
	}

	applyFacing();
	currentAnimation->getSprite().move({ velocity.x, 0.f });
	currentAnimation->update(dt * animSpeed);
}

// ---------------------------------------------------------------------
// Transitions
// ---------------------------------------------------------------------
void wolf::engageKnight()
{
	faceKnight();

	if (std::rand() % 100 < kSprintChancePct)
	{
		state = State::Windup;       // sprint attack
		windupTimer = kWindupTime;
		velocity.x = 0.f;
		suspenseSound.play();
	}
	else
	{
		state = State::Approach;     // normal walk-up attack
	}
}

void wolf::startSprint()
{
	state = State::Sprint;
	sprintTimer = kSprintMaxTime;
	chargeSound.play();
	switchAnimation(sprintAnim.get());
}

void wolf::startAttack(bool sprinting)
{
	if (sprinting)
	{
		switchAnimation(sprintAttackAnim.get());
	}
	else
	{
		switch (std::rand() % 3)
		{
		case 0:  switchAnimation(attack1.get()); break;
		case 1:  switchAnimation(attack2.get()); break;
		default: switchAnimation(attack3.get()); break;
		}
	}

	currentAnimation->reset();

	state = State::Attack;
	sprintAttacking = sprinting;
	lungeSpeed = sprinting ? kSprintSpeed : 0.f;
	velocity.x = dirSign() * lungeSpeed;
	attackCooldown = attackDelay;
	pendingShieldKnockback = wolfHitsShield;
}

void wolf::finishAttack()
{
	const bool wasSprint = sprintAttacking;

	sprintAttacking = false;
	lungeSpeed = 0.f;
	velocity.x = 0.f;

	if (pendingShieldKnockback)
	{
		pendingShieldKnockback = false;
		wolfHitsShield = true;
		startKnockback(kShieldKnockback);
	}
	else if (wasSprint)
	{
		startRecover(kRecoverTime);
	}
	else
	{
		state = inLure() ? State::Approach : State::Patrol;
	}
}

void wolf::startRecover(float time)
{
	state = State::Recover;
	recoverTimer = time;
	velocity.x = 0.f;
}

void wolf::startKnockback(float time)
{
	state = State::Knockback;
	knockbackTimer = time;
}

void wolf::turnAround()
{
	lastDir = (lastDir == enemyDirection::Right) ? enemyDirection::Left : enemyDirection::Right;
	velocity.x = -velocity.x;
}

bool wolf::knightBehind()
{
	return (lastDir == enemyDirection::Right && leftLure && !rightLure) ||
		(lastDir == enemyDirection::Left && rightLure && !leftLure);	
}


// ---------------------------------------------------------------------
// Boxes / death / hurt
// ---------------------------------------------------------------------
void wolf::updateBoundryBoxes()
{
	const sf::Vector2f pos = currentAnimation->getSprite().getPosition();

	wolfBox.setPosition(pos);
	rightLureBox.setPosition(pos);
	leftLureBox.setPosition(pos);
	wolfAttackBox.setPosition({ pos.x + 50.f * dirSign(), pos.y });
}

bool wolf::handleDeathLogic(float dt)
{
	if (health > 0)
		return false;

	if (!playAnimationOnce)
	{
		if (health <= 0)
			deathSound.play();

		switchAnimation(death.get());
		velocity.x = 0.f;
		playAnimationOnce = true;
		dead = true;
		state = State::Dead;
	}

	currentAnimation->getSprite().setColor(sf::Color::White);
	currentAnimation->getSprite().setScale({ dirSign() * kSpriteScale, kSpriteScale });
	currentAnimation->update(dt);
	return true;
}

void wolf::handleHurtLogic()
{
	if (!wolfHurt || playOnce)
		return;

	if (std::rand() % 2 == 0)
		hurtSound1.play();
	else
		hurtSound2.play();

	playOnce = true;
	health -= knightDamage;

	sf::Vector2f pos = { wolfBox.getPosition().x, wolfBox.getPosition().y - 140 };
	damageNumbers.push_back(std::move(
		createDamageNumberText(damageFontText, "-" + std::to_string(knightDamage), 2, 2, sf::Color::Red, pos)));
}

// ---------------------------------------------------------------------
// Facing
// ---------------------------------------------------------------------
void wolf::faceKnight()
{
	if (rightLure && !leftLure)      lastDir = enemyDirection::Right;
	else if (leftLure && !rightLure) lastDir = enemyDirection::Left;
}

void wolf::applyFacing()
{
	sf::Sprite& s = currentAnimation->getSprite();
	s.setScale({ dirSign() * kSpriteScale, kSpriteScale });
	s.setColor(state == State::Windup ? sf::Color(255, 150, 100) : sf::Color::White); // sprint telegraph
}

// ---------------------------------------------------------------------
// Environment
// ---------------------------------------------------------------------
void wolf::checkCollision(std::vector<Tile>& tiles)
{
	const sf::FloatRect box = wolfBox.getGlobalBounds();

	for (const auto& tile : tiles)
	{
		if (!tile.isCollidableTile()) continue;

		const sf::FloatRect tb = tile.getBounds();
		if (!box.findIntersection(tb)) continue;

		// Which side of the wolf is the tile on? (same test you had before)
		const bool tileOnRight = box.position.x + box.size.x - 50.f <= tb.position.x + 50.f;

		// Moving away from the tile: ignore it so the wolf can back out of the wall
		const bool movingIntoTile = (tileOnRight && velocity.x > 0.f) || (!tileOnRight && velocity.x < 0.f);
		if (!movingIntoTile) continue;

		switch (state)
		{
		case State::Patrol:
		{
			const bool fromRight = box.position.x + box.size.x - 50.f <= tb.position.x + 50.f;
			lastDir = fromRight ? enemyDirection::Left : enemyDirection::Right;
			velocity.x = -velocity.x;
			break;
		}
		case State::Sprint:
			startRecover(kRecoverTime);   // slammed into a wall
			break;
		default:
			velocity.x = 0.f;             // approach / lunge / knockback: just stop
			lungeSpeed = 0.f;
			break;
		}
		return;
	}
}

bool wolf::groundAhead(std::vector<Tile>& tiles)
{
	const sf::Vector2f pos = currentAnimation->getSprite().getPosition();

	const float look = (state == State::Sprint) ? 60.f : 20.f;
	const float checkX = pos.x + (velocity.x > 0.f ? look : -look);
	const float checkY = pos.y + 10.f;

	for (const auto& tile : tiles)
	{
		if (!tile.isCollidableTile()) continue;
		if (tile.getBounds().contains({ checkX, checkY }))
			return true;
	}
	return false;
}

void wolf::checkForCliff(std::vector<Tile>& tiles)
{
	if (velocity.x == 0.f || groundAhead(tiles))
		return;

	switch (state)
	{
	case State::Patrol:
		turnAround();
		break;
	case State::Sprint:
		startRecover(kRecoverTime);
		break;
	default:
		velocity.x = 0.f;   // approach / lunge / knockback: stop at the edge
		lungeSpeed = 0.f;
		break;
	}
}

// ---------------------------------------------------------------------
// Accessors / Enemy interface
// ---------------------------------------------------------------------
sf::FloatRect wolf::getBounds() { return wolfBox.getGlobalBounds(); }
sf::FloatRect wolf::getAttackBoxBounds() { return wolfAttackBox.getGlobalBounds(); }
sf::FloatRect wolf::getEnemyRightLure() { return rightLureBox.getGlobalBounds(); }
sf::FloatRect wolf::getEnemyLeftLure() { return leftLureBox.getGlobalBounds(); }

void wolf::setWolfVelocity(float) {}

void wolf::knightDamagedTrue() { knightDamaged = true; }
void wolf::knightDamagedFalse() { knightDamaged = false; }
bool wolf::isDead() { return dead; }

void wolf::isHurtTrue()
{
	if (dead) return;

	chargeSound.stop();
	suspenseSound.stop();
	wolfHurt = true;
	sprintAttacking = false;
	pendingShieldKnockback = false;
	lungeSpeed = 0.f;

	startKnockback(kHurtKnockback);
}

void wolf::isHurtFalse() { wolfHurt = false; }
void wolf::hitsShieldTrue() { wolfHitsShield = true; }

enemyDirection wolf::getDirection() { return lastDir; }
void wolf::setDirection(enemyDirection newDir) { lastDir = newDir; }

Animation* wolf::getCurrentEnemyAnimation() { return currentAnimation; }

bool wolf::setEnemyRightLure(bool value) { return rightLure = value; }
bool wolf::setEnemyLeftLure(bool value) { return leftLure = value; }

int wolf::getHealth() { return health; }

bool wolf::setInitializerBox(bool) { return false; }
sf::FloatRect wolf::getIninitializerBox() { return sf::FloatRect(); }