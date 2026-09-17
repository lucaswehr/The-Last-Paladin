#include "Samurai.hpp"
#include "Client.hpp"

void Samurai::draw(sf::RenderWindow& window)
{
	for (auto arrow : arrows)
	{
		window.draw(arrow.sprite);
		//window.draw(arrow.arrowBox);
	}

	for (const auto& damageNumber : damageNumbers)
	{
		window.draw(damageNumber.text);
	}

	window.draw(currentAnimation->getSprite());
	//window.draw(attackBox);
	//window.draw(climbingBox);
	//window.draw(playerBox);
}

void Samurai::updateMultiplayer(float dt, vector<Tile>& tiles, sf::Font& standardFont)
{ 
	updateDamageText(dt);

	if (handleDeathLogic(dt, tiles)) return;

	updateBoundryBoxes();
	updatePotionLogic(standardFont);
	updateStun(dt);

	InputState input = readInput();

	if (!isHurt && !isDrinking && !isStunned && !isClimbing)
	{
		handleActions(input, dt);
		updateAttackCooldown(dt);
	}
	climbingLogic(tiles);
	updateHitBoxWindow(dt);

	switchAnimationByState(state);

	lastInput = input;

	currentAnimation->getSprite().setPosition(playerBox.getPosition() + animationOffset());

	if (!isStunned) currentAnimation->update(dt);

	if (!isDrinking && !isStunned && !isClimbing)
	{
		updateAttack(dt);
		updateSpecialAttack(dt, tiles);
	}

		updateGravity(dt, tiles);
}

void Samurai::handleActions(InputState& input, float dt)
{
	if (isClimbing) return;

	if (tryStartSpecial(input)) return;
	if (tryStartNormalAttacks(input, dt)) return;
	if (tryStartJump(input)) return;
	if (tryStartElixir(input)) return;

    if(!isNormalAttacking && !isSpecialAttack) handleMovement(input);
}

void Samurai::handleMovement(InputState& input)
{
	if (input.right && !blockRight && !isNormalAttacking)
	{
		if (input.sprintHeld) isSprinting = true;
		else isSprinting = false;
	
		velocity.x = input.sprintHeld ? runSpeed : walkSpeed;
		
		lastDir = Direction::Right;
		if (!isJumping) state = input.sprintHeld ? PlayerState::Run : PlayerState::Walk;
		if (isOnGround) handleWalkSounds();
	}
	else if (input.left && !blockLeft && !isNormalAttacking)
	{
		if (input.sprintHeld) isSprinting = true;
		else isSprinting = false;
	
		velocity.x = input.sprintHeld ? -runSpeed : -walkSpeed;
		lastDir = Direction::Left;
		if (!isJumping) state = input.sprintHeld ? PlayerState::Run : PlayerState::Walk;
		if (isOnGround) handleWalkSounds();
	}
	else if (!isJumping)
	{
		velocity.x = 0.f;
		state = PlayerState::Idle;
		isSprinting = false;
	}
}

void Samurai::switchAnimationByState(PlayerState& state)
{
	switch (state)
	{
		case PlayerState::Idle: switchAnimation(&Idle); break;
		case PlayerState::Walk: switchAnimation(&Walk); break;
		case PlayerState::Run: switchAnimation(&Run); break;
		case PlayerState::Jump: switchAnimation(&Jump); break;
		case PlayerState::Attack1: switchAnimation(&Attack1); break;
		case PlayerState::Attack2: switchAnimation(&Attack2); break;
		case PlayerState::Attack3: switchAnimation(&Attack3); break;
		case PlayerState::SpecialAttack: switchAnimation(&Special); break;
		case PlayerState::Hurt: switchAnimation(&Hurt); break;
		case PlayerState::Dead: switchAnimation(&Dead); break;
		case PlayerState::Drink: switchAnimation(&Elixir); break;
		case PlayerState::Climb: switchAnimation(&Climb); break;
	}
}

bool Samurai::tryStartJump(InputState& input)
{
	if (!input.jumpJustPressed) return false;
	if (isJumping || isNormalAttacking || isSpecialAttack) return false;

	jumpLogic();

	return true;
}

