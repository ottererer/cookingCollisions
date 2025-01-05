#include <raylib.h>
#include <sstream>
#include <map>
#include <unordered_map>
#include <random>
#include <math.h>
#include <cmath>
#include "config.h"
#include "player.h"
#include "counter.h"
#include "items.h"
#include "order.h"
#include "button.h"
#include "recipeBook.h"

// Handles events related to dishes, such as removing, combining, and serving items
int handleEvents(std::vector<BaseItem*>& items, std::vector<std::string>& orderedDishes,
    std::vector<Order*>& orders,float& gameTimer, int& score)
{
    int newItem = -1;  // Initialises the new item variable to null

    // Iterates through every currently existing item
    for (auto it = items.begin(); it != items.end();) {
        BaseItem* currentItem = *it;  // Gets the current item

        // Ensures the current item is a valid pointer
        if (currentItem != nullptr) {
            std::vector<BaseItem*> placedItems = currentItem->GetTemps();  // Gets the items to combine

            // If the current item is flagged to remove
            if (currentItem->GetRemove()) {
                it = items.erase(it);  // Remove the item from the vector
                continue;  // Moves on to the next item
            }

            // If the current item is flagged to combine items
            else if (currentItem->GetCombine()) {
                newItem = std::distance(items.begin(), it);  // Gets the index of the current item
            }

            // If the current item is flagged to serve
            else if (currentItem->GetServing()) {
                std::string dishToServe = currentItem->GetPlaced()->GetType();
                bool found = false;
                int index = 0;

                // A linear search through all ordered dishes
                while (!found && index < orderedDishes.size()) {
                    // If the current ordered dish is the same as the current item
                    if (orderedDishes[index] == dishToServe) {
                        orders.erase(orders.begin() + index);  // Removes the order from the list
                        found = true;

                        // Adds time and score
                        gameTimer += 105.f;
                        score += 10;
                        if (gameTimer > 180.f) gameTimer = 180.f; // Ensures the time does not go above 180 seconds
                    }

                    // Otherwise increments the index
                    else index++;
                }
                if (!found) gameTimer -= 20.f; // Time penalty
            }
            ++it;
        }
    }
    return newItem;
}

// Handles drawing and some logic during the main sequence
void drawMain(float deltaTime, int& score, float globalTime, float& gameTimer, float& timeSinceOrder, float& timeToNext,
    Player& player, Counter& counter, std::vector<BaseItem*>& items, std::vector<Order*>& orders, RecipeBook& recipeBook)
{
    BeginDrawing();
    ClearBackground(BROWN);

    counter.Tick(deltaTime);

    player.Tick(deltaTime);

    std::vector<CounterUnit*> unitPointers = counter.GetUnitsPointers(); // Gets a vector of pointers to all counter units

    player.ResolveCollisions(unitPointers, deltaTime);
    player.HandleSelect(unitPointers);
    player.HandleItems(unitPointers);

    // Creates a vector of the names of all ordered dishes
    std::vector<std::string> orderedDishes;
    for (auto order : orders) {
        orderedDishes.push_back(order->GetDish());
    }
    BaseItem::SetOrderedDishes(orderedDishes);

    std::vector<BaseItem*> firstItems;
    std::vector<BaseItem*> otherItems;

    // Defines the order of drawing items
    for (BaseItem* item : items) {
        if (dynamic_cast<Plate*>(item)) {
            firstItems.push_back(item);
        }
        else {
            otherItems.push_back(item);
        }
    }
    
    // Draws all items
    for (BaseItem* item : firstItems) {
        item->Tick(deltaTime);
    }
    for (BaseItem* item : otherItems) {
        item->Tick(deltaTime);
    }

    int newItem = handleEvents(items, orderedDishes, orders, gameTimer, score);
    
    // If there is a new item, add the new item to the list
    if (newItem != -1) {
        Plate* plate = dynamic_cast<Plate*>(items[newItem]);
        if (plate) {
            items.push_back(plate->GetPlaced());
        }
    }

    player.Draw();

    for (BaseItem* item : items) {
        item->ResetFlags();
    }

    // Draws the time to the screen
    char timerText[50];
    sprintf_s(timerText, "Time: %.1f", gameTimer);
    DrawText(timerText, 20, 20, 40, BLACK);

    // Draws the score to the screen
    std::string scoreText = "Score: " + std::to_string(score);
    DrawText(scoreText.c_str(), 20, 70, 40, BLACK);

    // Handles general logic for orders
    for (auto it = orders.begin(); it != orders.end();) {
        Order* currentItem = *it;

        // If the timer for an order runs out, impose time penalty
        if (currentItem->GetTime() <= 0.f) {
            it = orders.erase(it);
            gameTimer -= 20.f;
        }
        else ++it;
    }

    // Resets the time, and determines the time until the next order appears
    if (timeSinceOrder >= timeToNext) {
        timeSinceOrder = 0;
        timeToNext = 20.f / (1 + 4 * std::log(1 + 0.003 * globalTime));
        orders.push_back(new Order);
    }

    // Sets the order of and draws all orders
    if (orders.size() != 0) {
        for (int i = 0; i < orders.size(); i++) {
            orders[i]->SetOrderNum(i);
            orders[i]->Tick(deltaTime);
        }
    }

    recipeBook.Tick();

    EndDrawing();
}

