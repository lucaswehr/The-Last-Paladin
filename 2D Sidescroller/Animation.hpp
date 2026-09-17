#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <cmath>
#include <cstdlib>  
#include <ctime>
#include <fstream>
#include <string>
#include <sstream>


enum class Direction {
    None,
    Left,
    Right
};


class Animation
{
public:

    Animation(sf::Texture& texture, int frameCount, int frameRows, float frameDuration, int rowToAnimate = 0, bool enableFlip = false, bool finished = false, bool isKnight = false)
        : texture(texture), frameCount(frameCount), frameDuration(frameDuration), currentFrame(0), elapsedTime(0.f), sprite(texture), frameRows(frameRows), rowToAnimate(rowToAnimate), enableFlip(enableFlip), shouldLoop(finished), finished(false), isKnight(isKnight) {

        updateTexture();
    }

    void update(float deltaTime) {
        
        if (finished && !shouldLoop) return;

        elapsedTime += deltaTime;

        if (elapsedTime >= frameDuration) {
            elapsedTime = 0.f;
            currentFrame = (currentFrame + 1); 
            

         if (currentFrame >= frameCount)
         {
            if (shouldLoop)
            {
                
                currentFrame = 0;
            }
            else
            {
               currentFrame = frameCount - 1;

                finished = true;  

            
            }

         }
            
            updateTexture(); 
        }     
    }


    void setPosition(float x, float y) {
        sprite.setPosition(sf::Vector2f(x, y));
    }

    sf::Vector2f getPosition()
    {
        return this->sprite.getPosition();
    }


    void setScale(float scaleX, float scaleY) {
        sprite.setScale(sf::Vector2f(scaleX, scaleY));
    }

    void draw(sf::RenderWindow& window) {

      
        window.draw(sprite);
    }

    sf::Sprite& getSprite() {
        return sprite;
    }

    void setDirection(Direction dir) {
        direction = dir;
        updateTexture(); 
    }

    void setInitalDirectionToLeft()
    {
        direction = Direction::Left;
        updateTexture();
    }

    void setScale(const sf::Vector2f& scale) {
        this->sprite.setScale(scale);
    }

    void setBaseScale(const sf::Vector2f& scale) {
        baseScale = scale;
        updateTexture(); 
    }


    void reset()
    {
        elapsedTime = 0.f;
        currentFrame = 0;
        finished = false;
        updateTexture();
    }

    bool isFinished() const {
        return finished;
    }

    int getCurrentFrame()
    {
        return this->currentFrame;
    }

    void setFrameDuration(float newTime)
	{
		frameDuration = newTime;
	}

    void setFrame(int frame)
    {
        currentFrame = frame;
        updateTexture();
    }

    int nextFrame()
    {
        return currentFrame++;
    }

    int getFrameCount()
    {
        return frameCount;
    }

    void setOffset(const sf::Vector2f& off) { offset = off; }
    sf::Vector2f getOffset() const { return offset; }

private:

    sf::Sprite sprite;
    sf::Texture& texture;
    int frameCount;
    int frameRows;
    int rowToAnimate;
    float frameDuration;
    int currentFrame;
    float elapsedTime;
    Direction direction = Direction::Right;
    bool enableFlip;
    sf::Vector2f baseScale = { 2.f, 2.f };

    bool isKnight;
    bool finished;
    bool shouldLoop;

    sf::Vector2f offset = { 0.f, 0.f };

    void updateTexture()
    {
       
        auto size = texture.getSize();
      
        int frameWidth = texture.getSize().x / frameCount;  
        int frameHeight = texture.getSize().y / frameRows; 

       
       int left = frameWidth * currentFrame;   
        int top = frameHeight * rowToAnimate;  

       
       sprite.setTextureRect(sf::IntRect({ left, top }, { frameWidth, frameHeight }));
        
        if (enableFlip)
        {         
            sprite.setScale(sf::Vector2f(2.f * (direction == Direction::Left ? -1.f : 1.f), 2.f));

            if (isKnight)
            {
                sprite.setOrigin(sf::Vector2f(sprite.getLocalBounds().size.x / 2.f - 31, sprite.getLocalBounds().size.y / 2.f - 21));
            }
            else {
                sprite.setOrigin(sf::Vector2f(sprite.getLocalBounds().size.x / 2.f, sprite.getLocalBounds().size.y / 2.f - 21));
            }
          

        }
       
    }
};
