#include "ParryDemo.hpp"

void ParryDemo::update(float dt)
{
    if (!attackingKnight.isStunned)
    attackingKnight.currentAnimation->update(dt);

    parryingKnight.currentAnimation->update(dt);

    attackingKnight.currentAnimation->getSprite().setPosition(
        attackingKnight.bodyBox.getPosition()
    );

    parryingKnight.currentAnimation->getSprite().setPosition(
        parryingKnight.bodyBox.getPosition()
    );

    parryingKnight.currentAnimation->setInitalDirectionToLeft();

    updateStun(dt);

    switch (state)
    {
    case ParryDemoState::Setup:

        setup();
        state = ParryDemoState::Idle;
        idleTimer = 0.f;

        break;
    case ParryDemoState::Idle:

        idleTimer += dt;

        if (idleTimer >= 1.f)
        {
            startFight();
            state = ParryDemoState::Attacking;
        }

        break;
    case ParryDemoState::Attacking:

        if (attackingKnight.currentAnimation->getCurrentFrame() >= 3)
        {
            attackingKnight.isStunned = true;
            attackingKnight.currentAnimation = &knightStunned;
            attackingKnight.currentAnimation->reset();

            attackingKnight.stunPhase = 0;
            attackingKnight.stunFrameTimer = 0.f;
            attackingKnight.stunTimer = 2.f;
            attackingKnight.stunHoldFrame = 1;


            state = ParryDemoState::Parried;
            timer = 0.f;
        }

        break;
    case ParryDemoState::Parried:

        timer += dt;

        if (timer >= 4.f)
        {
            state = ParryDemoState::Reset;
        }

        break;
    case ParryDemoState::Reset:

        setup();
        state = ParryDemoState::Idle;

        idleTimer = 0.f;
        timer = 0.f;

        break;
    }
}

void ParryDemo::draw(sf::RenderWindow& window)
{
    window.draw(attackingKnight.currentAnimation->getSprite());
    window.draw(parryingKnight.currentAnimation->getSprite());

   // window.draw(attackingKnight.bodyBox);
   // window.draw(attackingKnight.attackBox);

  // window.draw(parryingKnight.bodyBox);
  //  window.draw(parryingKnight.parryBox);
}

void ParryDemo::reset()
{
    state = ParryDemoState::Reset;
}

void ParryDemo::setup()
{
    attackingKnight.currentAnimation = &attackingKnightIdle;
    parryingKnight.currentAnimation = &parryingKnightIdle;

    attackingKnight.currentAnimation->reset();
    parryingKnight.currentAnimation->reset();

    knightAttack.reset();
    knightSpecial.reset();
    knightStunned.reset();

    attackingKnight.bodyBox.setPosition({ 850.f, 310.f });

    parryingKnight.bodyBox.setPosition({ 1050.f,310.f });

    attackingKnight.attackBox.setPosition({
        attackingKnight.bodyBox.getPosition().x + 30.f,
        attackingKnight.bodyBox.getPosition().y
        });

    parryingKnight.parryBox.setPosition({
        parryingKnight.bodyBox.getPosition().x - 25.f,
        parryingKnight.bodyBox.getPosition().y
        });

    attackingKnight.isStunned = false;
    parryingKnight.isStunned = false;

    state = ParryDemoState::Idle;
    timer = 0.f;
}

void ParryDemo::startFight()
{
    attackingKnight.currentAnimation = &knightAttack;
    parryingKnight.currentAnimation = &knightSpecial;

    attackingKnight.currentAnimation->reset();
    parryingKnight.currentAnimation->reset();

}

void ParryDemo::updateStun(float dt)
{
    if (!attackingKnight.isStunned)
        return;

    attackingKnight.stunTimer -= dt;
    attackingKnight.stunFrameTimer += dt;

    float frameDuration = 0.08f;

    if (attackingKnight.stunPhase == 0)
    {
        if (attackingKnight.stunFrameTimer >= frameDuration)
        {
            attackingKnight.stunFrameTimer = 0.f;

            if (attackingKnight.currentAnimation->getCurrentFrame() < attackingKnight.stunHoldFrame)
            {
                attackingKnight.currentAnimation->nextFrame();
            }
            else
            {
                attackingKnight.stunPhase = 1;
            }
        }
    }
    else if (attackingKnight.stunPhase == 1)
    {
        attackingKnight.currentAnimation->setFrame(attackingKnight.stunHoldFrame);

        if (attackingKnight.stunTimer <= 0.f)
        {
            attackingKnight.stunPhase = 2;
            attackingKnight.stunFrameTimer = 0.f;
        }
    }
    else if (attackingKnight.stunPhase == 2)
    {
        if (attackingKnight.stunFrameTimer >= frameDuration)
        {
            attackingKnight.stunFrameTimer = 0.f;

            if (attackingKnight.currentAnimation->getCurrentFrame() > 0)
            {
                attackingKnight.currentAnimation->setFrame(
                    attackingKnight.currentAnimation->getCurrentFrame() - 1
                );
            }
            else
            {
                attackingKnight.isStunned = false;

                attackingKnight.currentAnimation = &attackingKnightIdle;
                attackingKnight.currentAnimation->reset();

                state = ParryDemoState::Reset;
                timer = 0.f;
            }
        }
    }
}
