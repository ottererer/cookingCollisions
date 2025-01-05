#pragma once

#include <raylib.h>
#include <array>
#include <iostream>
#include <string>
#include "items.h"

class CounterUnit
{
public:
    CounterUnit(Vector2 pos, std::array<bool, 4> edges);
    void AddItem(BaseItem* item);
    void AddServing(BaseItem* item) { servingItems.push_back(item); }
    void ClearPlaced() { itemsPlaced = { nullptr, nullptr }; }
    void DrawSelected();
    void Tick(float deltaTime);

    // Getters
    Vector2 GetCentre() { return Vector2{ screenPos.x + counterWidth / 2.f, screenPos.y + counterWidth / 2.f }; }
    Vector2 GetPos() { return screenPos; }
    float GetWidth() { return counterWidth; }
    bool GetSelected() { return isSelected; }
    BaseItem* GetPlaced() { return itemsPlaced[0]; }
    std::string GetType() { return unitType; }
    std::string GetSourceType() { return sourceType; }

    // Setters
    void SetSelected(bool state) { isSelected = state; }
    void SetType(std::string type, std::string spawnerType = "none") { unitType = type; sourceType = spawnerType; }

private:
    Vector2 screenPos{};
    std::array<BaseItem*, 2> itemsPlaced = { nullptr, nullptr };
    std::vector<BaseItem*> servingItems;
    std::array<bool, 4> counterEdges;

    bool isSelected = false;

    std::string unitType = "default";
    std::string sourceType = "none";

    float counterWidth;
    float outlineThickness = 2.f;
    float edgeThickness = 4.f;
};