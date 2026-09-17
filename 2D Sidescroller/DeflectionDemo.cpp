#include "DeflectionDemo.hpp"

void DeflectionDemo::update(float dt)
{
    samurai.currentAnimation->update(dt);
    knight.currentAnimation->update(dt);
    samurai.currentAnimation->getSprite().setPosition(samurai.bodyBox.getPosition());
    knight.currentAnimation->getSprite().setPosition(knight.deflectionBox.getPosition());
    knight.currentAnimation->setInitalDirectionToLeft();

    switch (state)
    {
    case DeflectionDemoState::SamuraiShooting:
        spawnArrow();

        if (arrow.active)
           state = DeflectionDemoState::ArrowFlying;
        break;

    case DeflectionDemoState::ArrowFlying:
        updateArrow(dt);
        processDeflection();
        break;

    case DeflectionDemoState::ArrowDeflected:
        updateArrow(dt);
        processArrowHit();
        break;

    case DeflectionDemoState::SamuraiDead:
        timer += dt;

        if (timer >= 2.f)
        {
            setup();
        }
        break;

    case DeflectionDemoState::Reset:
        setup();
        knight.currentAnimation->getSprite().setScale({ -2.f, 2.f });
        arrow.sprite.setScale({ 2,2 });
        break;
    }
}

void DeflectionDemo::draw(sf::RenderWindow& window)
{
    window.draw(samurai.currentAnimation->getSprite());
    window.draw(knight.currentAnimation->getSprite());

    if (arrow.active)
        window.draw(arrow.sprite);

    //// Debug hitboxes
    //window.draw(samurai.bodyBox);
    //window.draw(knight.deflectionBox);

   /* if (arrow.active)
        window.draw(arrow.hitbox);*/
}

void DeflectionDemo::setup()
{
    knight.deflectionBox.setSize({ 35.f, 40.f });
    knight.deflectionBox.setScale({ -2,2.5 });
    knight.deflectionBox.setOrigin({ -30.f, -20.f });
    knight.deflectionBox.setFillColor(sf::Color(255, 0, 0, 128));

    samurai.bodyBox.setSize({ 32.f, 48.f });
    samurai.bodyBox.setScale({ 2,2.5 });
    samurai.bodyBox.setOrigin({ 17.f, -20.f });
    samurai.bodyBox.setFillColor(sf::Color(255, 0, 0, 128));

    arrow.hitbox.setFillColor(sf::Color(255, 0, 0, 128));
    arrow.hitbox.setSize({ 30.f,10.f });
    arrow.hitbox.setScale({ 2,3 });

    samurai.currentAnimation = &samuraiSpecial;
    knight.currentAnimation = &knightIdle;

    samurai.currentAnimation->reset();
    knight.currentAnimation->reset();

    // Reset arrow
    arrow.active = false;
    arrow.reflected = false;
    arrow.velocity = { 0.f, 0.f };

    // Reset animations
    samuraiSpecial.reset();
    knightIdle.reset();
    knightSpecial.reset();
    samuraiDead.reset();

    samurai.bodyBox.setPosition({ 550.f, 310.f });
    knight.deflectionBox.setPosition({ 1400.f, 310.f });

    samurai.currentAnimation->getSprite().setPosition(samurai.bodyBox.getPosition());
    knight.currentAnimation->getSprite().setPosition(knight.deflectionBox.getPosition());

    samurai.dead = false;
    knight.dead = false;

    state = DeflectionDemoState::SamuraiShooting;
    timer = 0.f;

    arrow.sprite.setScale({ 2.f, 2.f });
}

void DeflectionDemo::spawnArrow()
{
    if (samurai.currentAnimation->getCurrentFrame() != 12) return;

    knight.currentAnimation = &knightSpecial;

    arrow.active = true;
    arrow.reflected = false;

    arrow.velocity = { 2000.f, 0.f };

    sf::Vector2f samuraiPos =
        samurai.currentAnimation->getSprite().getPosition();

    arrow.sprite.setPosition({
        samuraiPos.x,
        samuraiPos.y 
        });
}

void DeflectionDemo::updateArrow(float dt)
{
    if (!arrow.active)
        return;

    arrow.velocity.y += arrow.gravity * dt;

    arrow.sprite.move(arrow.velocity * dt);

    arrow.hitbox.setPosition({ arrow.sprite.getPosition().x, arrow.sprite.getPosition().y + 15.f });

    if (arrow.velocity.x < 0.f)
        arrow.hitbox.setOrigin({ 45,-10 });
    else if (arrow.velocity.x > 0.f)
        arrow.hitbox.setOrigin({ 20,-10 });
}

void DeflectionDemo::processDeflection()
{
    if (!arrow.active)
        return;

    if (arrow.reflected)
        return;

    if (arrow.hitbox.getGlobalBounds().findIntersection(
        knight.deflectionBox.getGlobalBounds()))
    {
        arrow.reflected = true;

        arrow.velocity.x *= -1.f;

        arrow.sprite.setScale({ -2.f, 2.f });

        state = DeflectionDemoState::ArrowDeflected;
    }
}

void DeflectionDemo::processArrowHit()
{
    if (!arrow.active)
        return;

    if (!arrow.reflected)
        return;

    if (arrow.hitbox.getGlobalBounds().findIntersection(
        samurai.bodyBox.getGlobalBounds()))
    {
        samurai.dead = true;
        arrow.active = false;

        samurai.currentAnimation = &samuraiDead;

        state = DeflectionDemoState::SamuraiDead;
    }
}

void DeflectionDemo::reset()
{
    state = DeflectionDemoState::Reset;
}
