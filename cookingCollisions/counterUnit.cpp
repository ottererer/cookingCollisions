#include "counterUnit.h"
#include "config.h"

// Constructor for the CounterUnit class
CounterUnit::CounterUnit(Vector2 pos, std::array<bool, 4> edges) :
    screenPos(pos)
{
    for (int i = 0; i < 4; ++i) {
        counterEdges[i] = edges[i];
    }

    counterWidth = static_cast<float>(winHeight) / 10.f;
}

// Adds an item to the counter unit
void CounterUnit::AddItem(BaseItem* item)
{
    // If empty, adds the item to the first slot
    if (itemsPlaced[0] == nullptr) {
        itemsPlaced[0] = item;
    }

    // Otherwise adds it to the second
    else {
        itemsPlaced[1] = item;
    }
}

// Draws an outline showing that the unit is currently selected
void CounterUnit::DrawSelected()
{
    Rectangle selectedRec = { screenPos.x - outlineThickness, screenPos.y - outlineThickness, counterWidth + outlineThickness * 2, counterWidth + outlineThickness * 2 };
    DrawRectangleLinesEx(selectedRec, outlineThickness, WHITE);
}

// Handles logic and drawing the units
void CounterUnit::Tick(float deltaTime)
{
    // Handles drawing additional lines to represent edges of the counter
    if (counterEdges[0]) {
        DrawLineEx(screenPos, Vector2{ screenPos.x + counterWidth, screenPos.y }, edgeThickness, BLACK);
    }
    if (counterEdges[1]) {
        DrawLineEx(Vector2{ screenPos.x + counterWidth, screenPos.y }, Vector2{ screenPos.x + counterWidth, screenPos.y + counterWidth }, edgeThickness, BLACK);
    }
    if (counterEdges[2]) {
        DrawLineEx(Vector2{ screenPos.x + counterWidth, screenPos.y + counterWidth }, Vector2{ screenPos.x, screenPos.y + counterWidth }, edgeThickness, BLACK);
    }
    if (counterEdges[3]) {
        DrawLineEx(Vector2{ screenPos.x, screenPos.y + counterWidth }, screenPos, edgeThickness, BLACK);
    }

    DrawRectangle(screenPos.x, screenPos.y, counterWidth, counterWidth, BEIGE);  // The main body of the unit

    // Additional features if the unit is a bin
    if (unitType == "bin") {
        DrawCircle(screenPos.x + counterWidth / 2.f, screenPos.y + counterWidth / 2.f, counterWidth / 2.f - counterWidth / 6.f, { 80, 80, 80, 255 });
        DrawCircle(screenPos.x + counterWidth / 2.f, screenPos.y + counterWidth / 2.f, counterWidth / 2.f - counterWidth / 4.f, BLACK);
    }

    // Additional features if the unit is a delivery unit
    else if (unitType == "delivery") {
        DrawRectangle(screenPos.x + 10.f, screenPos.y, counterWidth - 20.f, counterWidth - 10.f, BLACK);
        DrawRectangle(screenPos.x + 10.f, screenPos.y - counterWidth, counterWidth - 20.f, 2 * counterWidth - 10.f, BLACK);
    }
    
    // Sets the position of any placed items to the unit's position
    for (auto* item : itemsPlaced) {
        if (item != nullptr) {
            item->SetPos(GetCentre());
        }
    }

    // Makes items being served move up and despawn
    for (auto* item : servingItems) {
        if (item != nullptr) {
            item->MovePos({ 0, -20 * deltaTime });  // Moves the item up

            // If the item moves off the screen
            if (item->GetPos().y < -50) {
                item->RemoveItems();  // Removes the item from the game
                servingItems.erase(servingItems.begin());
            }
        }
    }
}