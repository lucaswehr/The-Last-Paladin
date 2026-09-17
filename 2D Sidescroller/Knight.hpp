#pragma once
#include "Animation.hpp"
#include <SFML/Graphics.hpp>
#include "Tile.hpp"
#include "wolf.hpp"
#include "Enemy.hpp"
#include "Arrow.hpp"
#include "text.hpp"
#include "PlayerState.hpp"
#include "attackData.h"
#include "Healthbar.hpp"
#include "PlayerTextures.h"
#include "InputState.hpp"
#include "Player.hpp"

class Knight : public Player
{
public:

	Knight(float xPos, float yPos, const knightTextures& textures,std::string& fontText, sf::Color& color, sf::Font& standardFont) :
		Player(100, 50, 5.f, 8.f, -500.f, 1200.f, 6.f, color, standardFont),
		Idle(textures.idleTex, 4, 1, 0.3f, 0, true, true, true), //Texture, number of frames, rows, speed, 0 if its just one row of animatoin, if it should invert, if the animation should loop infintly, isKnight
		walk(textures.walkTex, 8, 1, 0.1f, 0, true, true, true),
		jump(textures.jumpTex, 6, 1, 0.15f, 0, true, true,true),
		attack1(textures.attack1Tex, 5, 1, 0.06f, 0, true, false, true),
		attack2(textures.attack2Tex, 4, 1, 0.07f, 0, true, false, true),
		attack3(textures.attack3Tex, 4, 1, 0.07f, 0, true, false, true),
		run(textures.runTex, 7, 1, 0.1f, 0, true, true, true),
		guard(textures.shieldTex, 4, 1, 0.15f, 0, true, false, true),
		runningAttack(textures.runAttackTex, 6, 1, 0.1f, 0, true, false, true),
		hurt(textures.hurtTex, 2, 1, 0.25f, 0, true, true, true),
		dead(textures.deadTex, 6, 1, 0.1f, 0, false, false, true),
		hanging(textures.hangingTex,6,1, 0.1f,0,true, true, true),
		climbing(textures.climbTex,6, 1, 0.1f , 0, true, false, true),
		roll(textures.rollTex, 6, 1, 0.1f, 0, true, false, true),
		elixir(textures.elixirTex, 4, 1, 0.3f, 0, true, false, true),
		jumpAttack(textures.jumpAttackTex, 5, 1, 0.15f,0,true, false, true),						
		dragonScreamSound(dragonScreamBuffer),		
		nameText(nameFont)
	{

		stunHoldFrame = 1;

		potionNumber = 1;
		currentAnimation = &Idle;
		
		if (!nameFont.openFromFile(fontText))
		{
			std::cout << "Error in text constructor in Knight class" << std::endl;
		}

		nameText.setFont(nameFont);
		nameText.setCharacterSize(16);
		nameText.setFillColor(sf::Color::White);
		nameText.setOutlineColor(sf::Color::Black);
		nameText.setOutlineThickness(2);

		initializeSounds();

		playerBox.setSize({ 32.f, 48.f });
		playerBox.setScale({ 2,2.5 });
		playerBox.setOrigin({ 17.f, -20.f });
		playerBox.setFillColor(sf::Color(255, 0, 0, 128));

		attackBox.setSize({ 32.f, 48.f });
		attackBox.setScale({ 4,2.5 });
	    attackBox.setFillColor(sf::Color(255, 0, 0, 128));
		attackBox.setOrigin({ 17.f, -20.f });

		specialAttackBox.setSize({ 35.f, 40.f });
		specialAttackBox.setScale({ 2,2.5 });
		specialAttackBox.setOrigin({ -10.f, -20.f });
		specialAttackBox.setFillColor(sf::Color(255, 0, 0, 128));

		rightLureBox.setSize({ 300.f, 10.f });
		rightLureBox.setScale({ 2,2.5 });
		rightLureBox.setOrigin({ -20.f, -30.f });
		rightLureBox.setFillColor(sf::Color(255, 0, 0, 128));

		leftLureBox.setSize({ -300.f, 10.f });
		leftLureBox.setScale({ 2,2.5 });
		leftLureBox.setOrigin({ 20.f, -30.f });
		leftLureBox.setFillColor(sf::Color(255, 0, 0, 128));

		climbingBox.setSize({ 50.f, 50.f });
		climbingBox.setScale({ 1,1.2 });
		climbingBox.setOrigin({ 20.f, 10.f });
		climbingBox.setFillColor(sf::Color(0, 0, 255, 128));

		bossMusicBox.setSize({ 500.f, 500.f });
		bossMusicBox.setScale({ 1,1.2 });
		bossMusicBox.setOrigin({ 20.f, 10.f });
		bossMusicBox.setFillColor(sf::Color(0, 0, 255, 128));

		bossMusicBox2.setSize({ 500.f, 500.f });
		bossMusicBox2.setScale({ 1,1.2 });
		bossMusicBox2.setOrigin({ 20.f, 10.f });
		bossMusicBox2.setFillColor(sf::Color(0, 0, 255, 128));
		
		//potionSprite.setScale({ 4,4 });

		bossMusicBox.setPosition({ 7000,-300 });

		bossMusicBox2.setPosition({ 10900,200 });
		
		if (xPos > 1770.f / 2) lastDir = Direction::Left;
		else lastDir = Direction::Right;
			
		//400
		//1825
		float startX = xPos;  // X start position on map
		float startY = yPos; // Y start position on ground level

		playerBox.setPosition({ startX, startY });
		currentAnimation->getSprite().setPosition({ startX, startY });
	
		playerIdentifier.setFillColor(color);

		playerIdentifier.setRadius(7);

	}

