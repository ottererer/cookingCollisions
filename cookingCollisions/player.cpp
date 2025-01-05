#include "player.h"

// Works out the angle to a given target
float Player::AngleToTarget(Vector2 targetPos)
{
    float angle = Radians(viewDir);  // Converts the view angle into radians
    Vector2 viewVector = Vector2Normalize(Vector2{ sin(angle), cos(angle) });  // Creates a view vector of size 1
    Vector2 targetRelative = Vector2Normalize(Vector2Subtract(screenPos, targetPos));  // Gets the normalised position vector of the target relative to the player

    // Uses the cosine rule to work out the angle
    float a = Vector2Length(Vector2Add(targetRelative, viewVector));
    float b = Vector2Length(viewVector);
    float c = Vector2Length(targetRelative);
    float targetAngle = acos((pow(b, 2) + pow(c, 2) - pow(a, 2)) / (2 * b * c));

    return targetAngle;
}

// Calculates the position of and draws the arms of the player
void Player::DrawArms(float viewDir)
{
    viewDir = Radians(viewDir);  // Converts the view angle into radians
    // Calculates where the arm starts and ends
    Vector2 startPos = { screenPos.x + cos(viewDir) * -(playerRadius - armWidth / 2),
            screenPos.y + sin(viewDir) * (playerRadius - armWidth / 2) };
    Vector2 endPos = { startPos.x + cos(viewDir + pi / 2.f) * -armLength,
            startPos.y + sin(viewDir + pi / 2.f) * armLength };
    DrawLineEx(startPos, endPos, armWidth, DARKBLUE);

    // Calculates where the arm starts and ends
    startPos = { screenPos.x + cos(viewDir) * (playerRadius - armWidth / 2),
            screenPos.y + sin(viewDir) * -(playerRadius - armWidth / 2) };
    endPos = { startPos.x + cos(viewDir + pi / 2.f) * -armLength,
            startPos.y + sin(viewDir + pi / 2.f) * armLength };
    DrawLineEx(startPos, endPos, armWidth, DARKBLUE);
}

// Determine - if any - which counter unit the player is currently looking at
void Player::HandleSelect(std::vector<CounterUnit*> units)
{
    CounterUnit* selectedUnit = nullptr;
    float smallestAngle = 10.f;  // Sets a large initial angle
    for (auto* unit : units) {
        unit->SetSelected(false);  // Initially deselects each unit

        // If the distance and angle to the target falls within the bounds, it is valid
        if (AngleToTarget(unit->GetCentre()) <= visionAngle && DistanceToTarget(unit->GetCentre()) <= visionDist) {
            // If the the angle to the target is less than the currently smallest angle
            if (abs(AngleToTarget(unit->GetCentre())) < smallestAngle) {
                selectedUnit = unit;
                smallestAngle = abs(AngleToTarget(unit->GetCentre()));  // Ensures the angle is positibe
            }
        }
    }

    if (selectedUnit != nullptr) selectedUnit->SetSelected(true);
}

