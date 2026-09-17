#include "Knight.hpp"
#include "Skeleton.hpp"
#include "Dragon.hpp"
#include "wolf.hpp"

void Knight::updateMultiplayer(float dt, std::vector<Tile>& tiles, sf::Font& standardFont)
{
	/*std::cout << "X: " << currentAnimation->getPosition().x << "Y: " << currentAnimation->getPosition().y << std::endl;*/
	updateDamageText(dt);

	if (handleDeathLogic(dt, tiles)) return;

	updatePotionLogic(standardFont);
	updateStun(dt);
	updateBoundryBoxes();

	currentAnimation->setDirection(lastDir);

	InputState input = readInput();

	if (!isHurt && !isDrinking && !isStunned && !isClimbing)
	{
		handleActions(input, dt);

		updateJumpAttack(dt);
		updateJumpAttackCooldown(dt);

		updateRollLogic();

		updateAttack(dt);
		updateAttackCooldown(dt);

		updateSprintAttack();
		updateSprintAttackCooldown(dt);

		updateSpecialAttack(dt,tiles);

		if (isJumping && !isJumpAttacking)
		{
			switchAnimation(&jump);
			state = PlayerState::Jump;
		}

		if (isNormalAttacking && isOnGround) velocity.x = 0.f;
	
		if ((isSpecialAttack && currentAnimation->isFinished()))
		{
			isSpecialAttack = false;
			state = PlayerState::Idle;
		}

	}

	updateHitBoxWindow(dt);
	climbingLogic(tiles);
	updateGravity(dt, tiles);
	pullUpLogic();

    switchAnimationByState(state);

	lastInput = input;

	currentAnimation->getSprite().setPosition(playerBox.getPosition() + animationOffset());
	
	if (!isStunned) currentAnimation->update(dt);

}

void Knight::switchAnimationByState(PlayerState& state)
{
	switch (state)
	{
		case PlayerState::Idle: switchAnimation(&Idle); break;
		case PlayerState::Walk: switchAnimation(&walk); break;
		case PlayerState::Run: switchAnimation(&run); break;
		case PlayerState::Jump: switchAnimation(&jump); break;
		case PlayerState::Roll: switchAnimation(&roll); break;
		case PlayerState::Attack1: switchAnimation(&attack1); break;
		case PlayerState::Attack2: switchAnimation(&attack2); break;
		case PlayerState::Attack3: switchAnimation(&attack3); break;
		case PlayerState::SprintAttack: switchAnimation(&runningAttack); break;
		case PlayerState::SpecialAttack: switchAnimation(&guard); break;
		case PlayerState::Drink: switchAnimation(&elixir); break;
		case PlayerState::Hurt: switchAnimation(&hurt); break;
		case PlayerState::Climb: switchAnimation(&climbing); break;
		case PlayerState::Hang: switchAnimation(&hanging); break;
		case PlayerState::Dead: switchAnimation(&dead); break;
		case PlayerState::JumpAttack: switchAnimation(&jumpAttack); break;
	}
}

void Knight::initializeSounds()
{
	
	walkBuffer1.loadFromFile("Sounds/step1.wav");
	walkSound1.setBuffer(walkBuffer1);

	walkBuffer2.loadFromFile("Sounds/step2.wav");
	walkSound2.setBuffer(walkBuffer2);

	walkSound1.setVolume(30);
	walkSound2.setVolume(30);


	attackBuffer1.loadFromFile("Sounds/swish-1.wav");
	attackSound1.setBuffer(attackBuffer1);

	attackBuffer2.loadFromFile("Sounds/swish-2.wav");
	attackSound2.setBuffer(attackBuffer2);

	attackBuffer3.loadFromFile("Sounds/swish-3.wav");
	attackSound3.setBuffer(attackBuffer3);

	jumpBuffer1.loadFromFile("Sounds/jump1.wav");
	jumpSound1.setBuffer(jumpBuffer1);

	jumpBuffer2.loadFromFile("Sounds/jump2.wav");
	jumpSound2.setBuffer(jumpBuffer2);

	hurtBuffer1.loadFromFile("Sounds/hurt1.wav");
	hurtSound1.setBuffer(hurtBuffer1);

	hurtBuffer2.loadFromFile("Sounds/hurt2.wav");
	hurtSound2.setBuffer(hurtBuffer2);

	hurtBuffer3.loadFromFile("Sounds/hurt3.wav");
	hurtSound3.setBuffer(hurtBuffer3);

	deathBuffer1.loadFromFile("Sounds/deathSounds2/15.wav");
	deathSound1.setBuffer(deathBuffer1);

	deathSound1.setVolume(300);

	deathBuffer2.loadFromFile("Sounds/deathSounds2/16.wav");
	deathSound2.setBuffer(deathBuffer2);

	deathSound2.setVolume(300);

	rollSound1Buffer.loadFromFile("Sounds/rollSound1.mp3");
	rollSound1.setBuffer(rollSound1Buffer);

	rollSound2Buffer.loadFromFile("Sounds/rollSound2.mp3");
	rollSound2.setBuffer(rollSound2Buffer);

	swordHit1Buffer.loadFromFile("Sounds/swordHit1.mp3");
	swordHitSound1.setBuffer(swordHit1Buffer);

	swordHit2Buffer.loadFromFile("Sounds/swordHit2.mp3");
	swordHitSound2.setBuffer(swordHit2Buffer);

	swordHit3Buffer.loadFromFile("Sounds/swordHit3.mp3");
	swordHitSound3.setBuffer(swordHit3Buffer);

	parrySoundBuffer.loadFromFile("Sounds/parrySound.mp3");
	parrySound.setBuffer(parrySoundBuffer);

	dragonScreamBuffer.loadFromFile("Sounds/DragonScream.mp3");
	dragonScreamSound.setBuffer(dragonScreamBuffer);

	potionSoundBuffer.loadFromFile("Sounds/potionSound.wav");
	potionSound.setBuffer(potionSoundBuffer);

	potionSound.setVolume(100.f);

	enemeyFelledBuffer.loadFromFile("Sounds/enemeyFelled.mp3");
	enemeyFelledSound.setBuffer(enemeyFelledBuffer);
}

