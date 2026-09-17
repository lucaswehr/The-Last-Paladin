#pragma once
#include "Animation.hpp"
#include "Enemy.hpp"
#include "Healthbar.hpp"
#include "text.hpp"

class Dragon : public Enemy
{

public:

	Dragon(sf::Texture& idleTexture, sf::Texture& walkTexture, sf::Texture& attack2Texture, sf::Texture& dragonRiseTexture, sf::Texture& dragonFlightTexture, sf::Texture& dragonSpecialTexture, sf::Texture& dragonLandingTexture, sf::Texture& hurtTexture, sf::Texture& deadTexture, std::string& fontText, float x, float y, sf::Color& color) :
		dragonHurtSound(dragonHurtBuffer),
		dragonHurtSound2(dragonHurtBuffer2),
		dragonHurtSound3(dragonHurtBuffer3),
		dragonDeathSound(dragonDeathBuffer),
		dragonFlapSound(dragonFlapBuffer),
		dragonFlameSound(dragonFlameBuffer),
		dragonTakeoffSound(dragonTakeoffBuffer),
		dragonWhooshSound(dragonWhooshBuffer),
		dragonFireSpecialSound(dragonFireSpecialBuffer),
		dragonLandSound(dragonLandBuffer),
		dragonScreamSound(dragonScreamBuffer),
		dragonBar(1000,color,200),
		name(fontText, "Ploopwing", 840, 800.f, sf::Color::White, 40)

