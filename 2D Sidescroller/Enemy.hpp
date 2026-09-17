#pragma once
#include <memory>
#include "Animation.hpp"
#include "Tile.hpp"
//#include "wolf.hpp"

enum class enemyDirection {
	Left,
	Right
};

class Enemy
{
public:

	virtual void update(float dt, std::vector<Tile>& tiles, std::vector<std::unique_ptr<Enemy>>& enemies) = 0;
	virtual void draw(sf::RenderWindow& window) = 0;
	virtual sf::FloatRect getBounds() = 0;
	virtual sf::FloatRect getAttackBoxBounds() = 0;
	virtual bool isDead() = 0;
	virtual int getHealth() = 0;
	virtual void checkCollision(std::vector<Tile>& tiles) = 0;
	virtual void checkForCliff(std::vector<Tile>& tiles) = 0;
	virtual void isHurtTrue() = 0;
	virtual void isHurtFalse() = 0;
	virtual void hitsShieldTrue() = 0;
    virtual Animation* getCurrentEnemyAnimation() = 0;
	virtual void knightDamagedTrue() = 0;
	virtual void knightDamagedFalse() = 0;
	virtual enemyDirection getDirection() = 0;
	virtual void setDirection(enemyDirection newDir) = 0;
	virtual bool setEnemyLeftLure(bool value) = 0;
	virtual bool setEnemyRightLure(bool value) = 0;
	virtual sf::FloatRect getEnemyRightLure() = 0;
	virtual sf::FloatRect getEnemyLeftLure() = 0;
	virtual bool setInitializerBox(bool value) = 0;
	virtual sf::FloatRect getIninitializerBox() = 0;

};
