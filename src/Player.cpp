#include "../include/Player.hpp"
#include "../include/Enemy.hpp"
#include "../include/Utils.hpp"
#include <algorithm>
#include <raylib.h>
#include <raymath.h>

void Player::Init() {
    // player
    pos = {100, 100};
    dir = {0, 0};
    lastDir = {0, 0};
    cursorPos = {0, 0};

    texture = LoadTexture("../sprites/husk_one.png");

    speed = 10.0;

    damage = 5.0;
    atkSpeed = 100.0;
    atkCooldown = 1.0; // once a second
    atkDuration = 0.5;
    atkReady = true;
    atkTexture = LoadTexture("../sprites/crescent_slash.png");
    atkHitbox = {0, 0, 4, 16};
    knockback = 1000.0;

    dashSpeed = 600.0;
    dashDur = 0.2;
    dashCooldown = 1.0;
    dashTimer = 0.0;
    dashReady = true;

    friction = 4000.0;

    velocity = 200.0;
    currentVelocity = 0.0;
    maxVelocity = 200.0;
    moving = false;

    width = 12.0;
    height = 26.0;

    health = 50.0;

    hitbox = {0, 0, width, height};

    // camera
    camera = {0};
    camera.target = pos;
    camera.offset = {1920 / 2.0, 1080 / 2.0}; // TODO: make 1920/1080 variable names
    camera.rotation = 0.0;
    camera.zoom = 2.0f;
}

void Player::Move() {
    float delta = GetFrameTime();
    dashTimer -= delta;

    if (IsKeyPressed(KEY_LEFT_SHIFT) && dashReady) {
        dashTimer = dashDur;
        dashReady = false;
    }

    if (dashTimer > 0) {
        pos.x += lastDir.x * dashSpeed * delta;
        pos.y += lastDir.y * dashSpeed * delta;
    } else {
        if (dashTimer <= -dashCooldown) {
            dashReady = true;
        }

        dir = {0, 0};

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
            dir.y -= 1;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
            dir.y += 1;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            dir.x -= 1;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            dir.x += 1;

        if (dir.x != 0 || dir.y != 0) {
            dir = Vector2Normalize(dir);
            moving = true;
            lastDir = dir;
            currentVelocity += speed * velocity * delta;
            currentVelocity = std::min(currentVelocity, maxVelocity);
        } else {
            moving = false;
            currentVelocity -= friction * delta;
            currentVelocity = std::max(0.0f, currentVelocity);
        }

        pos.x += lastDir.x * currentVelocity * delta;
        pos.y += lastDir.y * currentVelocity * delta;

        hitbox.x = pos.x;
        hitbox.y = pos.y;

        currentVelocity = std::min(currentVelocity, maxVelocity);
    }

    camera.target = Vector2Lerp(camera.target, pos, 20.0f * delta);
}

void Player::Attack(std::vector<Enemy> &enemies) {
    atkReady = false;

    for (auto &enemy : enemies) {
        if (CheckCollisionRecs(atkHitbox, enemy.hitbox)) {
            enemy.Knockback(pos, knockback);
        }
    }
}

void Player::Update(std::vector<Enemy> &enemies) {
    cursorPos = GetScreenToWorld2D(GetMousePosition(), camera);

    Attack(enemies);

    for (const auto &enemy : enemies) {
        bool isColliding = Utils::CheckRectangles(hitbox, enemy.hitbox);
        if (isColliding) {
            // TODO: remove player health && add iframes
            break;
        }
    }
}

void Player::Draw() {
    // cursor
    DrawCircleV(cursorPos, 10.0f, RED);

    Vector2 direction = Vector2Subtract(cursorPos, pos);
    float distance = Vector2Length(direction);
    float rotation = atan2f(direction.y, direction.x) * RAD2DEG;

    Vector2 playerCenter = {pos.x + texture.width / 2.0f, pos.y + texture.height / 2.0f};

    // atk
    const float maxRadius = 30.0f;

    Vector2 atkPosition;
    Vector2 normalized = Vector2Normalize(direction);
    atkPosition = Vector2Add(playerCenter, Vector2Scale(normalized, maxRadius));

    if (!atkReady) {
        atkHitbox.width = (float)atkTexture.width;
        atkHitbox.height = (float)atkTexture.height;

        Vector2 txtOrigin = {atkHitbox.width / 2, atkHitbox.height / 2};

        Rectangle destRec = {atkPosition.x, atkPosition.y, (float)atkTexture.width, (float)atkTexture.height};
        DrawTexturePro(atkTexture, Rectangle{0, 0, (float)atkTexture.width, (float)atkTexture.height}, destRec,
                       txtOrigin, rotation, WHITE);

        Rectangle rotHitbox = {atkPosition.x, atkPosition.y, atkHitbox.width, atkHitbox.height};
        // DrawRectanglePro(rotHitbox, txtOrigin, rotation, RED);

        atkHitbox = rotHitbox;
    }

    // player
    DrawTextureRec(texture, Rectangle{0, 0, (float)texture.width, (float)texture.height}, pos, WHITE);
}
