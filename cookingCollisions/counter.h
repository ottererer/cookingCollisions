#pragma once

#include "raylib.h"
#include "counterUnit.h"
#include <vector>
#include <array>
#include "config.h"

class Counter
{
public:
    void Tick(float deltaTime);
    void CreateCounter();
    std::array<bool, 4> CheckEdges(int row, int col);

    // Getters
    std::vector<CounterUnit*> GetUnitsPointers();
    std::vector<CounterUnit>& GetUnits() { return counterUnits; }

private:
    std::vector<CounterUnit> counterUnits;
    std::vector<std::vector<bool>> tileMap = {
        {false, false, true, true, true, true, false, false},
        {false, false, false, false, false, false, false, false},
        {true, true, false, true, true, false, true, true},
        {true, true, false, true, true, false, true, true},
        {true, false, false, false, false, false, false, true},
        {true, true, false, true, true, false, true, true},
        {true, true, false, true, true, false, true, true},
        {false, false, false, false, false, false, false, false},
    };
};
