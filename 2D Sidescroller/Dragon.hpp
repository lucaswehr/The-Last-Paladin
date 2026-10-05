#pragma once
#include "Animation.hpp"
#include "Enemy.hpp"
#include "Healthbar.hpp"
#include "text.hpp"

class Dragon : public Enemy
{

public:

	Dragon(sf::Texture& idleTexture, sf::Texture& walkTexture, sf::Texture& attack2Texture, sf::Texture& dragonRiseTexture, sf::Texture& dragonFlightTexture, sf::Texture& dragonSpecialTexture, sf::Texture& dragonLandingTexture, sf::Texture& hurtTexture, sf::Texture& deadTexture, std::string& fontText, const sf::Font standardFont, float x, float y, sf::Color& color) :
		Enemy(-1), // Not used
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
		dragonBar(1000, color, 200, 30),
		name(fontText, "Ploopwing", 840, 800.f, sf::Color::White, 40)

	{
		damageFontText = standardFont;

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
		walkSprite.setOrigin({ walkSprite.getLocalBounds().size.x * 0.5f, walkSprite.getLocalBounds().position.x });

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
	void gravityLogic(std::vector<Tile>& tiles, float dt);
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

	// ------------------------------------------------------------------
	//  Air attack state machine
	// ------------------------------------------------------------------
	enum class AirState
	{
		None,
		Takeoff,
		SpecialFlight,   // hovering while the Flight animation plays, before the fire sweep
		Special,         // fire sweep across the platform
		SwoopClimb,
		SwoopOut,
		SwoopBack,
		SwoopRise,
		SwoopDive,
		Landing
	};

	AirState airState = AirState::None;

	float phaseTime = 0.f;   // seconds spent in the current air state
	float airTimer = 0.f;   // counts up while the knight is lured (ground only)
	float ledgeCooldown = 0.f;   // gap between ledge-forced take-offs
	float hurtCooldown = 0.f;   // min time between damage ticks

	bool hurtStun = false;   // Hurt animation currently playing
	bool swoopAttack = false;   // current air attack is a swoop
	bool whooshPlayed = false;
	bool grounded = false;   // set by gravityLogic

	// Tunables
	static constexpr float airAttackInterval = 8.f;     // seconds of lure before a flight
	static constexpr float ledgeCooldownTime = 5.f;     // gap between ledge take-offs
	static constexpr float ledgeWarnDistance = 150.f;   // how far past the cliff probe counts as "near a ledge"
	static constexpr float hurtCooldownTime = 0.5f;    // min time between damage ticks

	void takeHit();
	void updateAttackBox();
	void turnAround();
	bool isNearLedgeAhead(const std::vector<Tile>& tiles) const;
	void startAirAttack(bool fromLedge, const std::vector<Tile>& tiles);
	void enterAirState(AirState next);
	void airAttackLogic(std::vector<Tile>& tiles, float dt);
	void finishAirAttack();

	// ------------------------------------------------------------------
	//  Animations
	// ------------------------------------------------------------------
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

	Animation* currentAnimation = nullptr;

	// ------------------------------------------------------------------
	//  Movement / combat state
	// ------------------------------------------------------------------
	sf::Vector2f velocity = { 0.f,0.f };
	sf::Vector2f specialOffset = { 0.f,0.f };

	enemyDirection lastDir = enemyDirection::Right;

	float health = 250;
	float maxHealth = 250.f;

	sf::RectangleShape dragonBox;
	sf::RectangleShape attackBox;

	sf::RectangleShape leftLureBox;
	sf::RectangleShape rightLureBox;

	sf::RectangleShape placeHolder;     // cliff probe

	sf::RectangleShape attackInitializer;
	sf::RectangleShape helper;          // remembers last grounded position

	bool dead = false;
	bool isHurt = false;                // was uninitialized before
	bool rightLure = false;
	bool leftLure = false;
	bool knightDamaged = false;
	bool initalizer = false;

	sf::Clock dragonIdleClock;
	sf::Time idleInterval = sf::milliseconds(20000);
	bool isColliding = false;
	bool isLanding = false;
	bool isMoving = false;

	const float gravity = 25.f;

	sf::Time waitInterval = sf::milliseconds(5000);    // swoop: climb duration
	sf::Time waitInterval2 = sf::milliseconds(3700);   // swoop: first sweep duration

	bool playOnce4 = false;    // death
	bool playOnce9 = true;     // ground flame sound
	bool playOnce16 = true;    // boss music

	bool fightStarted = false;

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