void Samurai::jumpLogic()
{
	velocity.y = jumpStrength;
	isJumping = true;
	state = PlayerState::Jump;
	isSprinting = false;
	switchAnimation(&Jump);
	currentAnimation->reset();
	isOnGround = false;

	int randomNumber = std::rand() % 2 + 1;

	if (randomNumber == 1)
		jumpSound1.play();
	else
		jumpSound2.play();
}

bool Samurai::tryStartNormalAttacks(InputState& input, float dt)
{
	if (!input.attackJustPressed) return false;
	if (isSprinting || !isOnGround || isJumping || isHurt || isSpecialAttack) return false;

	velocity = { 0.f,0.f };

	if (isNormalAttacking)
	{
		comboQueued = true;
		return true;
	}

	attackLogicMultiplayer();

	return true;
}

void Samurai::attackLogicMultiplayer()
{
	isNormalAttacking = true;

	lastAttackFrame = -1;

	switch (comboStep)
	{
	case 0:
		comboStep = 1;
		state = PlayerState::Attack1;
		break;

	case 1:
		comboStep = 2;
		state = PlayerState::Attack2;
		break;

	case 2:
		comboStep = 3;
		state = PlayerState::Attack3;
		break;

	default:
		comboStep = 1;
		state = PlayerState::Attack1;
		break;
	}

	int attackID = determineAttackID();

	cout << "comboStep: " << comboStep
		<< " state: " << static_cast<int>(state)
		<< " attackID: " << attackID
		<< " damage: " << samuraiAttackTable[attackID].damage
		<< endl;

	startAttack(samuraiAttackTable[attackID], lastDir);

	comboQueued = false;
}

void Samurai::updateAttack(float dt)
{
	if (!isNormalAttacking) return;

	if (currentAnimation->isFinished())
	{
		currentAnimation->reset();
		attackTimer = 0.f;

		if (comboQueued)
		{
			attackLogicMultiplayer();
		}
		else
		{
			isNormalAttacking = false;
			comboClock.restart();		
		}
	}

	int frame = currentAnimation->getCurrentFrame();

	velocity.x = 0.f;

	if (state == PlayerState::Attack1 && frame >= 3 && frame <= 5)
	    velocity.x = (lastDir == Direction::Right) ? 9.f : -9.f;
	else if (state == PlayerState::Attack2 && frame >= 2 && frame <= 4)
	    velocity.x = (lastDir == Direction::Right) ? 8.f : -8.f;
	else if (state == PlayerState::Attack3 && frame >= 4 && frame <= 6)
	    velocity.x = (lastDir == Direction::Right) ? 17.f : -17.f;
	

	if (frame != lastAttackFrame)
	{
		lastAttackFrame = frame;

		if (state == PlayerState::Attack1 && frame == 3)
		    attackSound1.play();
		
		if (state == PlayerState::Attack2 && frame == 3)
		    attackSound2.play();
		
		if (state == PlayerState::Attack3 && frame == 4)
			attackSound3.play();
		
	}
}

void Samurai::updateAttackCooldown(float dt)
{
	if (!isNormalAttacking &&
		comboStep != 0 &&
		comboClock.getElapsedTime().asSeconds() > comboWindow)
	{
		comboStep = 0;
		
	}
}

bool Samurai::tryStartSpecial(InputState& input)
{
	if (!input.specialJustPressed) return false;
	if (isSpecialAttack) return false;
	if (isJumping || isNormalAttacking || isSprinting || !isOnGround) return false;

	specialAttackLogic();
	
	return true;
}

void Samurai::specialAttackLogic()
{
	state = PlayerState::SpecialAttack;
	isSpecialAttack = true;
	switchAnimation(&Special);
	currentAnimation->reset();
	velocity = { 0.f,0.f };
}