// Handles player interactiosn with items
void Player::HandleItems(std::vector<CounterUnit*> units)
{
    if (IsKeyPressed(KEY_E)) {
        for (auto* unit : units) {
            if (!unit->GetSelected()) continue;
            auto* itemPlaced = unit->GetPlaced();

            // If the player is not currently holding an item
            if (itemHeld == nullptr) {
                // If there is nothing placed
                if (itemPlaced == nullptr) continue;

                // If the item placed can be picked up
                else if (itemPlaced->CanPickup()) {
                    SetHolding(itemPlaced);
                    unit->ClearPlaced();
                }
                // If there is nothing placed on the item that cannot itself be picked up
                else if (itemPlaced->GetPlaced() == nullptr) continue;

                // Otherwise pick up the item
                else {
                    SetHolding(itemPlaced->GetPlaced());
                    itemPlaced->ClearItems();
                }
            }

            // If the currently selected unit is a bin
            else if (itemHeld != nullptr && unit->GetType() == "bin") {
                if (itemHeld->GetType() == "plate" && itemHeld->GetPlaced() != nullptr) itemHeld->RemoveItems(false);
                else {
                    itemHeld->RemoveItems();
                    itemHeld = nullptr;
                }
            }

            // If the currently selected unit is a delivery unit
            else if (itemHeld != nullptr && unit->GetType() == "delivery") {
                if (itemHeld->GetPlaced() != nullptr && itemHeld->GetPlaced()->CanServe()) {
                    unit->AddServing(itemHeld);
                    itemHeld->SetPos(unit->GetCentre());
                    itemHeld->SetServing(true);
                    SetHolding(nullptr);
                }
            }

            // If there is nothing currently placed
            else if (itemPlaced == nullptr) {
                unit->AddItem(itemHeld);
                SetHolding(nullptr);
            }

            // If the item is compatible with what is already placed
            else if (itemPlaced->CanPlace(itemHeld->GetType())) {
                itemPlaced->AddItem(itemHeld);
                SetHolding(nullptr);
                if (itemPlaced->GetType() == "frying pan") {
                    itemPlaced->ResetTimer();
                }
            }

            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                if (itemPlaced->GetType() == "plate" && itemPlaced->GetPlaced() != nullptr) {
                    SetHolding(itemPlaced->GetPlaced());
                    itemPlaced->ClearItems();
                }
            }
        }
    }
    // Handles actions, e.g. chopping
    if (IsKeyPressed(KEY_F)) {
        for (auto* unit : units) {
            auto* itemPlaced = unit->GetPlaced();
            if (itemPlaced == nullptr) continue;
            if (itemPlaced->GetPlaced() == nullptr) continue;
            itemPlaced->HandleCooking();
        }
    }
}

// Works out where carried items should be drawn
void Player::CarryItem()
{
    Vector2 offset{ 0.f, 0.f };
    float itemAngle = Radians(viewDir);

    // Works out the vertical and horizontal offset to the player
    offset.x = sin(itemAngle) * holdingDistance;
    offset.y = cos(itemAngle) * holdingDistance;

    // Updates the position and rotation of the object to match the player
    itemHeld->SetPos(Vector2{ screenPos.x + offset.x, screenPos.y + offset.y });
    itemHeld->SetRot(-viewDir + 180.f);
    if (itemHeld->GetPlaced() != nullptr) itemHeld->GetPlaced()->SetRot(-viewDir + 180.f);
}

// Checks if and if so how, the player is colliding with a unit
Vector2 Player::CheckCollisions(std::vector<CounterUnit*> units, float deltaTime)
{
    // Gets the position of the player last frame
    Vector2 tempPos = Vector2Subtract(screenPos, Vector2Scale(Vector2Normalize(moveDir), moveSpeed * deltaTime));
    Vector2 edgePos{ -1.f, -1.f };

    for (CounterUnit* unit : units) {
        // Checks for any overlap between a unit and the player
        bool overlap = !(screenPos.x + playerRadius <= unit->GetPos().x ||
            screenPos.x - playerRadius >= unit->GetPos().x + unit->GetWidth() ||
            screenPos.y + playerRadius <= unit->GetPos().y ||
            screenPos.y - playerRadius >= unit->GetPos().y + unit->GetWidth());

        // If there is overlap, determines where the edge is
        if (overlap) {
            // For collisions in the y-axis
            if (!(tempPos.y + playerRadius <= unit->GetPos().y ||
                tempPos.y - playerRadius >= unit->GetPos().y + unit->GetWidth())) {
                if (unit->GetCentre().x > screenPos.x) edgePos.x = unit->GetPos().x - playerRadius;
                else edgePos.x = unit->GetPos().x + unit->GetWidth() + playerRadius;
                }

            // For collisions in the x-axis
            if (!(tempPos.x + playerRadius <= unit->GetPos().x ||
                tempPos.x - playerRadius >= unit->GetPos().x + unit->GetWidth())) {
                if (unit->GetCentre().y > screenPos.y) edgePos.y = unit->GetPos().y - playerRadius * 2;
                else edgePos.y = unit->GetPos().y + unit->GetWidth() + playerRadius * 2;
            }
        }
    }
    return edgePos;
}