void Knight::setAttackBoolean()
{
	bool normalAttacking =
		state == PlayerState::Attack1 ||
		state == PlayerState::Attack2 ||
		state == PlayerState::Attack3;

	if (normalAttacking) isNormalAttacking = true;
	else if (state == PlayerState::SprintAttack) isSprintAttacking = true;
	else if (state == PlayerState::JumpAttack) isJumpAttacking = true;
}

void Knight::updateRemotePlayers(float dt, vector<Tile>& tiles)
{
	sf::Vector2f pos = playerBox.getPosition();
	pos += (networkTargetPos - pos) * 20.f * dt;

	playerBox.setPosition(pos);

	sf::Vector2f offset = animationOffset();

	currentAnimation->getSprite().setPosition(pos + offset);

	if (state == PlayerState::Walk) handleWalkSounds();

	updateDamageText(dt);

	if (!isDead)
	{

		if (isNormalAttacking || isSprintAttacking || (isJumpAttacking && !jumpAttack.isFinished()))
		{
			attackTimer += dt;
			hitboxActive = (attackTimer >= currentAttack.hitStart && attackTimer <= currentAttack.hitEnd);

		}
		else
		{
			hitboxActive = false;
		}

		if (isSpecialAttack)
		{
			guardTimer += dt;

			parryWindowActive =
				guardTimer >= guardParryStart &&
				guardTimer <= guardParryEnd;

			deflectionWindowActive =
				guardTimer >= guardDeflectionStart &&
				guardTimer <= guardDeflectionEnd;
		}
		else
		{
			parryWindowActive = false;
			deflectionWindowActive = false;
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
				state == PlayerState::SprintAttack ||
				state == PlayerState::JumpAttack ||
				state == PlayerState::Roll)
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

bool Knight::isAttackingBool()
{
	return isNormalAttacking || isSprintAttacking || isJumpAttacking || isSpecialAttack;
}

bool Knight::isSpecialAttackBool()
{
	return isSpecialAttack;
}

sf::RectangleShape Knight::getAttackBox()
{
	return this->attackBox;
}

void Knight::determineCharacterHitbox()
{
	AttackHitbox hitbox;

	if (state == PlayerState::JumpAttack) hitbox = attackBoxTable[(int)attackType::jumpAttack];
	else if (state == PlayerState::SprintAttack) hitbox = attackBoxTable[(int)attackType::sprintAttack];
	else if (state == PlayerState::Attack1 || state == PlayerState::Attack2 || state == PlayerState::Attack3) hitbox = attackBoxTable[(int)attackType::normalAttacks];
	else if (state == PlayerState::SpecialAttack) hitbox = attackBoxTable[(int)attackType::deflection];

	sf::FloatRect bounds = playerBox.getGlobalBounds();

	float centerX = bounds.position.x + bounds.size.x * 0.5f;
	float centerY = bounds.position.y + bounds.size.y * 0.5f;

	float dirSign = (lastDir == Direction::Right) ? 1.f : -1.f;

	attackBox.setSize(hitbox.size);

	attackBox.setOrigin({hitbox.size.x * 0.5f,hitbox.size.y * 0.5f});

	attackBox.setPosition({centerX + hitbox.offset.x * dirSign, centerY + hitbox.offset.y});
}

void Knight::applyDamage(int damage, Direction attackerDir)
{
	if (invulnerable)
		return;

	health -= damage;

	startKnockback(attackerDir);
	std::cout << "HEALTH: " << health << std::endl;

	if (health <= 0)
		health = 0;

	state = PlayerState::Hurt;

	invulnerable = true;
	invulTimer = 0.3f;

	if (health == 0)
	{
		state = PlayerState::Dead;
	}

}

void Knight::updateHitBoxWindow(float dt)
{
	if (invulnerable)
	{
		invulTimer -= dt;

		if (invulTimer <= 0) invulnerable = false;
	}

	if (isNormalAttacking || isSprintAttacking || isJumpAttacking)
	{
		attackTimer += dt;

		hitboxActive = (attackTimer >= currentAttack.hitStart && attackTimer <= currentAttack.hitEnd);		
	}

	if (isSpecialAttack)
	{
		guardTimer += dt;

		parryWindowActive =
			guardTimer >= guardParryStart &&
			guardTimer <= guardParryEnd;

		deflectionWindowActive =
			guardTimer >= guardDeflectionStart &&
			guardTimer <= guardDeflectionEnd;
	}
	else
	{
		parryWindowActive = false;
		deflectionWindowActive = false;
	}

	if (isKnockedback)
	{
		knockbackTimer -= dt;

		if (knockbackTimer <= 0.f)
		{
			isKnockedback = false;
			isHurt = false;
			velocity.x = 0.f; // stop horizontal movement
			state = PlayerState::Idle;
			switchAnimation(&Idle);
		}	
	}
}


void Knight::startAttack(const attackData& attack, Direction dir)
{
	cout << "KNIGHT DAMAGE: " << attack.damage << endl;

	setAttackBoolean();
	
	currentAttack = attack;       
	attackTimer = 0.0f;
	hitboxActive = false;
	hasBeenParried = false;
	lastDir = dir;
}

bool Knight::isSprintAttackingBool()
{
	return isSprintAttacking;
}


void Knight::MultiplayerDeath()
{
	//std::cout << "PLAYER DEAD" << std::endl;
	health = 0;
	state = PlayerState::Dead;

	currentAnimation = &dead;
	currentAnimation->getSprite().setScale({ 2.5f,2.5f });
}

void Knight::setKnightName(std::string& newName)
{
	this->name = newName;
	nameText.setString(name);
}

bool Knight::isGrounded()
{
	return isOnGround;
}

void Knight::jumpAttackLogic(float dt)
{
	isJumpAttacking = true;
	jumpAttackCooldown = false;
	attackTimer = 0.f;
	jumpAttackTimer = 0.f;

	state = PlayerState::JumpAttack;
	switchAnimation(&jumpAttack);

	int jumpAttackIndex = 3;

	startAttack(knightAttackTable[jumpAttackIndex], lastDir);

	currentAnimation->reset(); 
}

bool Knight::isJumpAttackingBool()
{
	return isJumpAttacking;
}

void Knight::updateBoundryBoxes()
{
	potionNumText.setString(std::to_string(potionNumber));
	sf::Vector2f base = playerBox.getPosition();

	climbingBox.setPosition(base);

	if (lastDir == Direction::Left)
	{
		attackBox.setPosition({ base.x - 90.f, base.y });
		specialAttackBox.setPosition({ base.x - 70.f, base.y });
		climbingBox.setPosition({ base.x - 15.f, base.y });
	}
	else
	{
		attackBox.setPosition({ base.x + 90.f, base.y });
		specialAttackBox.setPosition({ base.x, base.y });
	}
}

bool Knight::handleDeathLogic(float dt, std::vector<Tile>& tiles)
{
	//if (isDead) return true;

	if (health <= 0 || playerBox.getPosition().y > 2200.f)
	{
		if (!playDeathOnce)
		{
			int random = std::rand() % 2;
			(random == 0 ? deathSound1 : deathSound2).play();

			state = PlayerState::Dead;
			switchAnimation(&dead);

			health = 0;
			playDeathOnce = true;

		}

		if (currentAnimation->isFinished())
		{
			isDead = true;
			velocity.x = 0.f;

		}

		currentAnimation->getSprite().setScale({ lastDir == Direction::Right ? 2.f : -2.f, 2.f });

		currentAnimation->getSprite().setOrigin(sf::Vector2f(currentAnimation->getSprite().getLocalBounds().size.x / 2.f - 31, currentAnimation->getSprite().getLocalBounds().size.y / 2.f - 21));

		currentAnimation->setPosition(playerBox.getPosition().x, playerBox.getPosition().y);

		updateGravity(dt, tiles);

		currentAnimation->update(dt);

		return true;

	}

	return false;
}

void Knight::handleActions(InputState& input, float dt)
{
	if (tryStartNormalAttacks(input,dt)) return;
	if (tryStartJump(input)) return;
	if (tryStartJumpAttack(input, dt)) return; 
	if (tryStartSprintAttack(input)) return;
	if (tryStartRoll(input)) return;
	if (tryStartSpecial(input)) return;
	if (tryStartElixir(input)) return;
		
	if (!jumpAttackCooldown && !isNormalAttacking) handleMovement(input); // lowest priority
	
}

void Knight::attackLogicMultiplayer()
{
	isSprinting = false;
	isNormalAttacking = true;

	int randomNumber = std::rand() % 3 + 1;

	int attackID = determineAttackID();

	if (randomNumber == 1)
	{
		state = PlayerState::Attack1;
		switchAnimation(&attack1);
		attackSound1.play();

	}
	else if (randomNumber == 2)
	{
		state = PlayerState::Attack2;
		switchAnimation(&attack2);
		attackSound2.play();
	}
	else
	{
		state = PlayerState::Attack3;
		switchAnimation(&attack3);
	    attackSound3.play();
	}

	startAttack(knightAttackTable[attackID], lastDir);

	currentAnimation->reset();


	if (isNormalAttacking || isSprintAttacking)
	{
		if (isSprintAttacking)
		{
			if (lastDir == Direction::Right) velocity.x = 8.f;
			else if (lastDir == Direction::Left) velocity.x = -8.f;
		}

		if (isHurt)
			isNormalAttacking = false;

	}
}

bool Knight::tryStartJumpAttack(InputState& input, float dt)
{
	if (!isJumping) return false;
	if (!input.attackJustPressed) return false;
	if (jumpAttackCooldown || isSpecialAttack) return false;
	if (isJumpAttacking) return false;

	jumpAttackLogic(dt);
	return true;
}

void Knight::updateJumpAttack(float dt)
{
	if (!isJumpAttacking && !jumpAttackCooldown)
		return;

	if (isJumpAttacking)
	{
		jumpAttackTimer += dt;

		if (jumpAttackTimer >= jumpAttackDuration)
		{
			std::cout << "FINISHED JUMP ATTACK\n";

			isJumpAttacking = false;

			jumpAttackCooldownTimer = 0.6f;

			jumpAttackTimer = 0.f;

			currentAnimation->reset(); 
		}

		if (isOnGround)
		{
			state = PlayerState::Idle;
			velocity.x = 0.f;
			isJumpAttacking = false;
		}
	}
}

void Knight::updateJumpAttackCooldown(float dt)
{
	if (!jumpAttackCooldown)
		return;

	isJumpAttacking = false;
	jumpAttackCooldownTimer -= dt;
	
	if (isOnGround)
	{
		//currentAnimation->setFrame(jumpAttack.getFrameCount() - 1);
		velocity.x = 0.f;
	}
		
	if (jumpAttackCooldownTimer <= 0)
	{
		std::cout << "COOLDOWN" << std::endl;
		jumpAttackCooldown = false;
		currentAnimation->reset();
	}
	
}

bool Knight::tryStartSprintAttack(InputState& input)
{
	if (!input.attackJustPressed) return false;
	if (!isSprinting || isJumping || isRolling || sprintAttackCooldown) return false;

	sprintAttackLogic();
	return true;
}

void Knight::sprintAttackLogic()
{	
	switchAnimation(&runningAttack);
	state = PlayerState::SprintAttack;
    currentAnimation->reset();
	isSprinting = false;
	isSprintAttacking = true;

	int sprintAttackIndex = 2;

	startAttack(knightAttackTable[sprintAttackIndex], lastDir);

	currentAnimation->reset();
}

void Knight::updateSprintAttackCooldown(float dt)
{
	if (!sprintAttackCooldown) return;

	postSprintAttackTimer -= dt;

	if (postSprintAttackTimer <= 0.f)
	{
		sprintAttackCooldown = false;
		currentAnimation->reset();
	}

}

void Knight::updateSprintAttack()
{
	if (isSprintAttacking && currentAnimation->getCurrentFrame() == 5)
	{
		isSprintAttacking = false;
		sprintAttackCooldown = true;
		postSprintAttackTimer = 0.5f;

		state = PlayerState::Idle;
		velocity.x = 0.f;
	}
}

bool Knight::tryStartRoll(InputState& input)
{
	float timeSinceLastRoll = rollDelayClock.getElapsedTime().asSeconds();

	if (!input.rollPressed) return false;
	if (isRolling || isJumping || isSprinting || isSpecialAttack) return false;
	if (timeSinceLastRoll < minRollDelay) return false;

	rollLogic();

	rollDelayClock.restart();

	return true;
}

void Knight::rollLogic()
{
	isRolling = true;
	switchAnimation(&roll);

	int randomNumber = std::rand() % 2 + 1;

	if (randomNumber == 1)
		rollSound1.play();
	else
		rollSound2.play();

	state = PlayerState::Roll;
	currentAnimation->reset();

	rollDelayClock.reset();	
}

void Knight::updateRollLogic()
{
	if (isRolling)
	{
		if (lastDir == Direction::Left) velocity.x = -9.f;
		else if (lastDir == Direction::Right) velocity.x = 9.f;

		if (currentAnimation->isFinished())
		{
			velocity.x = 0.f;
			isRolling = false;
			state = PlayerState::Idle;
		}
	}
}

bool Knight::tryStartJump(InputState& input)
{
	if (!input.jumpJustPressed) return false;
	if (isJumping || isRolling || isNormalAttacking || isSprintAttacking || postAttackCooldown || isSpecialAttack) return false;

	jumpLogic();

	return true;
}

void Knight::jumpLogic()
{
	velocity.y = jumpStrength;
	isJumping = true;
	switchAnimation(&jump);
	state = PlayerState::Jump;
	isSprinting = false;
	currentAnimation->reset();
	isOnGround = false;

	int randomNumber = std::rand() % 2 + 1;

	if (randomNumber == 1)
		jumpSound1.play();
	else
		jumpSound2.play();
}

bool Knight::tryStartNormalAttacks(InputState& input, float dt)
{
	if (!input.attackJustPressed) return false;
	if (postAttackCooldown || isSprinting || isSprintAttacking || !isOnGround || isRolling || isJumping || isNormalAttacking || isHurt || isSpecialAttack) return false;

	attackLogicMultiplayer();

	return true;
}

void Knight::updateAttack(float dt)
{

	if (!isNormalAttacking && !isSprintAttacking) return;

	if (currentAnimation->isFinished())
	{
		postAttackCooldown = true;
		postAttackTimer = attackPauseDuration;
		isNormalAttacking = false;
		isSprintAttacking = false;
		attackTimer = 0.0f;

	}
}

void Knight::updateAttackCooldown(float dt)
{
	if (postAttackCooldown)
	{
		postAttackTimer -= dt;

		if (postAttackTimer <= 0.f)
		{
			postAttackCooldown = false;
		}
	}
}

bool Knight::tryStartSpecial(InputState& input)
{
	if (!input.specialJustPressed) return false;
	if (isSpecialAttack) return false;
	if (isJumping || isNormalAttacking || postAttackCooldown || isSprintAttacking || isSprinting || isRolling || !isOnGround) return false;
	if (postGuardCooldown) return false;

	specialAttackLogic();
	return true;
}

void Knight::specialAttackLogic()
{	
	switchAnimation(&guard);
	state = PlayerState::SpecialAttack;
	isSpecialAttack = true;
	postGuardCooldown = true;
	guardCooldownTimer = 1.5f;
	currentAnimation->reset();
	velocity.x = 0.f;
	guardTimer = 0.f;
}

void Knight::updateSpecialAttack(float dt, std::vector<Tile>& tiles)
{
	if (!postGuardCooldown) return;

	guardCooldownTimer -= dt;

	if (guardCooldownTimer <= 0.f)
	{
		postGuardCooldown = false;
	}
}

bool Knight::cancelElixir()
{
	return isJumping || isNormalAttacking || postAttackCooldown || isSprintAttacking || isSprinting || isRolling || !isOnGround;
}

void Knight::handleMovement(InputState& input)
{

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) &&
		!isNormalAttacking && !postAttackCooldown && !isSprintAttacking &&
		!blockRight && !isHanging && !isRolling && !isDrinking && !isSpecialAttack)
	{
		if (!isJumpAttacking && !jumpAttackCooldown)
		{
			lastDir = Direction::Right;
			velocity.x = walkSpeed;
		}

		if (!isJumping && !isNormalAttacking && !postAttackCooldown)
		{
			switchAnimation(&walk);
			state = PlayerState::Walk;
		}

		isSprinting = false;
		isSprintAttacking = false;

		handleWalkSounds();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) &&
			!isNormalAttacking && !isSprintAttacking && !isRolling && !isDrinking && !isJumpAttacking)
		{
			if ((!isJumping && !blockLeft) || (!isJumping && !blockRight) || !isJumping && !isRolling)
			{
				switchAnimation(&run);
				state = PlayerState::Run;
			}

			velocity.x = runSpeed;
			isSprinting = true;
		}
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) &&
		!postAttackCooldown && !isSprintAttacking &&
		!blockLeft && !isHanging && !isRolling && !isDrinking && !isSpecialAttack)
	{
		if (!isJumpAttacking && !jumpAttackCooldown)
		{
			lastDir = Direction::Left;
			velocity.x = -walkSpeed;
		}

		isSprinting = false;
		isSprintAttacking = false;

		if (!isJumping && !isNormalAttacking && !postAttackCooldown)
		{
			switchAnimation(&walk);
			state = PlayerState::Walk;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) &&
			!isNormalAttacking && !isSprintAttacking && !blockLeft &&
			!isRolling && !isDrinking && !isJumpAttacking)
		{
			if (!isJumping && !isRolling)
			{
				switchAnimation(&run);
				state = PlayerState::Run;
			}

			velocity.x = -runSpeed;
			isSprinting = true;
		}

		handleWalkSounds();
	}
	else if (!isSpecialAttack && !isJumpAttacking && !isRolling && ((!isJumping && !isNormalAttacking && !postAttackCooldown && !isSprinting && !isSprintAttacking && !isDrinking && !isSpecialAttack) || (blockLeft && !isJumpAttacking) || (blockRight && !isJumpAttacking) || isSprinting || isClimbing))
	{
		switchAnimation(&Idle);
		state = PlayerState::Idle;
		isSprinting = false;
		isSprintAttacking = false;
		isSpecialAttack = false;
		velocity.x = 0.f;
	}

}

