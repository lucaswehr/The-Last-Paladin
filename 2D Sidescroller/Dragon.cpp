#include "Dragon.hpp"
#include <algorithm>
#include <cstdlib>

namespace
{
	bool touchesAnyTile(const sf::FloatRect& box, const std::vector<Tile>& tiles)
	{
		for (const auto& tile : tiles)
			if (box.findIntersection(tile.getBounds()))
				return true;
		return false;
	}
}

// ============================================================================
//  UPDATE
// ============================================================================
void Dragon::update(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>& enemies)
{
	updateDamageText(dt);

	if (hurtCooldown > 0.f)
		hurtCooldown -= dt;

	if (ledgeCooldown > 0.f && airState == AirState::None)
		ledgeCooldown -= dt;

	// ---- Hitbox / helper positioning -------------------------------------
	const sf::Vector2f pos = currentAnimation->getSprite().getPosition();

	const bool raised =
		currentAnimation == Flight.get() ||
		currentAnimation == Takeoff.get() ||
		currentAnimation == Special.get() ||
		currentAnimation == Landing.get();

	dragonBox.setPosition({ pos.x, raised ? pos.y - 100.f : pos.y });
	placeHolder.setPosition({ pos.x, raised ? pos.y + 250.f : pos.y });

	// helper remembers the last grounded position (swoop teleports are relative to it)
	if (currentAnimation != Flight.get() && currentAnimation != Takeoff.get() &&
		currentAnimation != Special.get() && currentAnimation != Hurt.get())
		helper.setPosition(pos);

	leftLureBox.setPosition(pos);
	rightLureBox.setPosition(pos);
	attackInitializer.setPosition(pos);

	if (currentAnimation == Flight.get() || (!initalizer && !isMoving))
		attackBox.setPosition(pos);

	if (lastDir == enemyDirection::Left)
	{
		placeHolder.setOrigin({ 180.f, -500.f });
		attackInitializer.setOrigin({ 100.f, -175.f });
	}
	else
	{
		placeHolder.setOrigin({ -160.f, -500.f });
		attackInitializer.setOrigin({ -100.f, -175.f });
	}

	// ---- Ground patrol velocity ------------------------------------------
	if (currentAnimation == Walk.get())
		velocity.x = (lastDir == enemyDirection::Right) ? 2.f : -2.f;
	else if (currentAnimation == Idle.get())
		velocity.x = 0.f;

	// ---- Boss music starts the first time the knight is lured -------------
	if ((leftLure || rightLure) && playOnce16)
	{
		bossMusic.play();
		fightStarted = true;
		playOnce16 = false;
	}

	// ---- Incoming hit -----------------------------------------------------
	if (isHurt)
	{
		isHurt = false;
		if (!dead && hurtCooldown <= 0.f)
			takeHit();
	}

	// ---- Hurt animation finished ------------------------------------------
	if (hurtStun)
	{
		if (currentAnimation != Hurt.get())
		{
			hurtStun = false;
		}
		else if (currentAnimation->isFinished())
		{
			velocity.x = 0.f;
			Hurt->reset();
			hurtStun = false;
			switchAnimation(Idle.get());
		}
	}

	// ---- Behaviour --------------------------------------------------------
	if (!dead && !hurtStun)
	{
		if (airState == AirState::None)
		{
			checkCollision(tiles);
			checkForCliff(tiles);
		}

		attackLogic(tiles, dt);
	}

	// ---- Death ------------------------------------------------------------
	if (health <= 0)
	{
		if (!playOnce4)
		{
			dead = true;
			playOnce4 = true;
			fightStarted = false;
			airState = AirState::None;
			hurtStun = false;
			velocity = { 0.f, 0.f };

			dragonDeathSound.play();
			dragonFlapSound.stop();
			dragonFireSpecialSound.stop();
			dragonFlameSound.stop();
		}

		switchAnimation(Dead.get());
		gravityLogic(tiles, dt);
		currentAnimation->getSprite().move({ 0.f, velocity.y });
		currentAnimation->update(dt);

		if (background)
			dragonBar.update(dt, health, maxHealth, background->getPosition().x + 400.f, background->getPosition().y + 900.f);
		return;
	}

	updateAttackBox();

	gravityLogic(tiles, dt);

	currentAnimation->getSprite().move(velocity);
	currentAnimation->update(dt);

	if (background)
	{
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

	for (const auto& damageNumber : damageNumbers)
	{
		window.draw(damageNumber.text);
	}

	//window.draw(placeHolder);
	//window.draw(dragonBox);
	//window.draw(this->rightLureBox);
	//window.draw(this->leftLureBox);
	//window.draw(attackBox);
	//window.draw(attackInitializer);
	//window.draw(helper);
}

// ============================================================================
//  SIMPLE GETTERS / SETTERS (unchanged)
// ============================================================================
sf::FloatRect Dragon::getBounds() { return this->dragonBox.getGlobalBounds(); }
sf::FloatRect Dragon::getAttackBoxBounds() { return this->attackBox.getGlobalBounds(); }
bool Dragon::isDead() { return this->dead; }
int Dragon::getHealth() { return health; }

void Dragon::isHurtTrue() { isHurt = true; }
void Dragon::isHurtFalse() { isHurt = false; }
void Dragon::hitsShieldTrue() {}
Animation* Dragon::getCurrentEnemyAnimation() { return currentAnimation; }
void Dragon::knightDamagedTrue() { knightDamaged = true; }
void Dragon::knightDamagedFalse() { knightDamaged = false; }
enemyDirection Dragon::getDirection() { return lastDir; }
void Dragon::setDirection(enemyDirection newDir) { lastDir = newDir; }
bool Dragon::setEnemyLeftLure(bool value) { return leftLure = value; }
bool Dragon::setEnemyRightLure(bool value) { return rightLure = value; }
sf::FloatRect Dragon::getEnemyRightLure() { return rightLureBox.getGlobalBounds(); }
sf::FloatRect Dragon::getEnemyLeftLure() { return leftLureBox.getGlobalBounds(); }
bool Dragon::setInitializerBox(bool value) { return this->initalizer = value; }
sf::FloatRect Dragon::getIninitializerBox() { return this->attackInitializer.getGlobalBounds(); }
void Dragon::setBackgroundShape(sf::RectangleShape* bg) { background = bg; }

// ============================================================================
//  GROUND PATROL (unchanged behaviour)
// ============================================================================
void Dragon::checkCollision(std::vector<Tile>& tiles)
{
	for (auto& tile : tiles)
	{
		if (dragonBox.getGlobalBounds().findIntersection(tile.getBounds()))
			isColliding = true;
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

	for (const auto& tile : tiles)
	{
		if (placeHolder.getGlobalBounds().findIntersection(tile.getBounds()))
		{
			isColliding = true;
			switchAnimation(Walk.get());
		}
	}

	if (!isColliding)
		switchAnimation(Idle.get());

	if (!isColliding && lastDir == enemyDirection::Right)
	{
		if (dragonIdleClock.getElapsedTime() >= idleInterval && currentAnimation == Idle.get())
		{
			lastDir = enemyDirection::Left;
			currentAnimation->getSprite().setPosition({ currentAnimation->getSprite().getPosition().x - 20.f, currentAnimation->getSprite().getPosition().y });
			switchAnimation(Walk.get());
			dragonIdleClock.restart();
		}
	}
	else if (!isColliding && lastDir == enemyDirection::Left)
	{
		if (dragonIdleClock.getElapsedTime() >= idleInterval && currentAnimation == Idle.get())
		{
			lastDir = enemyDirection::Right;
			currentAnimation->getSprite().setPosition({ currentAnimation->getSprite().getPosition().x + 20.f, currentAnimation->getSprite().getPosition().y });
			switchAnimation(Walk.get());
			dragonIdleClock.restart();
		}
	}
}

// ============================================================================
//  DAMAGE
// ============================================================================
void Dragon::takeHit()
{
	hurtCooldown = hurtCooldownTime;
	health -= knightDamage;

	sf::Vector2f pos = { dragonBox.getPosition().x, dragonBox.getPosition().y + 50 };
	damageNumbers.push_back(std::move(
		createDamageNumberText(damageFontText, "-" + std::to_string(knightDamage), 3, 3, sf::Color::Red, pos)));

	if (health <= 0)
		return;   // death is handled in update()

	const int r = std::rand() % 3;
	if (r == 0)      dragonHurtSound.play();
	else if (r == 1) dragonHurtSound2.play();
	else             dragonHurtSound3.play();

	// Only stagger on the ground and not in the middle of a flame attack.
	if (airState == AirState::None && currentAnimation != Attack2.get())
	{
		hurtStun = true;
		Hurt->reset();
		switchAnimation(Hurt.get());

		if (health == 100 || health == 40)
			velocity.x = (lastDir == enemyDirection::Right) ? -7.f : 7.f;
	}
}

// ============================================================================
//  ATTACK BOX
// ============================================================================
void Dragon::updateAttackBox()
{
	if (currentAnimation == Special.get())
	{
		attackBox.setSize({ 35.f, 100.f });

		if (lastDir == enemyDirection::Right)
			attackBox.setOrigin({ -40.f, -80.f });
		else
			attackBox.setOrigin({ 80.f, -80.f });

		isMoving = true;

		attackBox.setPosition(currentAnimation->getSprite().getPosition());

		specialOffset.y += 10.f;

		if (Special->getCurrentFrame() >= 8)
			specialOffset.y = 0.f;

		if (Special->getCurrentFrame() == 12)
			attackBox.setOrigin({ -80.f, -80.f });

		attackBox.move(specialOffset);
	}
	else
	{
		attackBox.setSize({ 100.f, 35.f });
		attackBox.setScale({ 2, 2 });
		attackBox.setOrigin({ 50.f, -170.f });
		attackBox.setFillColor(sf::Color(255, 0, 0, 128));

		isMoving = false;
	}
}

// ============================================================================
//  ATTACK DECISIONS
// ============================================================================
void Dragon::attackLogic(std::vector<Tile>& tiles, float dt)
{
	// Already airborne: the state machine owns everything.
	if (airState != AirState::None)
	{
		airAttackLogic(tiles, dt);
		return;
	}

	const bool knightInArena = leftLure || rightLure;
	bool flameCycleDone = false;

	// ---- Ground flame attack (knight stepped into the initializer box) -----
	if (initalizer)
	{
		switchAnimation(Attack2.get());

		if (playOnce9 && currentAnimation->getCurrentFrame() > 3)
		{
			dragonFlameSound.play();
			playOnce9 = false;
		}

		if (Attack2->getCurrentFrame() < 8)
			attackBox.move({ lastDir == enemyDirection::Left ? -5.f : 5.f, 0.f });

		velocity.x = 0.f;

		if (currentAnimation->isFinished())
		{
			playOnce9 = true;
			currentAnimation->reset();
			flameCycleDone = true;
		}
	}

	// ---- Air attack timer: only runs while the knight is in the arena ------
	if (knightInArena)
		airTimer += dt;

	// Let a flame finish its cycle before taking off (a window opens every cycle,
	// so a knight standing in front can't block the dragon from flying forever).
	if (currentAnimation == Attack2.get() && !flameCycleDone)
		return;

	// ---- Ledge guarantee ---------------------------------------------------
	// Knight is on the side the dragon is facing AND there is a ledge right ahead
	// -> the knight would be pinned between the dragon and the cliff. Take off now.
	const bool knightOnLedgeSide = (lastDir == enemyDirection::Right) ? rightLure : leftLure;

	if (ledgeCooldown <= 0.f && knightOnLedgeSide && isNearLedgeAhead(tiles))
	{
		startAirAttack(true, tiles);
	}
	else if (knightInArena && airTimer >= airAttackInterval)
	{
		startAirAttack(false, tiles);
	}
}

bool Dragon::isNearLedgeAhead(const std::vector<Tile>& tiles) const
{
	// Same probe as the cliff check, pushed further ahead so "near" counts too.
	sf::FloatRect probe = placeHolder.getGlobalBounds();
	probe.position.x += (lastDir == enemyDirection::Right) ? ledgeWarnDistance : -ledgeWarnDistance;

	return !touchesAnyTile(probe, tiles);
}

void Dragon::turnAround()
{
	const bool wasRight = (lastDir == enemyDirection::Right);

	lastDir = wasRight ? enemyDirection::Left : enemyDirection::Right;

	const sf::Vector2f p = currentAnimation->getSprite().getPosition();
	currentAnimation->getSprite().setPosition({ p.x + (wasRight ? -20.f : 20.f), p.y });

	switchAnimation(currentAnimation);   // re-applies scale from lastDir
}

void Dragon::startAirAttack(bool fromLedge, const std::vector<Tile>& tiles)
{
	airTimer = 0.f;

	// 1 in 4 timer-based attacks is a swoop. Ledge escapes are never swoops.
	swoopAttack = !fromLedge && (std::rand() % 4 == 3);

	// The Special sweep flies the way the dragon faces until the platform ends,
	// so make sure it has room: face away from a nearby ledge.
	if (!swoopAttack && (fromLedge || isNearLedgeAhead(tiles)))
		turnAround();

	enterAirState(AirState::Takeoff);
}

// ============================================================================
//  AIR STATE MACHINE
// ============================================================================
void Dragon::enterAirState(AirState next)
{
	airState = next;
	phaseTime = 0.f;
	whooshPlayed = false;

	const sf::Vector2f anchor = helper.getGlobalBounds().getCenter();

	switch (next)
	{
	case AirState::Takeoff:
		Takeoff->reset();
		switchAnimation(Takeoff.get());
		dragonTakeoffSound.play();
		velocity = { 0.f, 0.f };
		break;

	case AirState::SpecialFlight:
		Flight->reset();
		switchAnimation(Flight.get());
		Flight->setFrameDuration(0.1f);
		dragonFlapSound.play();
		velocity = { 0.f, 0.f };
		break;

	case AirState::Special:
		Special->reset();
		switchAnimation(Special.get());
		specialOffset = { 0.f, 0.f };
		dragonFlapSound.stop();
		dragonFireSpecialSound.play();
		break;

	case AirState::SwoopClimb:
		Flight->reset();
		switchAnimation(Flight.get());
		Flight->setFrameDuration(0.04f);
		dragonFlapSound.play();
		break;

	case AirState::SwoopOut:
		lastDir = enemyDirection::Left;
		switchAnimation(currentAnimation);
		currentAnimation->getSprite().setPosition({ anchor.x + 3000.f, anchor.y - 270.f });
		velocity = { 0.f, 0.f };
		break;

	case AirState::SwoopBack:
		lastDir = enemyDirection::Right;
		switchAnimation(currentAnimation);
		break;

	case AirState::SwoopRise:
		break;

	case AirState::SwoopDive:
		lastDir = enemyDirection::Left;
		switchAnimation(currentAnimation);
		currentAnimation->getSprite().setPosition({ anchor.x + 3000.f, anchor.y - 1700.f });
		velocity = { 0.f, 0.f };
		break;

	case AirState::Landing:
		Landing->reset();
		switchAnimation(Landing.get());
		isLanding = true;
		dragonFlapSound.stop();
		dragonFireSpecialSound.stop();
		dragonLandSound.play();
		if (!swoopAttack)
			velocity = { 0.f, 0.f };
		break;

	default:
		break;
	}
}

void Dragon::airAttackLogic(std::vector<Tile>& tiles, float dt)
{
	phaseTime += dt;

	const float dir = (lastDir == enemyDirection::Right) ? 1.f : -1.f;

	switch (airState)
	{
		// ---------------- shared start ----------------
	case AirState::Takeoff:
		velocity.x = 0.f;
		velocity.y = (currentAnimation->getCurrentFrame() >= 3) ? -7.f : 0.f;

		if (currentAnimation->isFinished() || phaseTime > 3.f)
			enterAirState(swoopAttack ? AirState::SwoopClimb : AirState::SpecialFlight);
		break;

		// ---------------- fire sweep ----------------
	case AirState::SpecialFlight:
		velocity = { 0.f, 0.f };

		if (Flight->getCurrentFrame() >= 11 || phaseTime > 3.f)
			enterAirState(AirState::Special);
		break;

	case AirState::Special:
		velocity = { dir * 7.f, 0.f };

		// Fly until the platform ends (probe below the dragon loses the ground)
		if ((phaseTime > 0.25f && !touchesAnyTile(placeHolder.getGlobalBounds(), tiles)) || phaseTime > 12.f)
			enterAirState(AirState::Landing);
		break;

		// ---------------- swoop ----------------
	case AirState::SwoopClimb:
		velocity = { dir * 12.f, -8.f };

		if (phaseTime >= waitInterval.asSeconds())
			enterAirState(AirState::SwoopOut);
		break;

	case AirState::SwoopOut:
		velocity = { -27.f, 0.f };

		if (!whooshPlayed && phaseTime >= 0.3f)
		{
			dragonWhooshSound.play();
			whooshPlayed = true;
		}

		if (phaseTime >= waitInterval2.asSeconds())
			enterAirState(AirState::SwoopBack);
		break;

	case AirState::SwoopBack:
		velocity = { 27.f, 0.f };

		if (!whooshPlayed && phaseTime >= 0.6f)
		{
			dragonWhooshSound.play();
			whooshPlayed = true;
		}

		if (phaseTime >= 2.5f)
			enterAirState(AirState::SwoopRise);
		break;

	case AirState::SwoopRise:
		velocity = { 20.f, -8.f };

		if (phaseTime >= 2.5f)
			enterAirState(AirState::SwoopDive);
		break;

	case AirState::SwoopDive:
	{
		velocity = { -20.f, 7.f };

		const bool hitGround = touchesAnyTile(placeHolder.getGlobalBounds(), tiles);
		const bool fellOut = currentAnimation->getSprite().getPosition().y > helper.getPosition().y + 800.f;

		if (hitGround)
		{
			enterAirState(AirState::Landing);
		}
		else if (fellOut || phaseTime > 8.f)
		{
			// Failsafe: never found ground, put the dragon back above where it took off
			const sf::Vector2f home = helper.getPosition();
			currentAnimation->getSprite().setPosition({ home.x, home.y - 150.f });
			velocity = { 0.f, 0.f };
			enterAirState(AirState::Landing);
		}
		break;
	}

	// ---------------- landing ----------------
	case AirState::Landing:
		if (swoopAttack)
		{
			velocity.y += gravity * 4.f * dt;                       // heavy crash landing
			velocity.x -= velocity.x * std::min(1.f, 8.f * dt);     // skid to a stop
		}
		else
		{
			velocity.x = 0.f;
		}

		if (currentAnimation->isFinished() && (grounded || phaseTime > 3.f))
			finishAirAttack();
		break;

	default:
		break;
	}
}

void Dragon::finishAirAttack()
{
	airState = AirState::None;
	phaseTime = 0.f;
	airTimer = 0.f;
	ledgeCooldown = ledgeCooldownTime;   // stops back-to-back ledge take-offs
	isLanding = false;
	swoopAttack = false;
	velocity = { 0.f, 0.f };
	specialOffset = { 0.f, 0.f };

	leftLure = false;
	rightLure = false;
	initalizer = false;
	playOnce9 = true;

	Takeoff->reset();
	Flight->reset();
	Landing->reset();
	Special->reset();
	Attack2->reset();

	switchAnimation(Idle.get());
}

// ============================================================================
//  GRAVITY
// ============================================================================
void Dragon::gravityLogic(std::vector<Tile>& tiles, float dt)
{
	grounded = false;

	if (currentAnimation != Flight.get() && currentAnimation != Special.get() &&
		currentAnimation != Takeoff.get() && currentAnimation != Hurt.get())
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
					  currentAnimation->getSprite().getPosition().y - intersection->size.y });

				velocity.y = 0.f;
				grounded = true;
			}
		}
	}
}