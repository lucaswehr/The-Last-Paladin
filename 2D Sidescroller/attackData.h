#pragma once

enum class attackType
{
    normalAttacks,
    sprintAttack,
    jumpAttack,
    deflection,
    none
};

enum class samuraiAttackType
{
    Attack1,
    Attack2,
    Attack3
};

struct AttackHitbox
{
    sf::Vector2f size;
    sf::Vector2f offset;
};

// Changes the hitbox dynamically depending on the attack
inline AttackHitbox attackBoxTable[] = {

    {{35.f, 50.f}, {80.f, 0.f}},   // normal three attacks
    {{20.f, 50.f}, {65.f, 0.f}},   // Sprint Attack
    {{20.f, 35.f}, {40.f, -45.f}}, // Jump attack
    {{15.f, 50.f}, {55.f, 0.f}},   // Deflection
};

inline AttackHitbox samuraiAttackBoxTable[] = {

    {{25.f, 40.f}, {80.f, -120.f}},   // 1st Attack
    {{25.f, 50.f}, {80.f, -155.f}},   // 2nd Attack
    {{40.f, 20.f}, {55.f, -100.f}}, // 3rd attack
   
};

struct attackData
{
    float duration;
    float hitStart;
    float hitEnd;
    int damage;
};

static const attackData knightAttackTable[] =
{
    {0.30f, 0.24f, 0.30f, 25}, // Knight's 1st attack (5 frames)
    {0.28f, 0.21f, 0.28f, 25}, // Knight's 2nd and 3rd Attack (Each 4 frames)
    {0.60f, 0.35f, 0.50f, 30}, // Knight's Sprint attack (6 frames)
    {0.75f, 0.45f, 0.75f, 40},  // Knight's Jump Attack (5 Frames)
    {0.60f, 0.30f, 0.45f, 0}   // Knights' Deflection/Parry (4 Frames)
};

static const attackData samuraiAttackTable[] =
{
    {0.32f, 0.25f, 0.32f, 15}, // Samurai's 1st attack (5 frames)
    {0.50f, 0.30f, 0.50f, 15}, // Samurai's 2nd attack (5 frames)
    {0.48f, 0.32f, 0.48f, 20}, // Samurai's 3rd attack (6 frames)
};