sf::Vector2f Knight::animationOffset()
{
	sf::Vector2f result = { 0.f,0.f };
	sf::Vector2f leftOffset;
	sf::Vector2f rightOffset;

	if (state == PlayerState::JumpAttack)
	{
		leftOffset = { 100.f,0.f };
		rightOffset = { -100.f,0.f };

		result = lastDir == Direction::Right ? rightOffset : leftOffset;
	}
	else if (state == PlayerState::Roll)
	{
		leftOffset = { 75.f,0.f };
		rightOffset = { -75.f,0.f };

		result = lastDir == Direction::Right ? rightOffset : leftOffset;
	}
	else if (state == PlayerState::Dead)
	{
		leftOffset = { 75.f,-82.f };
		rightOffset = { -75.f,-82.f };

		result = lastDir == Direction::Right ? rightOffset : leftOffset;
	}
	else if (state == PlayerState::SpecialAttack || state == PlayerState::Drink)
	{
		leftOffset = { 40.f,0.f };
		rightOffset = { -40.f,0.f };

		result = lastDir == Direction::Right ? rightOffset : leftOffset;
	}
	else if (state == PlayerState::Stunned)
	{
		leftOffset = { 35.f,-82.f };
		rightOffset = { -75.f,-82.f };
		result = lastDir == Direction::Right ? rightOffset : leftOffset;
	}

	return result;
}

