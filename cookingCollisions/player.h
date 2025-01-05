#pragma once

#include "config.h"
#include <raylib.h>
#include "raymath.h"
#include "counterUnit.h"
#include <cmath>
#include <vector>
#include <iostream>
#include <tuple>
#include <algorithm>
#include "counterUnit.h"
#include "items.h"

class Player
{
public:
    Player(Vector2 pos) : screenPos(pos) {};
    float DistanceToTarget(Vector2 targetPos) { return Vector2Distance(screenPos, targetPos); }
    float AngleToTarget(Vector2 targetPos);
    float Radians(float angle) { return angle * pi / 180.f; }
    void HandleSelect(std::vector<CounterUnit*> units);
    void HandleItems(std::vector<CounterUnit*> units);
    void CarryItem();
    Vector2 CheckCollisions(std::vector<CounterUnit*>, float deltaTime);
    void ResolveCollisions(std::vector<CounterUnit*>, float deltaTime);
    void HandleBounds(float deltaTime);
    void DrawArms(float viewDir);
    float ClampPlate(float viewDir, float rotation, float clampTarget);
    float HandleMovement(float deltaTime);
    void Tick(float delta_time);
    void Draw();

    // Getters
    Vector2 GetPos() { return screenPos; }

    // Setters
    void SetPos(Vector2 pos) { screenPos = pos; }
    void SetHolding(BaseItem* item) { itemHeld = item; }

private:
    Vector2 screenPos{};
    float moveSpeed = 300.f * static_cast<float>(winWidth) / 800.f;
    float playerRadius = 25.f * static_cast<float>(winWidth) / 800.f;
    float armLength = 30.f * static_cast<float>(winWidth) / 800.f;
    float armWidth = 8.f * static_cast<float>(winWidth) / 800.f;
    Vector2 moveDir{};
    float viewDir = 0.f;
    float rotateSpeed = 360.f * 3;

    BaseItem* itemHeld = nullptr;
    float holdingDistance = 30.f;

    float visionAngle = Radians(60.f);
    float visionDist = 120.f;
};