#include "order.h"
#include <math.h>

// Defines static attributes
std::vector<std::string> Order::availableTypes;
std::unordered_map<std::string, Texture2D> Order::dishTextures;

// Constructor for the Order class
Order::Order()
{
    dish = availableTypes[RandomNumber(0, availableTypes.size() - 1)];  // Randomly determines which dish to order
}

void Order::SetupOrders(std::unordered_map<std::string, Texture2D> textures)
{
    dishTextures = textures;
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
void Order::DrawDish()
{   
    // The selects the section of the texture to draw (the whole texture)
    Rectangle source = { 0, 0, (float)GetTextures()[dish].width, (float)GetTextures()[dish].height };

    // The rect that determines the position and dimensions of the texture
    Rectangle dest = {
        GetPos().x + GetSize().x / 2.f,
        GetPos().y + GetSize().y / 2.5f,
        GetSize().x,
        GetSize().x
    };

    // What point on the texture is treated as the origin (the centre)
    Vector2 origin = {
        (GetSize().x) / 2.f,
        (GetSize().x) / 2.f
    };

    DrawTexturePro(GetTextures()[dish], source, dest, origin, 0.f, WHITE);
}

// Handles logic and drawing orders
void Order::Tick(float deltaTime)
{
    timeRemaining -= deltaTime;

    screenPos.x = 250.f * static_cast<float>(winWidth) / 800.f + (size.x + 20.f) * orderNum;  // Determines the position based on its number

    DrawRectangle(screenPos.x, screenPos.y, size.x, size.y, WHITE);

    unsigned char rValue = 255 * pow(1.f - timeRemaining/maxTime, 4);  // Determines the R values such that it increases over time
    DrawRectangle(screenPos.x + 5.f, size.y - 25.f, static_cast<int>(timeRemaining/maxTime * (size.x - 10.f)), 20, {rValue, 0, 0, 255});

    DrawDish();
}