void Knight::applyStun(float duration)
{
	isStunned = true;

	cancelAttack();

	stunTimer = duration;
	velocity = { 0.f, 0.f };

	state = PlayerState::Stunned;

	switchAnimation(&dead);   
	currentAnimation->reset();

	int scalar = lastDir == Direction::Right ? 1.f : -1.f;

	currentAnimation->setScale(2.f * scalar, 2.f);

	stunFrameTimer = 0.f;     
	stunPhase = 0;            // 0 = going to frame 2, 1 = holding, 2 = reversing
}

void Knight::setHealth(int healNum)
{
	health = healNum;

	if (health > 100) health = 100;
}

void Knight::setHealBoolean(bool x)
{
	healed = x;
}


bool Knight::isParryWindow()
{
	return state == PlayerState::SpecialAttack && parryWindowActive;
}

bool Knight::isDeflectionWindow()
{
	return state == PlayerState::SpecialAttack;
}

AttackHitbox* Knight::getAttackBoxTable()
{
	return attackBoxTable;
}

const attackData* Knight::getAttackTable() const
{
	return knightAttackTable;
}

void Knight::cancelAttack()
{
	isNormalAttacking = false;
	isSprintAttacking = false;
	isJumpAttacking = false;
	isSpecialAttack = false;
	//hitboxActive = false;
	attackTimer = 0.f;
}