// Handles drawing and some logic in the menu
void drawMenu(std::string& gameState, std::vector<Button*>& buttons)
{
    BeginDrawing();
    ClearBackground(WHITE);

    // Draws all buttons
    for (Button* button : buttons) {
        button->Tick();
        if (button->GetPressed()) {
            gameState = "main";
        }
    }

    if (IsKeyPressed(KEY_ENTER)) gameState = "main";

    EndDrawing();
}

// Handles drawing and some logic for the end screen
void drawEnd()
{
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText("You Lose!", static_cast<int>(400.f - MeasureText("You Lose!", 100) / 2.f), 300, 100, WHITE);

    EndDrawing();
}

// The main sequence of the program
int main() {
    InitWindow(winWidth, winHeight, "Cooking Collisions");
    SetTargetFPS(FPS);

    std::string gameState = "menu";  // Initialises the game state to the menu

    std::vector<Button*> buttons;
    buttons.push_back(new Button("Start", { 150.f, 100.f }, { 500.f, 100.f }, 10.f));

    // Initialises values relating to the game loop
    int score = 0;
    float globalTime = 0.f;
    float gameTimer = 120.f;
    float timeSinceOrder = 0.f;
    float timeToNext = 20.f;

    Player player(Vector2{ static_cast<float>(winWidth) / 2.f, static_cast<float>(winHeight) / 2.f });

    Order::AddType({ "caramel energy cube", "spicy frost bomb", "protein salad",
                    "liquid flame soup", "frosted energy treat"});

    std::vector<Order*> orders;
    orders.push_back(new Order);  // Initially adds an order to the queue

    BaseItem::SetupClass();


    // Sets up all textures to be used
    // =============================================================================================
    std::unordered_map<std::string, std::map<std::string, Texture2D>> textures;
    std::vector<Texture2D> recipeTextures;

    std::map<std::string, Texture2D> nullTextures;
    nullTextures["default"] = BaseItem::LoadTexture("assets/NULL.png");
    textures["null"] = nullTextures;

    std::map<std::string, Texture2D> choppingBoardTextures;
    choppingBoardTextures["default"] = BaseItem::LoadTexture("assets/ChoppingBoard.png");
    textures["chopping board"] = choppingBoardTextures;

    std::map<std::string, Texture2D> fryingPanTextures;
    fryingPanTextures["default"] = BaseItem::LoadTexture("assets/FryingPan.png");
    textures["frying pan"] = fryingPanTextures;

    std::map<std::string, Texture2D> plateTextures;
    plateTextures["default"] = BaseItem::LoadTexture("assets/Plate.png");
    textures["plate"] = plateTextures;

    std::map<std::string, Texture2D> sweetCrystalTextures;
    sweetCrystalTextures["default"] = BaseItem::LoadTexture("assets/SweetCrystal.png");
    sweetCrystalTextures["cooked"] = BaseItem::LoadTexture("assets/CaramelEssence.png");
    sweetCrystalTextures["chopped"] = BaseItem::LoadTexture("assets/SugarShards.png");
    textures["sweet crystal"] = sweetCrystalTextures;

    std::map<std::string, Texture2D> spiceParticleTextures;
    spiceParticleTextures["default"] = BaseItem::LoadTexture("assets/SpiceParticle.png");
    textures["spice particle"] = spiceParticleTextures;

    std::map<std::string, Texture2D> energyParticleTextures;
    energyParticleTextures["default"] = BaseItem::LoadTexture("assets/EnergyParticle.png");
    textures["energy particle"] = energyParticleTextures;

    std::map<std::string, Texture2D> liquidEssenceTextures;
    liquidEssenceTextures["default"] = BaseItem::LoadTexture("assets/LiquidEssence.png");
    textures["liquid essence"] = liquidEssenceTextures;

    std::map<std::string, Texture2D> proteinOrbTextures;
    proteinOrbTextures["default"] = BaseItem::LoadTexture("assets/ProteinOrb.png");
    textures["protein orb"] = proteinOrbTextures;

    std::map<std::string, Texture2D> vegetableCoreTextures;
    vegetableCoreTextures["default"] = BaseItem::LoadTexture("assets/VegetableCore.png");
    textures["vegetable core"] = vegetableCoreTextures;

    std::map<std::string, Texture2D> aromaSphereTextures;
    aromaSphereTextures["default"] = BaseItem::LoadTexture("assets/AromaSphere.png");
    textures["aroma sphere"] = aromaSphereTextures;

    std::map<std::string, Texture2D> coolingShardTextures;
    coolingShardTextures["default"] = BaseItem::LoadTexture("assets/CoolingShard.png");
    textures["cooling shard"] = coolingShardTextures;

    recipeTextures.push_back(BaseItem::LoadTexture("assets/RecipeBookCaramelEnergyCube.png"));
    recipeTextures.push_back(BaseItem::LoadTexture("assets/RecipeBookSpicyFrostBomb.png"));
    recipeTextures.push_back(BaseItem::LoadTexture("assets/RecipeBookProteinSalad.png"));
    recipeTextures.push_back(BaseItem::LoadTexture("assets/RecipeBookLiquidFlameSoup.png"));
    recipeTextures.push_back(BaseItem::LoadTexture("assets/RecipeBookFrostedEnergyTreat.png"));
    // =============================================================================================

    RecipeBook recipeBook(recipeTextures);

    // Adds individual items
    std::vector<BaseItem*> items;
    items.push_back(new Tool(choppingBoardTextures, "chopping board"));
    items.push_back(new Tool(fryingPanTextures, "frying pan", 120.f, 120.f));

    // Sets the types of the counters
    // Delivery = dishes are submitted here
    // Bin = ingredients can be thrown out here
    // Source = infinite supply of the item specified
    Counter counter;
    counter.CreateCounter();
    counter.GetUnits()[0].SetType("delivery");
    counter.GetUnits()[1].SetType("delivery");
    counter.GetUnits()[2].SetType("bin");
    counter.GetUnits()[18].SetType("source", "sweet crystal");
    counter.GetUnits()[17].SetType("source", "energy particle");
    counter.GetUnits()[16].SetType("source", "spice particle");
    counter.GetUnits()[15].SetType("source", "plate");
    counter.GetUnits()[14].SetType("source", "liquid essence");
    counter.GetUnits()[13].SetType("source", "protein orb");
    counter.GetUnits()[12].SetType("source", "vegetable core");
    counter.GetUnits()[11].SetType("source", "aroma sphere");
    counter.GetUnits()[10].SetType("source", "cooling shard");

    int index = 0;
    for (int i = 0; i < items.size(); i++) {
        while (counter.GetUnits()[index].GetType() != "default") {
            index++;
        }
        counter.GetUnits()[index].AddItem(items[i]);
        index++;
    }

    bool running = true;

    // The main game loop
    while (running)
    {
        running = !WindowShouldClose();

        float deltaTime = GetFrameTime();
        float fps = GetFPS();

        // Displays the framerate as the window title
        std::stringstream ss;
        ss << "FPS: " << fps;
        std::string titleText = ss.str();
        SetWindowTitle(titleText.c_str());
        
        // Logic if the game is on the menu
        if (gameState == "menu") {
            drawMenu(gameState, buttons);
        }

        // Logic if the game is in the main sequence
        else if (gameState == "main") {
            globalTime += deltaTime;
            gameTimer -= deltaTime;
            timeSinceOrder += deltaTime;

            // Logic to ensure source units are always filled
            auto& units = counter.GetUnits();
            for (int i = 0; i < units.size(); i++) {
                // If the unit is a source, and there is nothing currently placed
                if (units[i].GetType() == "source" && units[i].GetPlaced() == nullptr) {
                    if (units[i].GetSourceType() == "plate") {
                        items.push_back(new Plate(textures[units[i].GetSourceType()]));  // Adds an item of type Plate
                        units[i].AddItem(items[items.size() - 1]);  // Places the plate on the unit
                    }
                    else {
                        items.push_back(new Ingredient(textures[units[i].GetSourceType()], units[i].GetSourceType()));  // Adds the item based on the type of the source
                        units[i].AddItem(items[items.size() - 1]);  // Places the item on the unit
                    }
                }
            }

            drawMain(deltaTime, score, globalTime, gameTimer, timeSinceOrder, timeToNext, player,
                    counter, items, orders, recipeBook);

            // If time runs out, end the game
            if (gameTimer <= 0.f) {
                gameState = "end";
            }
        }

        // Logic if the game is at the end
        else if (gameState == "end") {
            drawEnd();
        }
    }

    // Unloads textures and closes the window
    BaseItem::UnloadStaticTextures();
    CloseWindow();

    return 0;
}