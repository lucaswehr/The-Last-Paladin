#pragma once
#include "Animation.hpp"
#include "PlayerTextures.h"
using namespace std;

class AssetManager
{
public:

    bool load();

    knightTextures getKnightTextures();
    samuraiTextures getSamuraiTextures();
    backgroundTextures getBackgroundTextures();

private:

    bool loadTexture(sf::Texture& texture, const string& path);

    // Knight Texures
    sf::Texture knightIdleTex;
    sf::Texture knightWalkTex;
    sf::Texture knightJumpTex;
    sf::Texture knightAttack1Tex; 
    sf::Texture knightAttack2Tex; 
    sf::Texture knightAttack3Tex;
    sf::Texture knightRunTex; 
    sf::Texture knightShieldTex;  
    sf::Texture knightRunningAttackTex; 
    sf::Texture knightHurtTex;
    sf::Texture knightDeadTex;  
    sf::Texture knightHangingTex;  
    sf::Texture knightClimbingTex;   
    sf::Texture knightRollingTex;  
    sf::Texture knightElixirTex;  
    sf::Texture knightJumpAttackTex;
    sf::Texture potionTexture;
   
  
    // Samurai Textures
    sf::Texture samuraiIdleTex;
    sf::Texture samuraiWalkTex;
    sf::Texture samuraiRunTex;
    sf::Texture samuraiJumpTex; 
    sf::Texture samuraiAttack1Tex;   
    sf::Texture samuraiAttack2Tex;  
    sf::Texture samuraiAttack3Tex;  
    sf::Texture samuraiSpecialTex;
    sf::Texture samuraiArrowTex;   
    sf::Texture samuraiPullUpTex;
    sf::Texture samuraiDeadTex;
    sf::Texture samuraiHurtTex;
    sf::Texture samuraiRollTex;
    sf::Texture samuraiElixirTex;

    // Backgrounds
    sf::Texture loadingBackgroundTex;
    sf::Texture createServerBackgroundTex;
    sf::Texture singlePlayerBackgroundTex;
    sf::Texture serverListBackgroundTex;
    sf::Texture titleBackgroundTex;
    sf::Texture arenaPictureTex;
    sf::Texture howToPlayBackrgoundTex;

    
};