void Samurai::updateSpecialAttack(float dt, std::vector<Tile>& tiles)
{
	updateArrows(dt, tiles);

	if (!isSpecialAttack) return;

	playBowPullBackSound();

	if (currentAnimation->getCurrentFrame() == 12 && !arrowFired)
	{
		sf::Vector2f pos = { currentAnimation->getSprite().getPosition().x, currentAnimation->getSprite().getPosition().y + 125.f };

		this->bowShotSound.play();

		float randomGravity = 500 + std::rand() % 400; // 400-900

		spawnArrow(pos, lastDir, nextArrowID++, randomGravity);

		arrowFired = true;

		arrowSpawnedThisFrame = true;
	}

	if (currentAnimation->isFinished())
	{
		isSpecialAttack = false;
		state = PlayerState::Idle;
		arrowFired = false;
		bowPullback = false;
	}
}

void Samurai::spawnArrow(sf::Vector2f pos, Direction dir, int arrowID, float arrowGravity)
{
	sf::Vector2f direction = (dir == Direction::Right) ? sf::Vector2f(1, 0) : sf::Vector2f(-1, 0);

	arrows.emplace_back(arrowTex, pos, direction, arrowGravity);

	arrows.back().id = arrowID;

	lastArrowID = arrows.back().id;
}

void Samurai::updateArrows(float dt, vector<Tile>& tiles)
{
	for (auto it = arrows.begin(); it != arrows.end(); )
	{
		it->update(dt, arrows, tiles);

		for (const auto& tile : tiles)
		{
			if (tile.isCollidableTile() && it->getBounds().findIntersection(tile.getBounds()))
			{
				it->velocity = { 0.f, 0.f };
				it->setArrowOnTile(true);
			}
		}

		if (it->isExpired())
			it = arrows.erase(it);
		else
			++it;
	}
}


