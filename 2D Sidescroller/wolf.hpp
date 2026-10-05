#pragma once
#include "Enemy.hpp"
#include "Animation.hpp"
#include "Tile.hpp"
#include "AssetManager.hpp"
#include <memory>
#include <iostream>

class wolf : public Enemy
{
public:

	wolf(const WolfTextures& t, const sf::Font& standardFont, float x, float y) :
		Enemy(kMaxHealth),
		hurtSound1(hurtBuffer1),
		hurtSound2(hurtBuffer2),
		deathSound(deathBuffer),
		chargeSound(chargeBuffer),
		suspenseSound(suspenseBuffer)
	{

		damageFontText = standardFont;
		loadSound(hurtBuffer1, hurtSound1, "Sounds/NEWwolfHurt2.wav");
		loadSound(hurtBuffer2, hurtSound2, "Sounds/wolfHurt1.wav");
		loadSound(deathBuffer, deathSound, "Sounds/wolfDeath.wav");
		loadSound(chargeBuffer, chargeSound, "Sounds/wolfChargeSound.mp3");
		loadSound(suspenseBuffer, suspenseSound, "Sounds/wolfSuspenseSound.mp3");

		// texture, frames, rows, frameTime, ?, ?, loop
		idleAnim = std::make_unique<Animation>(t.wolfIdleTex, 8, 1, 0.2f, 0, false, true); // <-- set the real frame count
		walk = std::make_unique<Animation>(t.wolfWalkTex, 11, 1, 0.10f, 0, false, true);
		attack1 = std::make_unique<Animation>(t.wolfAttack1Tex, 6, 1, 0.10f, 0, false, false);
		attack2 = std::make_unique<Animation>(t.wolfAttack2Tex, 4, 1, 0.15f, 0, false, false);
		attack3 = std::make_unique<Animation>(t.wolfAttack3Tex, 5, 1, 0.10f, 0, false, false);
		death = std::make_unique<Animation>(t.wolfDeathTex, 2, 1, 0.15f, 0, false, false);
		hurt = std::make_unique<Animation>(t.wolfHurtTex, 2, 1, 0.20f, 0, false, true);
		sprintAnim = std::make_unique<Animation>(t.wolfSprintTex, 9, 1, 0.1f, 0, false, true);
		sprintAttackAnim = std::make_unique<Animation>(t.wolfSprintAttackTex, 7, 1, 0.05f, 0, false, false);

		for (Animation* a : { idleAnim.get(), walk.get(), attack1.get(), attack2.get(), attack3.get(),
							  death.get(), hurt.get(), sprintAnim.get(), sprintAttackAnim.get() })
		{
			sf::Sprite& s = a->getSprite();
			s.setOrigin({ s.getLocalBounds().size.x * 0.5f, s.getLocalBounds().position.x + 128.f });
		}

		currentAnimation = idleAnim.get();
		currentAnimation->getSprite().setPosition({ x, y });
		currentAnimation->getSprite().setScale({ kSpriteScale, kSpriteScale });

		setupBox(wolfBox, { 40.f, 40.f }, { 2.f, 3.f }, { 20.f, 42.f });
		setupBox(wolfAttackBox, { 30.f, 30.f }, { 2.f, 3.f }, { 15.f, 42.f });
		setupBox(rightLureBox, { 400.f, 10.f }, { 2.f, 2.5f }, { -20.f, 30.f });
		setupBox(leftLureBox, { -400.f, 10.f }, { 2.f, 2.5f }, { 20.f, 30.f });

		lastDir = enemyDirection::Right;
		state = State::Idle; // starts standing still until the knight enters a lure box
	}

	void draw(sf::RenderWindow& window);
	void update(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>& enemies) override;

	void checkCollision(std::vector<Tile>& tiles) override;
	void checkForCliff(std::vector<Tile>& tiles) override;

	sf::FloatRect getBounds() override;
	sf::FloatRect getAttackBoxBounds() override;

	void setWolfVelocity(float x);

	void knightDamagedTrue() override;
	void knightDamagedFalse() override;

	bool isDead();
	void isHurtTrue();
	void isHurtFalse();
	void hitsShieldTrue();

	enemyDirection getDirection() override;
	void setDirection(enemyDirection newDir) override;

	Animation* getCurrentEnemyAnimation() override;

	bool setEnemyRightLure(bool value) override;
	bool setEnemyLeftLure(bool value) override;

	int getHealth() override;

	sf::FloatRect getEnemyRightLure() override;
	sf::FloatRect getEnemyLeftLure() override;

	bool setInitializerBox(bool value) override;
	sf::FloatRect getIninitializerBox() override;

