#pragma once
#include "Animation.hpp"
#include "PlayerTextures.h"
#include "Player.hpp"
#include "Tile.hpp"
#include "Arrow.hpp"

class Client;

class Samurai : public Player
{

public:
	Samurai(float xPos, float yPos, const samuraiTextures& textures, sf::Color& color, sf::Font& standardFont) :
		Player(80,20, 7.f, 10.f, -700.f, 1500.f, 8.f, color, standardFont),
		Idle(textures.idleTex, 9, 1, 0.2f, 0, true, true),
		Walk(textures.walkTex, 8, 1, 0.1f, 0, true, true),
		Run(textures.runTex, 8, 1, 0.1f, 0, true, true),
		Jump(textures.jumpTex, 9, 1, 0.07f, 0, true, true),
		Attack1(textures.attack1Tex, 5, 1, 0.08f, 0, true, false),
		Attack2(textures.attack2Tex, 5, 1, 0.1f, 0, true, false),
		Attack3(textures.attack3Tex, 6, 1, 0.08f, 0, true, false),
		Special(textures.specialTex, 14, 1, 0.08f, 0, true, false),
		Elixir(textures.elixirTex, 4, 1, 0.2f, 0, true, false),
		Climb(textures.climbTex, 5, 1, 0.1f, 0, true, false),
		Dead(textures.deadTex, 5, 1, 0.1f, 0, false, false),
		Hurt(textures.hurtTex, 3, 1, 0.25f, 0, true, true),
		Roll(textures.rollTex, 6, 1, 0.1f, 0, true, false),
		arrowTex(textures.arrowTex)		
	{

		initializeSounds();

		stunHoldFrame = 3;

		currentAnimation = &Idle;
		potionNumber = 2;
		
		if (xPos > 1770.f / 2) lastDir = Direction::Left;
		else lastDir = Direction::Right;

		playerBox.setSize({ 32.f, 48.f });
		playerBox.setScale({ 2,2.5 });
		playerBox.setOrigin({ 17.f, -20.f });
		playerBox.setFillColor(sf::Color(255, 0, 0, 128));

		climbingBox.setSize({ 50.f, 50.f });
		climbingBox.setScale({ 1,1.2 });
		climbingBox.setOrigin({ 20.f, 10.f });
		climbingBox.setFillColor(sf::Color(0, 0, 255, 128));

		attackBox.setSize({ 32.f, 48.f });
		attackBox.setScale({ 4,2.5 });
		attackBox.setFillColor(sf::Color(255, 0, 0, 128));
		attackBox.setOrigin({ 17.f, -20.f });

		playerBox.setPosition({ xPos,yPos });
		currentAnimation->getSprite().setPosition({ xPos,yPos });
		
	};

	void draw(sf::RenderWindow& window) override;
	
	void updateMultiplayer(float dt, vector<Tile>& tiles, sf::Font& standardFont) override;

	void handleActions(InputState& input, float dt) override;

	void handleMovement(InputState& input) override;

	void switchAnimationByState(PlayerState& state) override;

	bool tryStartJump(InputState& input) override;
	void jumpLogic() override;

	bool tryStartNormalAttacks(InputState& input, float dt) override;
	void attackLogicMultiplayer() override;
	void updateAttack(float dt) override;
	void updateAttackCooldown(float dt) override;

	bool tryStartSpecial(InputState& input) override;
	void specialAttackLogic() override;
	void updateSpecialAttack(float dt, std::vector<Tile>& tiles) override;

	void spawnArrow(sf::Vector2f pos, Direction dir, int arrowID, float arrowGravity);
	void updateArrows(float dt, vector<Tile>& tiles);

	void climbingLogic(std::vector<Tile>& tiles) override;

	void updateBoundryBoxes() override;

	void determineCharacterHitbox() override;

	void updateHitBoxWindow(float dt) override;

	void startAttack(const attackData& attack, Direction dir) override;

	AttackHitbox* getAttackBoxTable() override;

	const attackData* getAttackTable() const override;

	void cancelAttack() override;

	bool isAttackingBool() override;

	void updateRemotePlayers(float dt, vector<Tile>& tiles) override;

	void resetCharacter(bool isSinglePlayer) override;

	void applyStun(float duration) override;

	bool isParryWindow() override;

	int determineAttackID() override;

	bool handleDeathLogic(float dt, std::vector<Tile>& tiles) override;

	void processProjectileCollisions(Player& victim, int attackerID, int victimID, Client& client, sf::Font& standardFont) override;

	void processProjectileDeflection(Player& victim, Client& client, int attackerID, int victimID, sf::Font& standardFont) override;

	sf::Vector2f animationOffset() override;

	bool didArrowSpawnThisFrame();

	void clearArrowSpawnFlag();

	vector<Arrow>& getArrows();

	bool cancelElixir();

	int getlastArrowID();

	float getArrowGravity();

private:

	Animation Idle;
	Animation Walk;
	Animation Run;
	Animation Jump;
	Animation Attack1;
	Animation Attack2;
	Animation Attack3;
	Animation Special;
	Animation Climb;
	Animation Dead;
	Animation Hurt;
	Animation Roll;
	Animation Elixir;

	sf::Texture& arrowTex;
	
	bool arrowFired = false;
	vector<Arrow> arrows;

	int nextArrowID = 0;
	int lastArrowID = -1;

	bool arrowSpawnedThisFrame = false;

	int lastAttackFrame = -1;

	float comboTransition = 0.2f;
	float comboTransitionTimer = 0.f;
	bool waitingForNextCombo = false;

	void initializeSounds();
};