	{
		Idle = std::make_unique<Animation>(idleTexture, 7, 1, 0.2f, 0, false, true);
		Walk = std::make_unique<Animation>(walkTexture, 12, 1, 0.1f, 0, false, true);
		Attack2 = std::make_unique<Animation>(attack2Texture, 10, 1, 0.07f, 0, false, false);
		Takeoff = std::make_unique<Animation>(dragonRiseTexture, 7, 1, 0.1f, 0, false, false);
		Flight = std::make_unique<Animation>(dragonFlightTexture, 12, 1, 0.04f, 0, false, true);
		Special = std::make_unique<Animation>(dragonSpecialTexture, 13, 1, 0.05f, 0, false, true);
		Landing = std::make_unique<Animation>(dragonLandingTexture, 5, 1, 0.07f, 0, false, false);
		Hurt = std::make_unique<Animation>(hurtTexture, 4, 1, 0.1f, 0, false, false);
		Dead = std::make_unique<Animation>(deadTexture, 3, 1, 0.1f, 0, false, false);

		currentAnimation = Walk.get();

		dragonBox.setFillColor(sf::Color(255, 0, 0, 128));
		dragonBox.setSize({ 100.f,100.f });
		dragonBox.setScale({ 2.4,1.3 });
		dragonBox.setOrigin({ 50,-235 });

		placeHolder.setFillColor(sf::Color(255, 0, 0, 128));
		placeHolder.setSize({ 20.f,20.f });
		placeHolder.setScale({ 1,1 });
		placeHolder.setOrigin({ -160,-500 });

		rightLureBox.setSize({ 500.f, 50.f });
		rightLureBox.setScale({ 2,2 });
		rightLureBox.setOrigin({ -100.f, -140.f });
		rightLureBox.setFillColor(sf::Color(255, 0, 0, 128));

		leftLureBox.setSize({ 500.f, 50.f });
		leftLureBox.setScale({ 2,2 });
		leftLureBox.setOrigin({ 500.f, -140.f });
		leftLureBox.setFillColor(sf::Color(255, 0, 0, 128));

		attackBox.setSize({ 100.f, 35.f });
		attackBox.setScale({ 2,2 });
		attackBox.setOrigin({ 50.f, -170.f });
		attackBox.setFillColor(sf::Color(255, 0, 0, 128));

		attackInitializer.setSize({ 10.f, 35.f });
		attackInitializer.setScale({ 2,2 });
		attackInitializer.setOrigin({ 100.f, -170.f });
		attackInitializer.setFillColor(sf::Color(255, 0, 0, 128));

		helper.setSize({ 10.f, 35.f });
		helper.setScale({ 2,2 });
		helper.setOrigin({ 0.f, -150.f });
		helper.setFillColor(sf::Color(255, 0, 0, 128));



		lastDir = enemyDirection::Right;

		currentAnimation->getSprite().setPosition({ x,y });

		currentAnimation->getSprite().setScale({ 1.7,1.7 });

		sf::Sprite& IdleSprite = Idle->getSprite();
		IdleSprite.setOrigin({ IdleSprite.getLocalBounds().size.x * 0.5f, IdleSprite.getLocalBounds().position.x });

		sf::Sprite& walkSprite = Walk->getSprite();
		walkSprite.setOrigin({ walkSprite.getLocalBounds().size.x * 0.5f, walkSprite.getLocalBounds().position.x});

		sf::Sprite& attack2Sprite = Attack2->getSprite();
		attack2Sprite.setOrigin({ attack2Sprite.getLocalBounds().size.x - 170.f, attack2Sprite.getLocalBounds().position.x });

		sf::Sprite& takeoffSprite = Takeoff->getSprite();
		takeoffSprite.setOrigin({ takeoffSprite.getLocalBounds().size.x - 130.f, takeoffSprite.getLocalBounds().position.x });

		sf::Sprite& FlightSprite = Flight->getSprite();
		FlightSprite.setOrigin({ FlightSprite.getLocalBounds().size.x - 130.f, FlightSprite.getLocalBounds().position.x });

		sf::Sprite& SpecialSprite = Special->getSprite();
		SpecialSprite.setOrigin({ SpecialSprite.getLocalBounds().size.x - 130.f, SpecialSprite.getLocalBounds().position.x - 100 });

		sf::Sprite& LandingSprite = Landing->getSprite();
		LandingSprite.setOrigin({ LandingSprite.getLocalBounds().size.x - 130.f, LandingSprite.getLocalBounds().position.y });

		sf::Sprite& hurtSprite = Hurt->getSprite();
		hurtSprite.setOrigin({ hurtSprite.getLocalBounds().size.x - 150, hurtSprite.getLocalBounds().position.x });

		sf::Sprite& deadSprite = Dead->getSprite();
		deadSprite.setOrigin({ deadSprite.getLocalBounds().size.x * 0.5f, deadSprite.getLocalBounds().position.x });

		dragonHurtBuffer.loadFromFile("Sounds/DragonHurt.mp3");
		dragonHurtSound.setBuffer(dragonHurtBuffer);

		dragonHurtBuffer2.loadFromFile("Sounds/DragonHurt2.mp3");
		dragonHurtSound2.setBuffer(dragonHurtBuffer2);

		dragonHurtBuffer3.loadFromFile("Sounds/DragonHurt3.mp3");
		dragonHurtSound3.setBuffer(dragonHurtBuffer3);

		dragonFlapBuffer.loadFromFile("Sounds/DragonFlap.mp3");
		dragonFlapSound.setBuffer(dragonFlapBuffer);

		dragonFlapSound.setVolume(100.f);
		dragonFlapSound.setLooping(true);

		dragonFlameBuffer.loadFromFile("Sounds/DragonFlame.mp3");
		dragonFlameSound.setBuffer(dragonFlameBuffer);

		dragonTakeoffBuffer.loadFromFile("Sounds/DragonJump.mp3");
		dragonTakeoffSound.setBuffer(dragonTakeoffBuffer);

		dragonWhooshBuffer.loadFromFile("Sounds/whoosh.wav");
		dragonWhooshSound.setBuffer(dragonWhooshBuffer);

		dragonDeathBuffer.loadFromFile("Sounds/DragonDeath.mp3");
		dragonDeathSound.setBuffer(dragonDeathBuffer);

		dragonFlameSound.setVolume(70.f);

		dragonFireSpecialBuffer.loadFromFile("Sounds/DragonSpecialFire2.mp3");
		dragonFireSpecialSound.setBuffer(dragonFireSpecialBuffer);

		dragonFireSpecialSound.setLooping(true);

		dragonLandBuffer.loadFromFile("Sounds/DragonLanding.mp3");
		dragonLandSound.setBuffer(dragonLandBuffer);

		if (!bossMusic.openFromFile("Sounds/bossMusic.mp3"))
			std::cerr << "Failed to load boss music\n";

		bossMusic.setLooping(true);
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

	void attackLogic(std::vector<Tile>& tiles, float dt);
	void landingLogic(std::vector<Tile>& tiles);
	void gravityLogic(std::vector<Tile>& tiles, float dt);
	void flightAttackLogic(std::vector<Tile>& tiles, float dt);
	void setBackgroundShape(sf::RectangleShape* bg);

	sf::Music bossMusic;

	sf::SoundBuffer dragonHurtBuffer;
	sf::Sound dragonHurtSound;
	sf::SoundBuffer dragonHurtBuffer2;
	sf::Sound dragonHurtSound2;
	sf::SoundBuffer dragonHurtBuffer3;
	sf::Sound dragonHurtSound3;
	sf::SoundBuffer dragonDeathBuffer;
	sf::Sound dragonDeathSound;
	sf::SoundBuffer dragonFlapBuffer;
	sf::Sound dragonFlapSound;
	sf::SoundBuffer dragonTakeoffBuffer;
	sf::Sound dragonTakeoffSound;
	sf::SoundBuffer dragonFlameBuffer;
	sf::Sound dragonFlameSound;
	sf::SoundBuffer dragonWhooshBuffer;
	sf::Sound dragonWhooshSound;
	sf::SoundBuffer dragonFireSpecialBuffer;
	sf::Sound dragonFireSpecialSound;
	sf::SoundBuffer dragonLandBuffer;
	sf::Sound dragonLandSound;
	sf::SoundBuffer dragonScreamBuffer;
	sf::Sound dragonScreamSound;

private:

	std::unique_ptr<Animation> Idle;
	std::unique_ptr<Animation> Walk;
	std::unique_ptr<Animation> Attack1;
	std::unique_ptr<Animation> Special;
	std::unique_ptr<Animation> Takeoff;
	std::unique_ptr<Animation> Flight;
	std::unique_ptr<Animation> Attack2;
	std::unique_ptr<Animation> Landing;
	std::unique_ptr<Animation> Hurt;
	std::unique_ptr<Animation> Dead;

	Animation* currentAnimation;

	sf::Vector2f velocity = { 0.f,0.f };
	sf::Vector2f specialOffset = { 0.f,0.f };

	enemyDirection lastDir;

	float health = 200;
	float maxHealth = 200.f;

	sf::RectangleShape dragonBox;
	sf::RectangleShape attackBox;

	sf::RectangleShape leftLureBox;
	sf::RectangleShape rightLureBox;

	sf::RectangleShape placeHolder;

	sf::RectangleShape attackInitializer;
	sf::RectangleShape helper;

	bool dead = false;
	bool isHurt;
	bool rightLure = false;
	bool leftLure = false;
	bool knightDamaged = false;
	bool initalizer = false;

	sf::Clock dragonIdleClock;
	sf::Time idleInterval = sf::milliseconds(20000);
	bool idleWaiting = false;
	bool isColliding = false;
	bool isLanding = false;
	bool playOnce2 = true;
	bool playOnce3 = true;
	bool isMoving = false;

	const float gravity = 25.f;

	sf::Clock dragonFlightClock;
	sf::Time flightInterval = sf::milliseconds(40);

	sf::Clock dragonWaitClock;
	sf::Time waitInterval = sf::milliseconds(5000);

	sf::Clock dragonWaitClock2;
	sf::Time waitInterval2 = sf::milliseconds(3700);

	bool playOnce = true;

	int counter = 0;

	bool playOnce4 = false;
	bool playOnce5 = true;
	bool playOnce6 = true;
	bool playOnce7 = true;
	bool playOnce8 = true;
	bool playOnce9 = true;
	bool playOnce10 = true;
	bool playOnce11 = true;
	bool playOnce12 = true;
	bool playOnce13 = true;
	bool playOnce14 = true;
	bool playOnce15 = true;
	bool playOnce16 = true;
	bool playOnce17 = true;
	bool playOnce18 = true;
	bool teleported = false;

	bool fightStarted = false;

	bool teleported2 = false;
	int random = std::rand() % 4;

	Healthbar dragonBar;
	sf::RectangleShape* background = nullptr;

	Text name;

	void switchAnimation(Animation* newAnim) {

		if (currentAnimation != newAnim)
		{
			sf::Vector2f pos = currentAnimation->getSprite().getPosition();
			currentAnimation = newAnim;
			currentAnimation->getSprite().setPosition(pos);
		}

		currentAnimation->getSprite().setScale({ lastDir == enemyDirection::Right ? 1.7f : -1.7f, 1.7f });
	}
};
