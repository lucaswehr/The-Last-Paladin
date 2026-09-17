#pragma once
#include "Animation.hpp"

struct knightTextures {
    sf::Texture& idleTex;
    sf::Texture& walkTex;
    sf::Texture& jumpTex;
    sf::Texture& attack1Tex;
    sf::Texture& attack2Tex;
    sf::Texture& attack3Tex;
    sf::Texture& runTex;
    sf::Texture& shieldTex;
    sf::Texture& runAttackTex;
    sf::Texture& hurtTex;
    sf::Texture& deadTex;
    sf::Texture& hangingTex;
    sf::Texture& climbTex;
    sf::Texture& rollTex;
    sf::Texture& elixirTex;
    sf::Texture& potionTex;
    sf::Texture& jumpAttackTex;
};

struct samuraiTextures {
    sf::Texture& idleTex;
    sf::Texture& walkTex;
    sf::Texture& runTex;
    sf::Texture& jumpTex;
    sf::Texture& attack1Tex;
    sf::Texture& attack2Tex;
    sf::Texture& attack3Tex;
    sf::Texture& specialTex;
    sf::Texture& arrowTex;
    sf::Texture& climbTex;
    sf::Texture& deadTex;
    sf::Texture& hurtTex;
    sf::Texture& rollTex;
    sf::Texture& elixirTex;
};

struct backgroundTextures {
    sf::Texture& loadingBackgroundTex;
    sf::Texture& createServerBackgroundTex;
    sf::Texture& singlePlayerBackgroundTex;
    sf::Texture& serverListBackgroundTex;
    sf::Texture& titleBackgroundTex;
    sf::Texture& arenaPictureTex;
    sf::Texture& howToPlayBackrgoundTex;
};