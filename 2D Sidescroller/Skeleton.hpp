#pragma once
#include "Enemy.hpp"
#include "Tile.hpp"
#include "Animation.hpp"
#include "Arrow.hpp"


class Skeleton : public Enemy
{

public:

	Skeleton(sf::Texture& idleTexture, sf::Texture& shot1Texture, sf::Texture& arrowTexture, sf::Texture& hurtTexture, sf::Texture& deadTexture, float x, float y, int arrowGravityLevel) :
		Enemy(maxHealth),
		arrowTex(arrowTexture),
		bowShotSound(bowShotBuffer),
		bowPullbackSound(bowPullbackBuffer),
		boneCrackSound(boneCrackBuffer),
		DeathSound(DeathBuffer)
	{
		changedGravity = arrowGravityLevel;

		Idle = std::make_unique<Animation>(idleTexture, 7, 1, 0.2f, 0, false, true);

		Shot1 = std::make_unique<Animation>(shot1Texture, 15, 1, 0.1f, 0, false, false);

		Hurt = std::make_unique<Animation>(hurtTexture, 2, 1, 0.1f, 0, false, false);

		Dead = std::make_unique<Animation>(deadTexture, 5, 1, 0.1f, 0, false, false);

		currentAnimation = Idle.get();

		currentAnimation->getSprite().setPosition({ x,y });

		currentAnimation->getSprite().setScale({ -2,2 });

		skeletonBox.setFillColor(sf::Color(255, 0, 0, 128));
		skeletonBox.setSize({ 40.f,40.f });
		skeletonBox.setScale({ 2,3 });
		skeletonBox.setOrigin({ 20,42 });

         
		 rightLureBox.setFillColor(sf::Color(255, 0, 0, 128));

		
		 leftLureBox.setFillColor(sf::Color(255, 0, 0, 128));

		if (arrowGravityLevel == 2)
		{
			rightLureBox.setSize({ 600.f, 50.f });
			rightLureBox.setScale({ 2,2.5 });
			rightLureBox.setOrigin({ -20.f, 50.f });
			
			leftLureBox.setSize({ -600.f, 50.f });
			leftLureBox.setScale({ 2,2.5 });
			leftLureBox.setOrigin({ 20.f, 50.f });
			
		}
		else if (arrowGravityLevel == 1)
		{
			rightLureBox.setSize({ 600.f, 50.f });
			rightLureBox.setScale({ 2,6 });
			rightLureBox.setOrigin({ -20.f, 20.f });

			leftLureBox.setSize({ -600.f, 50.f });
			leftLureBox.setScale({ 2,6 });
			leftLureBox.setOrigin({ 20.f, 20.f });

		}
		bowShotBuffer.loadFromFile("Sounds/BowShotSound.mp3");
		bowShotSound.setBuffer(bowShotBuffer);

		bowPullbackBuffer.loadFromFile("Sounds/BowPullback.mp3");
		bowPullbackSound.setBuffer(bowPullbackBuffer);

		boneCrackBuffer.loadFromFile("Sounds/BoneCrack2.mp3");
		boneCrackSound.setBuffer(boneCrackBuffer);

		DeathBuffer.loadFromFile("Sounds/SkeletonDeathSound.mp3");
		DeathSound.setBuffer(DeathBuffer);


		lastDir = enemyDirection::Right;

		sf::Sprite& IdleSprite = Idle->getSprite();
		IdleSprite.setOrigin({ IdleSprite.getLocalBounds().size.x * 0.5f, IdleSprite.getLocalBounds().position.x + 128.f });

		sf::Sprite& shot1Sprite = Shot1->getSprite();
		shot1Sprite.setOrigin({ shot1Sprite.getLocalBounds().size.x * 0.5f, shot1Sprite.getLocalBounds().position.x + 128.f });

		sf::Sprite& hurtSprite = Hurt->getSprite();
		hurtSprite.setOrigin({ hurtSprite.getLocalBounds().size.x * 0.5f, hurtSprite.getLocalBounds().position.x + 128.f });

		sf::Sprite& deadSprite = Dead->getSprite();
		deadSprite.setOrigin({ deadSprite.getLocalBounds().size.x * 0.5f, deadSprite.getLocalBounds().position.x + 128.f });


	}

	void update(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>& enemies) override;
	void draw(sf::RenderWindow& window) override;
	sf::FloatRect getBounds() override;
	sf::FloatRect getAttackBoxBounds() override;
	bool isDead() override;
	int getHealth() override;
	void checkCollision(std::vector<Tile>& tiles) override;
	void checkForCliff(std::vector<Tile>& tiles) override;
	void isHurtTrue() override;
	void isHurtFalse() override;
	void hitsShieldTrue() override;
	Animation* getCurrentEnemyAnimation() override;
	void knightDamagedTrue() override;
	void knightDamagedFalse() override;
	enemyDirection getDirection() override;
	void setDirection(enemyDirection newDir) override;
	bool setEnemyLeftLure(bool value) override;
	bool setEnemyRightLure(bool value) override;

	 sf::FloatRect getEnemyRightLure() override;
	sf::FloatRect getEnemyLeftLure() override;
	bool setInitializerBox(bool value) override;
	sf::FloatRect getIninitializerBox() override;

	void shootArrow(std::vector<Arrow>& arrows, std::vector<Tile>& tiles, float dt);

private:

	std::unique_ptr<Animation> Idle;
	std::unique_ptr<Animation> Shot1;
	std::unique_ptr<Animation> Hurt;
	std::unique_ptr<Animation> Dead;

	Animation* currentAnimation;

	sf::RectangleShape skeletonBox;
	sf::RectangleShape leftLureBox;
	sf::RectangleShape rightLureBox;
	sf::Texture& arrowTex;

	enemyDirection lastDir;

	sf::SoundBuffer bowShotBuffer;
	sf::Sound bowShotSound;

	sf::SoundBuffer bowPullbackBuffer;
	sf::Sound bowPullbackSound;

	sf::SoundBuffer boneCrackBuffer;
	sf::Sound boneCrackSound;

	sf::SoundBuffer DeathBuffer;
	sf::Sound DeathSound;

	bool leftLure = false;
	bool rightLure = true;

	int health = 60;
	int maxHealth = 60;
	bool dead = false;
	bool isHurt = false;
	bool arrowFired = false;
	bool playOnce = false;
	bool playSoundOnce = false;
	int changedGravity = 0.f;

	void switchAnimation(Animation* newAnim) {

		if (currentAnimation != newAnim) {
			sf::Vector2f pos = currentAnimation->getSprite().getPosition();
			currentAnimation = newAnim;
			currentAnimation->getSprite().setPosition(pos);
		}

		
	}



};