void Knight::resetCharacter(bool isSinglePlayer)
{
	isSprintAttacking = false;
	postAttackCooldown = false;

	isSinglePlayer ? potionNumber = 5 : potionNumber = 1;

	jumpCount = 0;

	playDeathOnce = false;
	playOnce2 = playOnce3 = playOnce4 = playOnce5 = true;

	postAttackTimer = 0.f;

	switchAnimation(&Idle);
	currentAnimation->reset();
	dead.reset();
}

int Knight::determineAttackID()
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
			attackID = 1;
			break;
		case PlayerState::SprintAttack:
			attackID = 2;
			break;
		case PlayerState::JumpAttack:
			attackID = 3;
			break;
		case PlayerState::SpecialAttack:
			attackID = 4;
			break;
		default:
			attackID = 0;

	}

	return attackID;
}

float Knight::getGuardTimer()
{
	return guardTimer;
}

void Knight::enemyKnightCollision(std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<Arrow>& arrows, float dt)
{
	if (isAttackingBool() && isHitboxActive())
	{
		for (auto& example : enemies)
		{
			if (attackBox.getGlobalBounds().findIntersection(example->getBounds()))
			{
				example->isHurtTrue();
			}
			else
			{
				example->isHurtFalse();
			}
		}
	}

	for (auto& arrow : arrows)
	{
		if (arrow.getArrowOnTile()) continue;

		// Check if arrow is in front of knight based on direction
		bool arrowIsComingFromFront =
			(lastDir == Direction::Right && arrow.velocity.x < 0) ||
			(lastDir == Direction::Left && arrow.velocity.x > 0);

		if (specialAttackBox.getGlobalBounds().findIntersection(arrow.getBounds()) && isSpecialAttack && arrowIsComingFromFront)
		{
			if (!shieldSoundPlayed)
			{
				int random = std::rand() % 2;
				(random == 0 ? shieldImpactSound1 : shieldImpactSound2).play();
				shieldSoundTimer.restart();
				shieldSoundPlayed = true;
			}

			arrow.velocity = { 0.f, 0.f };
			arrow.setArrowOnTile(true);
		}
		else if (playerBox.getGlobalBounds().findIntersection(arrow.getBounds()))
		{
			
			isHurt = true;
			state = PlayerState::Hurt;
			isKnockedback = true;
			knockbackTimer = 0.8f;

			health -= 30;

			arrow.velocity = { 0.f, 0.f };
			arrow.setArrowOnTile(true);
		}
	}

	for (auto& enemy : enemies)
	{
		if (enemy->isDead())
			continue;

		if (specialAttackBox.getGlobalBounds().findIntersection(enemy->getAttackBoxBounds()) && isSpecialAttack)
		{
			enemy->hitsShieldTrue();

			if (enemy->getCurrentEnemyAnimation()->isFinished())
			{
				if (!shieldSoundPlayed)
				{
					int random = std::rand() % 2;
					(random == 0 ? shieldImpactSound1 : shieldImpactSound2).play();
					shieldSoundTimer.restart();
					shieldSoundPlayed = true;
				}
			}
		}
		else if (playerBox.getGlobalBounds().findIntersection(enemy->getBounds()) || playerBox.getGlobalBounds().findIntersection(enemy->getAttackBoxBounds()))	
		{
			if (!isRolling && !isKnockedback)
			{
				cout << "INSIDE DAMAGE ENEMEY" << endl;
				enemy->knightDamagedTrue();
				state = PlayerState::Hurt;
				isHurt = true;
				isKnockedback = true;
				knockbackTimer = 0.8f;

				
					health -= 40;
					playOnce3 = false;
				

				std::cout << health << std::endl;


				if (enemy->getDirection() == enemyDirection::Right)
					velocity.x = 8.f;
				else
					velocity.x = -8.f;
			}
		}
		else if (knockbackTimer <= 0.f)
		{
			enemy->knightDamagedFalse();
		}


	 if (playerBox.getGlobalBounds().findIntersection(enemy->getIninitializerBox()) || doOnce)
	 {
			enemy->setInitializerBox(true);

			doOnce = true;

			if (enemy->getCurrentEnemyAnimation()->isFinished())
			{
				enemy->setInitializerBox(false);

				doOnce = false;
			}

	 }
	 
	}

	if (isKnockedback)
	{
		knockbackTimer -= dt;

		int randomNumber = std::rand() % 3 + 1;

			if (isHurt && health > 0)
			{
				switchAnimation(&hurt);

				if (hurtSoundClock.getElapsedTime() >= hurtInterval && !playSoundOnce)
				{

					if (randomNumber == 1)
					{
						hurtSound1.play();
					}
					else if (randomNumber == 2)
					{
						hurtSound2.play();
					}
					else
					{
						hurtSound3.play();
					}

					hurtSoundClock.restart();
				}

			}

		
		if (knockbackTimer <= 0.f)
		{
			velocity.x = 0.f;
			isKnockedback = false;
			knightWolfCollision = false;
			playSoundOnce = false;
			isHurt = false;
			playOnce3 = true;


			switchAnimation(&Idle);
			
		}

	}

	if (shieldSoundPlayed && shieldSoundTimer.getElapsedTime().asSeconds() > shieldSoundCooldown)
	{
		shieldSoundPlayed = false;
	}

}

