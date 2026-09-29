#pragma once
#include "Enemy.hpp"
#include "Animation.hpp"
#include "Tile.hpp"
#include <memory>

class wolf : public Enemy
{
public:

	wolf(sf::Texture& walk2, sf::Texture& textureAttack1, sf::Texture& textureAttack2, sf::Texture& textureAttack3, sf::Texture& textureDeath, sf::Texture& textureHurt, float x, float y) :
		Enemy(maxHealth),
		hurtSound1(hurtBuffer1),
		hurtSound2(hurtBuffer2),
		deathSound(deathBuffer)
	{
		if (!hurtBuffer1.loadFromFile("Sounds/NEWwolfHurt2.wav"))
			std::cerr << "Failed to load NEWwolfHurt2.wav\n";
		hurtSound1.setBuffer(hurtBuffer1);

		if (!hurtBuffer2.loadFromFile("Sounds/wolfHurt1.wav"))
			std::cerr << "Failed to load wolfHurt1.wav\n";
		hurtSound2.setBuffer(hurtBuffer2);

		if (!deathBuffer.loadFromFile("Sounds/wolfDeath.wav"))
			std::cerr << "Failed to load wolfDeath.wav\n";
		deathSound.setBuffer(deathBuffer);

		walk = std::make_unique<Animation>(walk2, 11, 1, 0.1f, 0, false, true);
		attack1 = std::make_unique<Animation>(textureAttack1, 6, 1, 0.1f, 0, false, false);
		attack2 = std::make_unique<Animation>(textureAttack2, 4, 1, 0.15f, 0, false, false);
		attack3 = std::make_unique<Animation>(textureAttack3, 5, 1, 0.1f, 0, false, false);
		death = std::make_unique<Animation>(textureDeath, 2, 1, 0.15f, 0, false, false);
		hurt = std::make_unique<Animation>(textureHurt, 2, 1, 0.2f, 0, false, true);
		currentAnimation2 = walk.get();

		currentAnimation2->getSprite().setPosition({ x,y });
		wolfBox.setFillColor(sf::Color(255, 0, 0, 128));
		wolfBox.setSize({ 40.f,40.f });
		wolfBox.setScale({ 2,3 });
		wolfBox.setOrigin({ 20,42 });
		currentAnimation2->getSprite().setScale({ 1.5,1.5 });

		wolfAttackBox.setFillColor(sf::Color(255, 0, 0, 128));
		wolfAttackBox.setSize({ 30.f,30.f });
		wolfAttackBox.setScale({ 2,3 });
		wolfAttackBox.setOrigin({ 15,42 });

		rightLureBox.setSize({ 300.f, 10.f });
		rightLureBox.setScale({ 2,2.5 });
		rightLureBox.setOrigin({ -20.f, 30.f });
		rightLureBox.setFillColor(sf::Color(255, 0, 0, 128));

		leftLureBox.setSize({ -300.f, 10.f });
		leftLureBox.setScale({ 2,2.5 });
		leftLureBox.setOrigin({ 20.f, 30.f });
		leftLureBox.setFillColor(sf::Color(255, 0, 0, 128));

		sf::Sprite& walkSprite = walk->getSprite();
		walkSprite.setOrigin({ walkSprite.getLocalBounds().size.x * 0.5f, walkSprite.getLocalBounds().position.x + 128.f });

		sf::Sprite& attackSprite = attack1->getSprite();
		attackSprite.setOrigin({ attackSprite.getLocalBounds().size.x * 0.5f, attackSprite.getLocalBounds().position.x + 128.f });

		sf::Sprite& attack2Sprite = attack2->getSprite();
		attack2Sprite.setOrigin({ attack2Sprite.getLocalBounds().size.x * 0.5f, attack2Sprite.getLocalBounds().position.x + 128.f });

		sf::Sprite& attack3Sprite = attack3->getSprite();
		attack3Sprite.setOrigin({ attack3Sprite.getLocalBounds().size.x * 0.5f, attack3Sprite.getLocalBounds().position.x + 128.f });

		sf::Sprite& deathSprite = death->getSprite();
		deathSprite.setOrigin({ deathSprite.getLocalBounds().size.x * 0.5f, deathSprite.getLocalBounds().position.x + 128.f });

		sf::Sprite& hurtSprite = hurt->getSprite();
		hurtSprite.setOrigin({ hurtSprite.getLocalBounds().size.x * 0.5f, hurtSprite.getLocalBounds().position.x + 128.f });


		lastDir = enemyDirection::Right;

	}

	//walk.getSprite().setOrigin()

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

	enemyDirection getDirection() override;

	void setDirection(enemyDirection newDir) override;

	void initializeSounds();

	void hitsShieldTrue();

	Animation* getCurrentEnemyAnimation() override;

	bool setEnemyRightLure(bool value) override;

	bool setEnemyLeftLure(bool value) override;

	int getHealth() override;

	sf::FloatRect getEnemyRightLure() override;
	sf::FloatRect getEnemyLeftLure() override;

	bool setInitializerBox(bool value) override;
	sf::FloatRect getIninitializerBox() override;

	bool pendingShieldKnockback = false;
	Animation* currentAnimation2;
	sf::Vector2f velocity = { 3.f, 0.f };
	enemyDirection lastDir;

private:

	std::unique_ptr<Animation> walk;
	std::unique_ptr<Animation> attack1;
	std::unique_ptr<Animation> attack2;
	std::unique_ptr<Animation> attack3;
	std::unique_ptr<Animation> death;
	std::unique_ptr<Animation> hurt;

	sf::RectangleShape wolfBox;
	sf::RectangleShape wolfAttackBox;
	sf::RectangleShape leftLureBox;
	sf::RectangleShape rightLureBox;

	bool active = false;
	bool isKnockedBack = false;
	bool knightDamaged = false;
	bool wolfDeath = false;
	bool playAnimationOnce = false;
	bool dead = false;
	float knockbackTimer = 0.f;
	float attackCooldown = 0.f;
	float attackDelay = 1.0f;
	bool right = true;
	int hitsToKill = 0;
	int health = 100;
	int maxHealth = 100;

	bool wolfHurt = false;
	bool wolfHitsShield = false;
	bool isAttacking = false;

	bool rightLure = false;
	bool leftLure = false;

	sf::SoundBuffer hurtBuffer1;
	sf::SoundBuffer hurtBuffer2;
	sf::SoundBuffer deathBuffer;

	sf::Sound hurtSound1;
	sf::Sound hurtSound2;
	sf::Sound deathSound;
	
	bool playOnce = false;

	void switchAnimation2(Animation* newAnim) {

		if (currentAnimation2 != newAnim) {
			sf::Vector2f pos = currentAnimation2->getSprite().getPosition();
			currentAnimation2 = newAnim;
			currentAnimation2->getSprite().setPosition(pos);
		}
	}
};
