#include "counter.h"

// Logic for generating the counter units based on the tile map
void Counter::CreateCounter()
{
    for (int row = 0; row < static_cast<int>(tileMap.size()); row++) {
        for (int col = 0; col < static_cast<int>(tileMap[row].size()); col++) {
            // If the tile is marked
            if (tileMap[row][col]) {
                std::array<bool, 4> edgesList = CheckEdges(row, col);  // Checks for neighbouring units

                // Determines the position of the unit
                float x = static_cast<float>(col) * static_cast<float>(winWidth) / 10.f + static_cast<float>(winWidth) / 10.f;
                float y = static_cast<float>(row) * static_cast<float>(winHeight) / 10.f + static_cast<float>(winHeight) / 10.f;

                counterUnits.emplace_back(Vector2{ x, y }, edgesList);  // Adds the unit to the list
            }
        }
    }
}

// Checks for neighbouring units
std::array<bool, 4> Counter::CheckEdges(int row, int col)
{
    std::array<bool, 4> edgesList = { true, true, true, true };

    // Unless it is at the edge, checks the unit to the left
    if (row != 0) {
        if (tileMap[row - 1][col]) {
            edgesList[0] = false;
        }
    }

    // Unless it is at the edge, checks the unit to the right
    if (row != static_cast<int>(tileMap.size()) - 1) {
        if (tileMap[row + 1][col]) {
            edgesList[2] = false;
        }
    }

    // Unless it is at the top, checks the unit above
    if (col != 0) {
        if (tileMap[row][col - 1]) {
            edgesList[3] = false;
        }
    }

    // Unless it is at the bottom, checks the unit below
    if (col != static_cast<int>(tileMap[row].size()) - 1) {
        if (tileMap[row][col + 1]) {
            edgesList[1] = false;
        }
    }

    return edgesList;
}

// Returns the units as a list of pointers
std::vector<CounterUnit*> Counter::GetUnitsPointers()
{
    std::vector<CounterUnit*> unitPointers;
    unitPointers.reserve(counterUnits.size());  // Reserves space in the vector to eliminate the need for dynamic resizing
    for (auto& unit : counterUnits) {
        unitPointers.push_back(&unit);  // Gets the address of a unit and adds it to the list
    }
    return unitPointers;
}

// Handles logic and drawing the units
void Counter::Tick(float deltaTime)
{
    // Iterates through and draws each unit
    for (auto& unit : counterUnits) {
        unit.Tick(deltaTime);
    }

    // Draws the outline around selected units
    for (auto& unit : counterUnits) {
        if (unit.GetSelected()) unit.DrawSelected();
    }
}