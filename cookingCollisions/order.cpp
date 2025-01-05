#include "order.h"
#include <math.h>

// Defines static attributes
std::vector<std::string> Order::availableTypes;

// Constructor for the Order class
Order::Order()
{
    dish = availableTypes[RandomNumber(0, availableTypes.size() - 1)];  // Randomly determines which dish to order
}

// Static method to generate a random integer in a range
int Order::RandomNumber(int min, int max)
{
    // Sets up a generator using a deterministic Mersenne Twister random number generator
    // std::random_device{}() generates a random seed to use for the generator
    static std::mt19937 rng{ std::random_device{}() };

    // The random engine (mt19937) generates raw random numbers in a fixed range which is specific to the engine used
    // In this case the range is 0 - 2^(32) - 1, for mt19937
    // The uniform_int_distribution simply maps the output of the generator to the range specified with uniform probability
    std::uniform_int_distribution<int> dist(min, max);

    return dist(rng);
}

// Draws the name of the dish to the screen
void Order::DrawDishText()
{   
    // Attempts to ensure that the text fits in the order banner
    int textWidth = MeasureText(dish.c_str(), fontSize);
    while (textWidth > size.x && fontSize > 0) {
        fontSize--;
        textWidth = MeasureText(dish.c_str(), fontSize);
    }

    // Centres and draws the text
    int x = screenPos.x + (size.x - textWidth) / 2;
    int y = screenPos.y + (size.y - fontSize) / 2;
    DrawText(dish.c_str(), x, y, fontSize, BLACK);
}

// Handles logic and drawing orders
void Order::Tick(float deltaTime)
{
    timeRemaining -= deltaTime;

    screenPos.x = 250.f * static_cast<float>(winWidth) / 800.f + (size.x + 20.f) * orderNum;  // Determines the position based on its number

    DrawRectangle(screenPos.x, screenPos.y, size.x, size.y, WHITE);

    unsigned char rValue = 255 * pow(1.f - timeRemaining/maxTime, 4);  // Determines the R values such that it increases over time
    DrawRectangle(screenPos.x + 5.f, size.y - 25.f, static_cast<int>(timeRemaining/maxTime * (size.x - 10.f)), 20, {rValue, 0, 0, 255});

    DrawDishText();
}