void Knight::attackLogic(std::vector<std::unique_ptr<Enemy>>& enemies, float dt)
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !postAttackCooldown && !isSprinting && !isSprintAttacking && isOnGround && !blockLeft && !blockRight && !isRolling && !isJumping && !isNormalAttacking && !isHurt) {

		isSprinting = false;

		//std::cout << "Attack" << std::endl;
		isNormalAttacking = true;

		if (isNormalAttacking) // if its false
		{
			int randomNumber = std::rand() % 3 + 1;

			if (randomNumber == 1)
			{
				switchAnimation(&attack1);
				attackSound1.play();

			}
			else if (randomNumber == 2)
			{
				switchAnimation(&attack2);
				attackSound2.play();
			}
			else
			{
				switchAnimation(&attack3);
				attackSound3.play();
			}


			currentAnimation->reset();
		

		}

	}

	if (isNormalAttacking || isSprintAttacking || isJumpAttacking)
	{
		if (isSprintAttacking)
		{

			if (lastDir == Direction::Right)
			{

				velocity.x = 8.f;
			}
			else if (lastDir == Direction::Left)
			{
				velocity.x = -8.f;
			}

		}

		if (isHurt)
			isNormalAttacking = false;

		if (currentAnimation->isFinished())
		{

			postAttackCooldown = true;
			postAttackTimer = attackPauseDuration;
			isNormalAttacking = false;
			isSprintAttacking = false;



		}
	}
	if (postAttackCooldown) {
		postAttackTimer -= dt;
		if (postAttackTimer <= 0.f) {
			postAttackCooldown = false;
		}
	}
}

