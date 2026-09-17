#include "AssetManager.hpp"

bool AssetManager::load()
{
    bool success = true;

    // (&=): Goes through all lines of code even if one fails, success rememebers what failed
    success &= loadTexture(knightIdleTex, "Knight_1/Idle.png");
    success &= loadTexture(knightWalkTex, "Knight_1/Walk.png");
    success &= loadTexture(knightJumpTex, "Knight_1/Jump.png");
    success &= loadTexture(knightAttack1Tex, "Knight_1/Attack 1.png");
    success &= loadTexture(knightAttack2Tex, "Knight_1/Attack 2.png");
    success &= loadTexture(knightAttack3Tex, "Knight_1/Attack 3.png");
    success &= loadTexture(knightRunTex, "Knight_1/Run.png");
    success &= loadTexture(knightShieldTex, "Knight_1/Shield_Strike.png");
    success &= loadTexture(knightRunningAttackTex, "Knight_1/Run+Attack.png");
    success &= loadTexture(knightHurtTex, "Knight_1/Hurt.png");
    success &= loadTexture(knightDeadTex, "Knight_1/Dead.png");
    success &= loadTexture(knightHangingTex, "Knight_1/Hang.png");
    success &= loadTexture(knightClimbingTex, "Knight_1/Pull_up.png");
    success &= loadTexture(knightRollingTex, "Knight_1/Roll.png");
    success &= loadTexture(knightElixirTex, "Knight_1/Elixir.png");
    success &= loadTexture(potionTexture, "Knight_1/potionIcon.png");
    success &= loadTexture(knightJumpAttackTex, "Knight_1/Jump+Attack.png");

    success &= loadTexture(samuraiIdleTex, "Samurai_Archer/Idle.png");
    success &= loadTexture(samuraiWalkTex, "Samurai_Archer/Walk.png");
    success &= loadTexture(samuraiRunTex, "Samurai_Archer/Run.png");
    success &= loadTexture(samuraiJumpTex, "Samurai_Archer/Jump.png");
    success &= loadTexture(samuraiAttack1Tex, "Samurai_Archer/Attack_1.png");
    success &= loadTexture(samuraiAttack2Tex, "Samurai_Archer/Attack_2.png");
    success &= loadTexture(samuraiAttack3Tex, "Samurai_Archer/Attack_3.png");
    success &= loadTexture(samuraiSpecialTex, "Samurai_Archer/Shot.png");
    success &= loadTexture(samuraiArrowTex, "Samurai_Archer/Arrow.png");
    success &= loadTexture(samuraiPullUpTex, "Samurai_Archer/Pull_up.png");
    success &= loadTexture(samuraiDeadTex, "Samurai_Archer/Dead.png");
    success &= loadTexture(samuraiHurtTex, "Samurai_Archer/Hurt.png");
    success &= loadTexture(samuraiRollTex, "Samurai_Archer/Roll.png");
    success &= loadTexture(samuraiElixirTex, "Samurai_Archer/Elixir.png");

    success &= loadTexture(loadingBackgroundTex, "Backgrounds/loadingBackground.jpg");
    success &= loadTexture(titleBackgroundTex, "Backgrounds/titleBackground.png");
    success &= loadTexture(createServerBackgroundTex, "Backgrounds/createServerBackground.jpg");
    success &= loadTexture(serverListBackgroundTex, "Backgrounds/serverListBackground.jpg");
    success &= loadTexture(arenaPictureTex, "Backgrounds/arena.png");
    success &= loadTexture(singlePlayerBackgroundTex, "Backgrounds/origbig.png");
    success &= loadTexture(howToPlayBackrgoundTex, "Backgrounds/forestBackground.png");

    return success;
}

knightTextures AssetManager::getKnightTextures()
{
    return { 
         knightIdleTex,
         knightWalkTex,
         knightJumpTex,
         knightAttack1Tex,
         knightAttack2Tex,
         knightAttack3Tex,
         knightRunTex,
         knightShieldTex,
         knightRunningAttackTex,
         knightHurtTex,
         knightDeadTex,
         knightHangingTex,
         knightClimbingTex,
         knightRollingTex,
         knightElixirTex,
         potionTexture,
         knightJumpAttackTex
    };
}

samuraiTextures AssetManager::getSamuraiTextures()
{
    return { 
        samuraiIdleTex,
        samuraiWalkTex,
        samuraiRunTex,
        samuraiJumpTex,
        samuraiAttack1Tex,
        samuraiAttack2Tex,
        samuraiAttack3Tex,
        samuraiSpecialTex,
        samuraiArrowTex,
        samuraiPullUpTex,
        samuraiDeadTex,
        samuraiHurtTex,
        samuraiRollTex,
        samuraiElixirTex
    };
}

backgroundTextures AssetManager::getBackgroundTextures()
{
    return {
        loadingBackgroundTex,
        createServerBackgroundTex,
        singlePlayerBackgroundTex,
        serverListBackgroundTex,
        titleBackgroundTex,
        arenaPictureTex,
        howToPlayBackrgoundTex
    };
}

bool AssetManager::loadTexture(sf::Texture& texture, const string& path)
{
    if (!texture.loadFromFile(path))
    {
        cout << "Error loading " << path << endl;
        return false;
    }

    return true;
}
