#include "counterUnit.h"
#include "config.h"

RecipeGraph& CounterUnit::recipes = RecipeGraph::GetInstance();

// Constructor for the CounterUnit class
CounterUnit::CounterUnit(const Vector2& pos, const std::array<bool, 4>& edges) :
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

// Checks if an item can be placed on a given unit
bool CounterUnit::CanPlace(const std::string& type) const
{
    // If the item being placed is compatible with what is already placed
    if (itemsPlaced[0] == nullptr) return 1;
    else if (recipes.FindCommonNode(itemsPlaced[0]->GetType(), type) != "NULL" && type != itemsPlaced[0]->GetType()) return 1;

    return 0;
}

// Removes the current item and/or anything placed on it
void CounterUnit::RemoveItems()
{
    for (auto item : itemsPlaced) {
        if (item != nullptr)
            item->RemoveItems();  // Removes any item placed
    }
    ClearPlaced();
}

// Handles logic for combining two items
void CounterUnit::CombineItems()
{
    if (itemsPlaced[0] == nullptr || itemsPlaced[1] == nullptr) return;  // If either item is empty, exit

    // Get the name of both items
    std::string type1 = itemsPlaced[0]->GetType();
    std::string type2 = itemsPlaced[1]->GetType();

    std::string combinedType = recipes.FindCommonNode(type1, type2);  // Find the result of combining the two items

    // If FindCommonNode returns NULL, the items cannot be combined
    if (combinedType != "NULL") {
        combineItems = true;
        RemoveItems();
        ClearPlaced();
        BaseItem* newItem = CreateCombinedItem(combinedType);  // Creates a new object based on the result of combining the items
        newItem->SetDimensions(50.f, 50.f);
        if (newItem) AddItem(newItem);  // If the new item as created successfully, place it on the current object
    }
}

// Creates a new object of a given type
BaseItem* CounterUnit::CreateCombinedItem(std::string& type)
{
    return new Ingredient(BaseItem::GetDishTextures()[type], type);
}

// Handles logic and drawing the units
void CounterUnit::Tick(float deltaTime)
{
    CombineItems();

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