	// True while the current attack is the sprint lunge (for bonus damage/knockback).
	bool isSprintAttacking() const { return state == State::Attack && sprintAttacking; }

	bool pendingShieldKnockback = false;
	Animation* currentAnimation;
	sf::Vector2f velocity = { 0.f, 0.f };
	enemyDirection lastDir;

private:

	// ---- tuning -------------------------------------------------------
	static constexpr int   kMaxHealth = 100;
	static constexpr float kSpriteScale = 1.5f;
	static constexpr float kWalkSpeed = 3.f;
	static constexpr float kSprintSpeed = 16.f;
	static constexpr float kKnockbackSpeed = 5.f;
	static constexpr float kWindupTime = 0.8f;
	static constexpr float kSprintMaxTime = 3.0f;
	static constexpr float kRecoverTime = 0.6f;
	static constexpr float kLungeDecay = 30.f;  // px/frame lost per second
	static constexpr int   kSprintChancePct = 75;
	static constexpr float kHurtKnockback = 0.3f;
	static constexpr float kShieldKnockback = 0.5f;
	static constexpr int   kHurtDamage = 20;
	static constexpr float kTurnPauseTime = 0.7f;

	//  Idle      spawn state, stands still until the knight enters a lure box
	//  Patrol    walks back and forth (knight is out of range)
	//  Approach  knight in lure box: walks up and attacks normally
	//  Windup    knight in lure box: telegraph before the sprint
	//  Sprint    charges the knight
	//  Attack    attack animation (sprint attack keeps lunging)
	//  Recover   short pause after a sprint
	//  Knockback pushed back by a hit or a blocked attack
	//  Dead
	enum class State { Idle, Patrol, Approach, Windup, Sprint, Attack, Turn, Recover, Knockback, Dead };
	State state = State::Idle;

	std::unique_ptr<Animation> idleAnim, walk, attack1, attack2, attack3, death, hurt, sprintAnim, sprintAttackAnim;
	sf::RectangleShape wolfBox, wolfAttackBox, leftLureBox, rightLureBox;

	bool knightDamaged = false;
	bool wolfHurt = false;
	bool wolfHitsShield = false;
	bool sprintAttacking = false;
	bool rightLure = false;
	bool leftLure = false;
	bool playOnce = false;          // damage/sound applied for the current hit
	bool playAnimationOnce = false; // death started
	bool dead = false;
	bool sprintOnce = false;

	float knockbackTimer = 0.f;
	float attackCooldown = 0.f;
	float attackDelay = 1.0f;
	float windupTimer = 0.f;
	float sprintTimer = 0.f;
	float turnTimer = 0.f;
	float recoverTimer = 0.f;
	float lungeSpeed = 0.f;

	int health = kMaxHealth;
	int maxHealth = kMaxHealth;

	sf::SoundBuffer hurtBuffer1, hurtBuffer2, deathBuffer, chargeBuffer, suspenseBuffer;
	sf::Sound hurtSound1, hurtSound2, deathSound, chargeSound, suspenseSound;

	// ---- helpers ------------------------------------------------------
	float dirSign() const { return lastDir == enemyDirection::Right ? 1.f : -1.f; }
	bool  inLure() const { return rightLure || leftLure; }
	bool  inContact() const { return knightDamaged || wolfHitsShield; }

	void updateBoundryBoxes();
	bool handleDeathLogic(float dt);
	void handleHurtLogic();
	void applyFacing();
	void faceKnight();
	bool groundAhead(std::vector<Tile>& tiles);

	void engageKnight();   // knight entered lure: pick normal approach or sprint
	void startSprint();
	void startAttack(bool sprinting);
	void finishAttack();
	void startRecover(float time);
	void startKnockback(float time);
	void turnAround();
	bool knightBehind();


	void switchAnimation(Animation* newAnim)
	{
		if (currentAnimation != newAnim)
		{
			sf::Vector2f pos = currentAnimation->getSprite().getPosition();
			currentAnimation = newAnim;
			currentAnimation->getSprite().setPosition(pos);
		}
	}

	static void setupBox(sf::RectangleShape& box, sf::Vector2f size, sf::Vector2f scale, sf::Vector2f origin)
	{
		box.setSize(size);
		box.setScale(scale);
		box.setOrigin(origin);
		box.setFillColor(sf::Color(255, 0, 0, 128));
	}

	static void loadSound(sf::SoundBuffer& buffer, sf::Sound& sound, const char* path)
	{
		if (!buffer.loadFromFile(path))
			std::cerr << "Failed to load " << path << "\n";
		sound.setBuffer(buffer);
	}
};