void Knight::lureLogic(std::vector<std::unique_ptr<Enemy>>& enemies)
{	
		for (auto& enemy : enemies)
		{

			if (!enemy->isDead())
			{
				bool inRightLure = false;
				bool inLeftLure = false;

				if (playerBox.getGlobalBounds().findIntersection(enemy->getEnemyRightLure()))
				{
					inRightLure = true;
				}

				if (playerBox.getGlobalBounds().findIntersection(enemy->getEnemyLeftLure()))
				{
					inLeftLure = true;
				}


				if (inRightLure)
				{
					enemy->setEnemyRightLure(true);
					enemy->setEnemyLeftLure(false); 
					enemy->setDirection(enemyDirection::Right);
				}
				else if (inLeftLure)
				{
					enemy->setEnemyLeftLure(true);
					enemy->setEnemyRightLure(false);
					enemy->setDirection(enemyDirection::Left);
				}
				else
				{



					// Knight has left both lure zones
					enemy->setEnemyLeftLure(false);
					enemy->setEnemyRightLure(false);

					
				}
			}
		}
	


}

bool Knight::Death()
{
	return isDead;
}

sf::RectangleShape Knight::getKnightBox()
{
	return this->playerBox;
}

void Knight::climbingLogic(std::vector<Tile>& tiles)
{
	for (auto& tile : tiles)
	{
		if (climbingBox.getGlobalBounds().findIntersection(tile.ledgeGrabBox) && !isOnGround && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
		{
			sf::Vector2f hangPos;

			if (tile.getID() == 32 || tile.getID() == 2)
			{
				hangPos = { tile.ledgeGrabBox.position.x + tile.ledgeGrabBox.size.x + 40, tile.ledgeGrabBox.position.y };
			}
			else if (tile.getID() == 30 || tile.getID() == 0)
			{
				hangPos = { tile.ledgeGrabBox.position.x + tile.ledgeGrabBox.size.x - 55, tile.ledgeGrabBox.position.y };
			}
			else
			{
				continue;
			}

			playerBox.setPosition(hangPos);

			isHanging = true;
		}
	}


}

void Knight::pullUpLogic()
{

	if (isHanging && !isClimbing && currentAnimation != &hanging)
	{
		/*switchAnimation(&hanging);
		state = PlayerState::Hang;
		*/
		if (playOnce2)
		{
			hangSound1.play();
			playOnce2 = false;
		}

	}

	if (isHanging && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
	{
		isClimbing = true;

	}

	if (isClimbing)
	{
		switchAnimation(&climbing);
		state = PlayerState::Climb;

		playerBox.move({ 0.f,-5.2f });

		if (currentAnimation->isFinished())
		{
			sf::Vector2f offset = (lastDir == Direction::Left) ? sf::Vector2f(-40.f, 0.f) : sf::Vector2f(40.f, 0.f);
			playerBox.move(offset);

			isHanging = false;
			isClimbing = false;
			playOnce2 = true;
			std::cout << "DONE";
			currentAnimation->reset();
		}
	}
	
}



void Knight::resetKnight(int x)
{
	std::cout << "Resetting Knight\n";
	health = 100;

	if (x == 0)
	{
		playerBox.setPosition({ 400.f,1825.f });
		currentAnimation->getSprite().setPosition({ 400.f,1825.f });

		potionNumber = 5;
	}
	else
	{
		potionNumber = 1;
	}
	
	velocity.x = 0.f;
	
	isNormalAttacking = false;
	postAttackCooldown = false;
	isSprinting = false;
	isSprintAttacking = false;
	isSpecialAttack = false;
	isJumping = false;
	isDead = false;
	playDeathOnce = false;
	wasWPressedLastFrame = false;
	isClimbing = false;
	isHurt = false;
	isKnockedback = false;
	blockLeft = false;
	blockRight = false;
	touch = false;
	isRolling = false;
	isHanging = false;

	postAttackTimer = 0.f;
	knockbackTimer = 0.f;
	jumpCount = 0;
	wPressed = 0;
	playOnce2 = playOnce3 = playOnce4 = playOnce5 = true;

	switchAnimation(&Idle);
	currentAnimation->reset();
	
	dead.reset();


	lastDir = Direction::Right;
	clock2.restart();
	std::cout << "Resetting Knight\n";
	

}