	void loadTileMap(sf::Texture& tileset, const std::vector<std::vector<int>>& mapData, sf::Vector2u tileSize, std::vector<Tile>& tiles);

	void loadDecorationMap(sf::Texture& decorationTexture, sf::Texture& bush, sf::Texture& roseBush, const std::vector<std::vector<int>>& decorationData, sf::Vector2u spriteSize, sf::Vector2u tileSize);

	

	void updateSinglePlayer(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<Arrow>& arrows, sf::Font& standardFont) override
	{
		cout << playerBox.getPosition().y << endl;
		potionNumText.setString(std::to_string(potionNumber));

		rightLureBox.setPosition({ currentAnimation->getSprite().getPosition() });

		leftLureBox.setPosition({ currentAnimation->getSprite().getPosition() });

		climbingBox.setPosition({ currentAnimation->getSprite().getPosition() });

		currentAnimation->setDirection(lastDir);

		updateDamageText(dt);

		if (handleDeathLogic(dt, tiles)) return;

		updatePotionLogic(standardFont);
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

			updateSpecialAttack(dt, tiles);

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

		if (isHurt)
		{
			cout << "HURTING" << endl;
			isNormalAttacking = false;
			isSprintAttacking = false;
			isJumpAttacking = false;
		}


		enemyKnightCollision(enemies, arrows, dt);
		
		updateHitBoxWindow(dt);
		climbingLogic(tiles);
		updateGravity(dt, tiles);
		lureLogic(enemies);
		pullUpLogic();
		musicLogic();

		switchAnimationByState(state);

		lastInput = input;

		currentAnimation->getSprite().setPosition(playerBox.getPosition() + animationOffset());

		currentAnimation->update(dt);
	}

	void draw(sf::RenderWindow& window) override
	{
		window.draw(currentAnimation->getSprite());

		for (const auto& damageNumber : damageNumbers)
		{
			window.draw(damageNumber.text);
		}
		//window.draw(playerIdentifier);

		//window.draw(rightLureBox);
		//window.draw(leftLureBox);
		//window.draw(specialAttackBox);
		//window.draw(climbingBox);
		/*window.draw(bossMusicBox);
		window.draw(bossMusicBox2);
		window.draw(healthPotion);*/
		//window.draw(playerBox);
		//window.draw(attackBox);
	}
	
	void setPosition(float x, float y)
	{
		currentAnimation->setPosition(x, y);
	}

	// -------------------------------------MULTIPLAYER FUNCTIONS---------------------------------------------//

	void updateMultiplayer(float dt, std::vector<Tile>& tiles, sf::Font& standardFont) override;

	void switchAnimationByState(PlayerState& state) override;

	void updateRemotePlayers(float dt, vector<Tile>& tiles) override;

	bool isAttackingBool() override;
	bool isSpecialAttackBool();

	sf::RectangleShape getAttackBox();

	void determineCharacterHitbox() override;

	void applyDamage(int damage, Direction attackerDir);

	void updateHitBoxWindow(float dt) override;

	void startAttack(const attackData& data, Direction dir) override;

	bool isSprintAttackingBool();

	void MultiplayerDeath();

	void setKnightName(std::string& newName);

	bool isGrounded();

	void jumpAttackLogic(float dt);

	bool isJumpAttackingBool();

	void updateBoundryBoxes() override;

	bool handleDeathLogic(float dt, std::vector<Tile>& tiles);

	void handleActions(InputState& input, float dt) override;
	void attackLogicMultiplayer() override;
	
	bool tryStartJumpAttack(InputState& input, float dt);
	void updateJumpAttack(float dt);
	void updateJumpAttackCooldown(float dt);

	bool tryStartSprintAttack(InputState& input);
	void sprintAttackLogic();
	void updateSprintAttackCooldown(float dt);
	void updateSprintAttack();

	bool tryStartRoll(InputState& input);
	void rollLogic();
	void updateRollLogic();

	bool tryStartJump(InputState& input) override;
	void jumpLogic() override;

	bool tryStartNormalAttacks(InputState& input, float dt) override;
	void updateAttack(float dt) override;
	void updateAttackCooldown(float dt) override;

	bool tryStartSpecial(InputState& input) override;
	void specialAttackLogic() override;
	void updateSpecialAttack(float dt, std::vector<Tile>& tiles) override;