void Samurai::climbingLogic(std::vector<Tile>& tiles)
{
	if (!isClimbing)
	{
		for (auto& tile : tiles)
		{
			if (climbingBox.getGlobalBounds().findIntersection(tile.ledgeGrabBox) && !isOnGround && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
			{
				sf::Vector2f hangPos;

				if (tile.getID() == 32 || tile.getID() == 2)
				{
					hangPos = { tile.ledgeGrabBox.position.x + tile.ledgeGrabBox.size.x, tile.ledgeGrabBox.position.y };
				}
				else if (tile.getID() == 30 || tile.getID() == 0)
				{
					hangPos = { tile.ledgeGrabBox.position.x + tile.ledgeGrabBox.size.x, tile.ledgeGrabBox.position.y };
				}
				else
				{
					continue;
				}

				playerBox.setPosition(hangPos);

				if (isClimbing) playLedgeSound();

				isClimbing = true;

				switchAnimation(&Climb);
				currentAnimation->reset();
			}
		}
	}

	if (!isClimbing) return;

	switchAnimation(&Climb);
	state = PlayerState::Climb;

	playerBox.move({ 0.f,-6.f });

	if (currentAnimation->isFinished())
	{
		sf::Vector2f offset = (lastDir == Direction::Left) ? sf::Vector2f(-25.f, 0.f) : sf::Vector2f(25.f, 0.f);
		playerBox.move(offset);

		isClimbing = false;
		state = PlayerState::Jump;
		currentAnimation->reset();
	}
}

void Samurai::updateBoundryBoxes()
{
	sf::Vector2f pos = { currentAnimation->getPosition().x, currentAnimation->getPosition().y};

	climbingBox.setPosition(pos);
	attackBox.setPosition({ pos.x + 90.f, pos.y });

	if (lastDir == Direction::Left)
	{
		attackBox.setPosition({ pos.x - 90.f, pos.y });
		climbingBox.setPosition({ pos.x - 15.f, pos.y });
		
	}
}

void Samurai::determineCharacterHitbox()
{
	AttackHitbox hitbox;

	if (state == PlayerState::Attack1) hitbox = samuraiAttackBoxTable[(int)samuraiAttackType::Attack1];
	else if (state == PlayerState::Attack2) hitbox = samuraiAttackBoxTable[(int)samuraiAttackType::Attack2];
	else if (state == PlayerState::Attack3) hitbox = samuraiAttackBoxTable[(int)samuraiAttackType::Attack3];

	sf::FloatRect bounds = playerBox.getGlobalBounds();

	float centerX = bounds.position.x + bounds.size.x * 0.5f;
	float centerY = bounds.position.y + bounds.size.y * 0.5f;

	float dirSign = (lastDir == Direction::Right) ? 1.f : -1.f;

	float offset = hitbox.offset.x;

	if (lastDir == Direction::Left && state != PlayerState::Attack3) offset -= 45.f;

	attackBox.setSize(hitbox.size);

	attackBox.setPosition({ centerX + offset * dirSign, centerY + hitbox.offset.y });
}

void Samurai::updateHitBoxWindow(float dt)
{
	if (invulnerable)
	{
		invulTimer -= dt;

		if (invulTimer <= 0) invulnerable = false;
	}

	if (isNormalAttacking)
	{
		attackTimer += dt;

		hitboxActive = (attackTimer >= currentAttack.hitStart && attackTimer <= currentAttack.hitEnd);
	}
	else 
	{
		hitboxActive = false;
	}

	if (isKnockedback)
	{
		knockbackTimer -= dt;

		if (knockbackTimer <= 0.f)
		{		
			cancelAttack();

			isKnockedback = false;
			isHurt = false;
			velocity.x = 0.f; // stop horizontal movement
			state = PlayerState::Idle;
			switchAnimation(&Idle);
			
		}
	}
}

void Samurai::startAttack(const attackData& attack, Direction dir)
{
	currentAttack = attack;        // <-- store it here
	attackTimer = 0.0f;
	isNormalAttacking = true;
	hitboxActive = false;
	lastDir = dir;
}

AttackHitbox* Samurai::getAttackBoxTable()
{
	return samuraiAttackBoxTable;
}

const attackData* Samurai::getAttackTable() const
{
	return samuraiAttackTable;
}

void Samurai::cancelAttack()
{
	isNormalAttacking = false;
	isSpecialAttack = false;
	//hitboxActive = false;
	attackTimer = 0.f;
	arrowFired = false;
}

bool Samurai::isAttackingBool()
{
	return isNormalAttacking || isSpecialAttack;
}

void Samurai::updateRemotePlayers(float dt, vector<Tile>& tiles)
{
	sf::Vector2f pos = playerBox.getPosition();
	pos += (networkTargetPos - pos) * 20.f * dt;

	playerBox.setPosition(pos);

	sf::Vector2f offset = animationOffset();

	currentAnimation->getSprite().setPosition(pos + offset);

	if (state == PlayerState::Walk) handleWalkSounds();

	updateArrows(dt, tiles);
	updateDamageText(dt);

	if (!isDead)
	{
		if (isNormalAttacking)
		{
			attackTimer += dt;
			hitboxActive = (attackTimer >= currentAttack.hitStart && attackTimer <= currentAttack.hitEnd);
		}
		else
		{
			hitboxActive = false;
		}

		updateStun(dt);

		if (!isStunned)
		{
			currentAnimation->update(dt);
		}
		else
		{
			std::cout << "Animation is not playing / is stunned" << std::endl;
		}

		if (currentAnimation->isFinished())
		{
			if (state == PlayerState::Attack1 ||
				state == PlayerState::Attack2 ||
				state == PlayerState::Attack3 ||
				state == PlayerState::SpecialAttack)
			{
				state = PlayerState::Idle;
				switchAnimationByState(state);
			}

			isHurt = false;
		}

		if (invulnerable)
		{
			invulTimer -= dt;

			if (invulTimer <= 0) invulnerable = false;
		}
	}
}

void Samurai::resetCharacter(bool isSinglePlayer)
{
	comboStep = 0;
	comboQueued = false;

	arrows.clear();

	potionNumber = 2;

	switchAnimation(&Idle);
	currentAnimation->reset();
}

void Samurai::applyStun(float duration)
{
	isStunned = true;

	cancelAttack();

	stunTimer = duration;
	velocity = { 0.f, 0.f };
	currentAnimation->reset();

	state = PlayerState::Stunned;

	switchAnimation(&Dead);
	currentAnimation->reset();

	int scalar = lastDir == Direction::Right ? 1.f : -1.f;

	currentAnimation->setScale(2.f * scalar, 2.f);

	stunFrameTimer = 0.f;
	stunPhase = 0;            // 0 = going to frame 2, 1 = holding, 2 = reversing
}

bool Samurai::isParryWindow()
{
	return false;
}

int Samurai::determineAttackID()
{
	int attackID;

	switch (state)
	{
		case PlayerState::Attack1:
			attackID = 0;
			break;
		case PlayerState::Attack2:
			attackID = 1;
			break;
		case PlayerState::Attack3:
			attackID = 2;
			break;
		default:
			attackID = 0;
	}

	return attackID;
}

bool Samurai::handleDeathLogic(float dt, std::vector<Tile>& tiles)
{
	if (health <= 0 || playerBox.getPosition().y > 2200.f)
	{
		if (!playDeathOnce)
		{
		    deathSound1.play();

			state = PlayerState::Dead;
			switchAnimation(&Dead);

			health = 0;
			playDeathOnce = true;

		}

		if (currentAnimation->isFinished())
		{
			isDead = true;
			velocity.x = 0.f;

		}

		currentAnimation->getSprite().setScale({ lastDir == Direction::Right ? 2.f : -2.f, 2.f });

		currentAnimation->setPosition(playerBox.getPosition().x + animationOffset().x, playerBox.getPosition().y + animationOffset().y);

		updateGravity(dt, tiles);

		updateArrows(dt, tiles);

		currentAnimation->update(dt);

		return true;

	}

	return false;
}

void Samurai::processProjectileCollisions(Player& victim, int attackerID, int victimID, Client& client, sf::Font& standardFont)
{
	for (auto& arrow : arrows)
	{
		if (arrow.wasReflected) continue;

		if (!arrow.hasBeenReflected && attackerID == victimID) continue;

		if (!arrow.hasHit && arrow.getBounds().findIntersection(victim.getPlayerBox().getGlobalBounds()))
		{
			if (arrow.arrowGround) continue;
			if (victim.getHealth() <= 0) continue;

			std::cout << "COLLISION\n";

			int arrowDamage = arrow.hasBeenReflected ? 40 : 25;

			arrow.hasHit = true;

			Direction arrowHitDir = arrow.velocity.x > 0.f ? Direction::Right : Direction::Left;

			arrow.velocity = { 0.f,0.f };

			victim.applyDamage(arrowDamage, arrowHitDir, standardFont);

			victim.setInvul(true);
			victim.setInvulTimer(0.5f);

			client.sendDamage(attackerID, victimID, arrowDamage, arrowHitDir);
		}
	}
}

void Samurai::processProjectileDeflection(Player& victim, Client& client, int attackerID, int victimID, sf::Font& standardFont)
{
	for (auto& arrow : arrows)
	{
		if (arrow.arrowGround) continue;

		if (victim.isParryWindow() && !arrow.wasReflected && arrow.getBounds().findIntersection(victim.getAttackBox().getGlobalBounds()))
		{
			cout << "DEFLECTED" << endl;

			int scaleX = 2, scaleY = 2;
			sf::Vector2f pos = victim.getAnimation()->getPosition();
			DamageNumber damageNumber = createDamageNumberText(standardFont, "BLOCKED!", scaleX,scaleY, sf::Color::White, pos);
			victim.getDamageNumber().push_back(std::move(damageNumber));

			arrow.wasReflected = true;
			arrow.hasBeenReflected = true;
			arrow.hasHit = false;
			arrow.reflect();

			arrow.updateDirectionVisuals();

			client.sendArrowDeflection(attackerID, victimID, arrow.id, arrow.arrowBox.getPosition(), arrow.velocity);

		}

		if (!arrow.getBounds().findIntersection(victim.getAttackBox().getGlobalBounds()))
		{
			arrow.wasReflected = false;
		}
	}
}

sf::Vector2f Samurai::animationOffset()
{
	sf::Vector2f result = { 0.f,0.f };
	sf::Vector2f leftOffset;
	sf::Vector2f rightOffset;

	if (state == PlayerState::Dead)
	{
		leftOffset = { 120.f,-82.f };
		rightOffset = { -110.f,-82.f };
		result = lastDir == Direction::Right ? rightOffset : leftOffset;
	}
	else if (state == PlayerState::Stunned)
	{
		leftOffset = { 85.f,-82.f };
		rightOffset = { -125.f,-82.f };
		result = lastDir == Direction::Right ? rightOffset : leftOffset;
	}
	else if (state == PlayerState::Run)
	{
		leftOffset = { -30.f,0.f };
		rightOffset = { 30.f,0.f };
		result = lastDir == Direction::Right ? rightOffset : leftOffset;
	}

	return result;
	
}

bool Samurai::didArrowSpawnThisFrame()
{
	return arrowSpawnedThisFrame;
}

void Samurai::clearArrowSpawnFlag()
{
	arrowSpawnedThisFrame = false;
}

vector<Arrow>& Samurai::getArrows()
{
	return arrows;
}

bool Samurai::cancelElixir()
{
	return isJumping || isNormalAttacking || postAttackCooldown || isSprinting || !isOnGround;
}

int Samurai::getlastArrowID()
{
	return lastArrowID;
}

float Samurai::getArrowGravity()
{
	if (arrows.empty()) return 0.f;

	return arrows.back().gravity;
}

void Samurai::initializeSounds()
{
	walkBuffer1.loadFromFile("Sounds/step1.wav");
	walkSound1.setBuffer(walkBuffer1);

	walkBuffer2.loadFromFile("Sounds/step2.wav");
	walkSound2.setBuffer(walkBuffer2);

	jumpBuffer1.loadFromFile("Sounds/samuraiJump1.wav");
	jumpSound1.setBuffer(jumpBuffer1);

	jumpBuffer2.loadFromFile("Sounds/samuraiJump2.wav");
	jumpSound2.setBuffer(jumpBuffer2);

	hurtBuffer1.loadFromFile("Sounds/samuraiHurt1.wav");
	hurtSound1.setBuffer(hurtBuffer1);

	hurtBuffer2.loadFromFile("Sounds/samuraiHurt2.wav");
	hurtSound2.setBuffer(hurtBuffer2);

	hurtBuffer3.loadFromFile("Sounds/samuraiHurt3.wav");
	hurtSound3.setBuffer(hurtBuffer3);

	hurtSound1.setVolume(70);
	hurtSound2.setVolume(70);
	hurtSound3.setVolume(70);

	attackBuffer1.loadFromFile("Sounds/samuraiSwingSound1.mp3");
	attackSound1.setBuffer(attackBuffer1);

	attackBuffer2.loadFromFile("Sounds/samuraiSwingSound2.mp3");
	attackSound2.setBuffer(attackBuffer2);

	attackBuffer3.loadFromFile("Sounds/samuraiSwingSound3.mp3");
	attackSound3.setBuffer(attackBuffer3);

	parrySoundBuffer.loadFromFile("Sounds/parrySound.mp3");
	parrySound.setBuffer(parrySoundBuffer);

	deathBuffer1.loadFromFile("Sounds/samuraiDeathSound.wav");
	deathSound1.setBuffer(deathBuffer1);

	deathBuffer2.loadFromFile("Sounds/samuraiDeathSound2.mp3");
	deathSound2.setBuffer(deathBuffer2);

	swordHit1Buffer.loadFromFile("Sounds/samuraiSwordHit1.mp3");
	swordHitSound1.setBuffer(swordHit1Buffer);

	swordHit2Buffer.loadFromFile("Sounds/samuraiSwordHit2.mp3");
	swordHitSound2.setBuffer(swordHit2Buffer);

	swordHit3Buffer.loadFromFile("Sounds/samuraiSwordHit3.mp3");
	swordHitSound3.setBuffer(swordHit3Buffer);

	potionSoundBuffer.loadFromFile("Sounds/samuraiDrinkSound.mp3");
	potionSound.setBuffer(potionSoundBuffer);

	deathSound1.setVolume(300);
}