// Moves the player to resolve any collisions
void Player::ResolveCollisions(std::vector<CounterUnit*> units, float deltaTime)
{
    Vector2 collisionEdge = CheckCollisions(units, deltaTime);
    if (collisionEdge != Vector2{ -1.f, -1.f }) {
        if (collisionEdge.x != -1.f) {
            screenPos.x = collisionEdge.x;
        }
        if (collisionEdge.y != -1.f) {
            if (collisionEdge.y > screenPos.y) screenPos.y = collisionEdge.y - playerRadius;
            else screenPos.y = collisionEdge.y + playerRadius;
        }
    }
}

void Player::HandleBounds(float deltaTime)
{
    Vector2 tempPos = Vector2Add(screenPos, Vector2Scale(Vector2Normalize(moveDir), moveSpeed * deltaTime));
    if (tempPos.x + playerRadius > winWidth) {
        tempPos.x = winWidth - playerRadius;
    }
    else if (tempPos.x - playerRadius < 0.f) {
        tempPos.x = playerRadius;
    }

    if (tempPos.y + playerRadius > winHeight) {
        tempPos.y = winHeight - playerRadius;
    }
    else if (tempPos.y - playerRadius < 0.f) {
        tempPos.y = playerRadius;
    }

    screenPos = tempPos;
}

float Player::ClampPlate(float viewDir, float rotation, float clampTarget)
{
    if ((viewDir <= clampTarget && viewDir - rotation >= clampTarget)
        || (viewDir >= clampTarget && viewDir - rotation <= clampTarget)) viewDir = clampTarget;

    return viewDir;
}

float Player::HandleMovement(float deltaTime)
{
    moveDir = { 0, 0 };
    float viewSum = 0.f;
    if (viewDir > 360.f) viewDir -= 360.f;
    else if (viewDir < 0.f) viewDir += 360.f;
    if (IsKeyDown(KEY_D)) {
        moveDir.x += 1.f;
        viewSum += 90.1f;
    }
    if (IsKeyDown(KEY_A)) {
        moveDir.x -= 1.0f;
        viewSum += 270.1f;
    }
    if (IsKeyDown(KEY_S)) {
        moveDir.y += 1.0f;
        if (moveDir.x == 1.f) viewSum += 0.1f;
        else viewSum += 360.1f;
    }
    if (IsKeyDown(KEY_W)) {
        moveDir.y -= 1.0f;
        viewSum += 180.1f;
    }

    return viewSum;
}

void Player::Tick(float deltaTime)
{
    float viewSum = HandleMovement(deltaTime);
    if (Vector2Length(moveDir) != 0) {
        HandleBounds(deltaTime);
        float rotation = rotateSpeed * deltaTime;
        float targetDir = (viewSum - remainder(viewSum, 1)) / (10 * remainder(viewSum, 1));
        float lowerBound;
        if (targetDir < 180.f) lowerBound = targetDir + 180.f;
        else lowerBound = targetDir - 180.f;
        if (lowerBound <= 180.f) {
            if (viewDir < targetDir && viewDir > lowerBound) viewDir += rotation;
            else {
                rotation *= -1; viewDir += rotation;
            }
        }
        else {
            if (viewDir < targetDir || viewDir > lowerBound) viewDir += rotation;
            else {
                rotation *= -1; viewDir += rotation;
            }
        }

        viewDir = ClampPlate(viewDir, rotation, targetDir);

    }
    if (itemHeld != nullptr) CarryItem();
}

void Player::Draw()
{
    DrawArms(viewDir);
    DrawCircle(screenPos.x, screenPos.y, playerRadius, BLUE);
}