	bool cancelElixir();

	void handleMovement(InputState& input) override;
	
	sf::Vector2f animationOffset() override;

	void applyStun(float duration) override;

	void setHealth(int newHealth);

	void setHealBoolean(bool x);

	bool isParryWindow() override;
	bool isDeflectionWindow() override;

	AttackHitbox* getAttackBoxTable() override;

	const attackData* getAttackTable() const override;

	void cancelAttack() override;

	void resetCharacter(bool isSinglePlayer) override;

	int determineAttackID() override;

	float getGuardTimer() override;

	// -------------------------------------------------------------------------------------------------------//

	void drawCollisionBox(sf::RenderWindow& window) {

	//	window.draw(knightBox);
	}

	void initializeSounds();

	void setAttackBoolean();

	void enemyKnightCollision(std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<Arrow>& arrows, float dt);

	void attackLogic(std::vector<std::unique_ptr<Enemy>>& enemies, float dt);

	void lureLogic (std::vector<std::unique_ptr<Enemy>>& enemies);

	bool Death();

	sf::RectangleShape getKnightBox();

	void climbingLogic(std::vector<Tile>& tiles) override;

	void pullUpLogic();

	void musicLogic()
	{

		if (playerBox.getGlobalBounds().findIntersection(bossMusicBox.getGlobalBounds())) {
			if (!touch) {
				touch = true;
				clock2.restart(); // start fade timer ONCE
			}
		}

		if (touch) {
			float elapsed = clock2.getElapsedTime().asSeconds();

			if (elapsed < fadeDuration) {
				float volume = startVolume * (1.f - (elapsed / fadeDuration));
				music.setVolume(volume);
			}
			else {
				music.stop();             // stop old music
				
				
			}
		}

		if (playerBox.getGlobalBounds().findIntersection(bossMusicBox2.getGlobalBounds()))
		{
			if (playOnce5)
			{
				dragonScreamSound.play();
				playOnce5 = false;
			}
		}
		

	}

	void resetKnight(int x);

	void placeEnemies(std::vector<std::unique_ptr<Enemy>>& enemies);

	/*sf::Music music;
	sf::Music deathMusic;*/

	//sf::Sprite healthPotion;

	float elasped;

	bool hasBeenParried = false;

	const float guardParryStart = 0.2f;
	const float guardParryEnd = 0.45f;
	bool parryWindowActive = false;

	const float guardDeflectionStart = 0.0f;
	const float guardDeflectionEnd = 0.8f;
	bool deflectionWindowActive = false;

	float guardTimer = 0.f;

private:

	Animation walk;
	Animation Idle;
	Animation jump;
	Animation attack1;
	Animation attack2;
	Animation attack3;
	Animation run;
	Animation guard;
	Animation runningAttack;
	Animation hurt;
	Animation dead;
	Animation hanging;
	Animation climbing;
	Animation roll;
	Animation elixir;
	Animation jumpAttack;

	sf::CircleShape playerIdentifier;
	
	std::string name;
	sf::Text nameText;
	sf::Font nameFont;

	sf::Vector2f deadlockedPosition;

	bool postAttackCooldown = false;
	float postAttackTimer = 0.f;

	float postSprintAttackTimer = 0.f;
	bool sprintAttackCooldown = false;

	const float attackPauseDuration = 0.2f;
	bool isSprintAttacking = false;

	bool postGuardCooldown = false;
	float guardCooldownTimer = 1.5f;

	bool isJumpAttacking = false;
	bool jumpAttackCooldown = false;
	float jumpAttackCooldownTimer = 0.f;

	bool reversePlayed = false;

	float jumpAttackTimer = 0.f;
	const float jumpAttackDuration = 0.7f; 

	bool wasMousePressedLastFrame = false;

	float groundY = 0.f;

	bool wasWPressedLastFrame = false;
	
	bool knightWolfCollision = false;

	bool wolfRight = true;
	bool wolfLeft = false;

	bool playSoundOnce = false;

	sf::RectangleShape rightLureBox;
	sf::RectangleShape leftLureBox;
	sf::RectangleShape bossMusicBox;
	sf::RectangleShape bossMusicBox2;

	sf::SoundBuffer dragonScreamBuffer;
	sf::Sound dragonScreamSound;

	sf::Clock rollDelayClock;
	float minRollDelay = 0.8f;

	bool shieldSoundPlayed = false;
	sf::Clock shieldSoundTimer;
	float shieldSoundCooldown = 0.6f;

	int jumpCount = 0;
	bool isHanging = false;
	int wPressed = 0;
	bool playOnce2 = true;
	bool playOnce3 = true;
	bool playOnce4 = true;
	bool playOnce5 = true;
	bool playOnce6 = true;

	bool doOnce = false;

	bool touch = false;

	const float fadeDuration = 10.f; // seconds
	const float startVolume = 100.f;
	sf::Clock clock2;	
};
