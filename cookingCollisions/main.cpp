#include "raylib.h"
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
std::unordered_map<std::string, int> handleEvents(std::vector<BaseItem*>& items, std::vector<std::string>& orderedDishes,
    std::vector<Order*>& orders, std::vector<CounterUnit*> units, float& gameTimer, int& score)
{
    int newItemPlate = -1;  // Initialises the new item variable to null
    int newItemUnit = -1;

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
                newItemPlate = std::distance(items.begin(), it);  // Gets the index of the current item
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
                        gameTimer += 20.f;
                        score += 10;
                        if (gameTimer > 150.f) gameTimer = 150.f; // Ensures the time does not go above 180 seconds
                    }

                    // Otherwise increments the index
                    else index++;
                }
                if (!found) gameTimer -= 20.f; // Time penalty
            }
            ++it;
        }
    }


    for (auto it = units.begin(); it != units.end();) {
        CounterUnit* currentUnit = *it;

        if (currentUnit != nullptr) {
            if (currentUnit->GetCombine()) {
                newItemUnit = std::distance(units.begin(), it);
            }
            it++;
        }
    }

    return { {"plate", newItemPlate}, {"unit", newItemUnit} };
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

    std::unordered_map<std::string, int> newItems = handleEvents(items, orderedDishes, orders,
                                                                counter.GetUnitsPointers(), gameTimer, score);
    
    // If there is a new item, add the new item to the list
    if (newItems["plate"] != -1) {
        Plate* plate = dynamic_cast<Plate*>(items[newItems["plate"]]);
        if (plate) {
            items.push_back(plate->GetPlaced());
        }
    }

    if (newItems["unit"] != -1) {
        items.push_back(counter.GetUnitsPointers()[newItems["unit"]]->GetPlaced());
    }

    player.Draw();

    for (BaseItem* item : items) {
        item->ResetFlags();
    }

    for (CounterUnit* unit : counter.GetUnitsPointers()) {
        unit->ResetFlags();
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
        if (orders.size() < 5) {
            timeSinceOrder = 0;
            timeToNext = 20.f / (1 + 4 * std::log(1 + 0.003 * globalTime));
            orders.push_back(new Order);
        }
    }

    // Sets the order of and draws all orders
    if (orders.size() != 0) {
        for (int i = 0; i < orders.size(); i++) {
            orders[i]->SetOrderNum(i);
            orders[i]->Tick(deltaTime);
        }
    }

    DrawText("Movement - W/A/S/D", 20, 650, 20, BLACK);
    DrawText("Pickup/place item - E", 20, 670, 20, BLACK);
    DrawText("Pickup/place item from plate - SHIFT + E", 20, 690, 20, BLACK);
    DrawText("Chop item - F", 20, 710, 20, BLACK);
    DrawText("Open recipe book - R", 20, 730, 20, BLACK);
    DrawText("Cycle page (recipe book) - LEFT / RIGHT", 20, 750, 20, BLACK);
    DrawText("Mute/unmute music - M", 20, 770, 20, BLACK);

    recipeBook.Tick();

    EndDrawing();
}

// Handles drawing and some logic in the menu
void drawMenu(std::string& gameState, std::vector<Button*>& buttons, int highScore)
{
    BeginDrawing();
    ClearBackground(BEIGE);

    DrawText("Recipe For", winWidth / 2.f - MeasureText("Recipe For", 100) / 2, 100.f, 100, BLACK);
    DrawText("Disaster", winWidth / 2.f - MeasureText("Disaster", 100) / 2, 220.f, 100, BLACK);

    DrawText("Start game - ENTER", 20, 750, 20, BLACK);
    DrawText("Mute/unmute music - M", 20, 770, 20, BLACK);
    
    std::string highScoreText = "Session best: " + std::to_string(highScore);
    DrawText(highScoreText.c_str(), winWidth / 2.f - MeasureText(highScoreText.c_str(), 60) / 2, 600, 60, BLACK);

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
void drawEnd(std::string& gameState, std::vector<Button*>& buttons, int score, float timeLived)
{
    BeginDrawing();
    ClearBackground(BEIGE);

    DrawText("You Lose!", static_cast<int>(400.f - MeasureText("You Lose!", 100) / 2.f), 100, 100, WHITE);

    DrawText("Restart - R", 20, 730, 20, BLACK);
    DrawText("Main menu - ENTER", 20, 750, 20, BLACK);
    DrawText("Mute/unmute music - M", 20, 770, 20, BLACK);

    std::string scoreText = "Score: " + std::to_string(score);
    DrawText(scoreText.c_str(), winWidth / 2.f - MeasureText(scoreText.c_str(), 60) / 2, 530, 60, BLACK);

    char timerText[50];
    sprintf_s(timerText, "Time: %.1f", timeLived);
    DrawText(timerText, winWidth / 2.f - MeasureText(timerText, 40) / 2, 600, 40, BLACK);

    for (Button* button : buttons) {
        button->Tick();
        if (button->GetPressed()) {
            if (button->GetLabel() == "Restart") {
                gameState = "main";
            }
            else if (button->GetLabel() == "Main Menu") {
                gameState = "menu";
            }
        }
    }

    if (IsKeyPressed(KEY_ENTER)) gameState = "menu";
    else if (IsKeyPressed(KEY_R)) gameState = "main";

    EndDrawing();
}

// The main sequence of the program
int main() {
    InitWindow(winWidth, winHeight, "Cooking Collisions");
    InitAudioDevice();
    SetTargetFPS(FPS);

    Music music = LoadMusicStream("assets/LaMusique.mp3");
    SetMasterVolume(0.4f);
    bool muted = false;

    PlayMusicStream(music);

    std::string gameState = "menu";  // Initialises the game state to the menu

    std::vector<Button*> startButtons;
    startButtons.push_back(new Button("Play", { 150.f, 400.f }, { 500.f, 100.f }, 10.f));

    std::vector<Button*> endButtons;
    endButtons.push_back(new Button("Restart", { 150.f, 250.f }, { 500.f, 100.f }, 10.f));
    endButtons.push_back(new Button("Main Menu", { 150.f, 400 }, { 500.f, 100.f }, 10.f));

    int highScore = 0;

    // Initialises values relating to the game loop
    int score = 0;
    float globalTime = 0.f;
    float gameTimer = 120.f;
    float timeSinceOrder = 0.f;
    float timeToNext = 20.f;
    
    int roundScore = 0;
    float roundTime = 0.f;

    Player player({ static_cast<float>(winWidth) / 2.f, static_cast<float>(winHeight) / 2.f });

    Order::AddType({ "caramel energy cube", "spicy frost bomb", "protein salad",
                    "liquid flame soup", "frosted energy treat"});

    std::vector<Order*> orders;
    orders.push_back(new Order);  // Initially adds an order to the queue

    BaseItem::SetupClass();


    // Sets up all textures to be used
    // =============================================================================================
    std::unordered_map<std::string, std::map<std::string, Texture2D>> textures;
    std::vector<Texture2D> recipeTextures;
    std::unordered_map<std::string, Texture2D> orderTextures;

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

    orderTextures["caramel energy cube"] = BaseItem::LoadTexture("assets/CaramelEnergyCube.png");
    orderTextures["spicy frost bomb"] = BaseItem::LoadTexture("assets/SpicyFrostBomb.png");
    orderTextures["protein salad"] = BaseItem::LoadTexture("assets/ProteinSalad.png");
    orderTextures["liquid flame soup"] = BaseItem::LoadTexture("assets/LiquidFlameSoup.png");
    orderTextures["frosted energy treat"] = BaseItem::LoadTexture("assets/FrostedEnergyTreat.png");
    // =============================================================================================

    RecipeBook recipeBook(recipeTextures);
    Order::SetupOrders(orderTextures);

    // Adds individual items
    std::vector<BaseItem*> items;
    items.push_back(new Tool(choppingBoardTextures, "chopping board"));
    items.push_back(new Tool(choppingBoardTextures, "chopping board"));
    items.push_back(new Tool(fryingPanTextures, "frying pan", 120.f, 120.f));
    items.push_back(new Tool(fryingPanTextures, "frying pan", 120.f, 120.f));

    // Sets the types of the counters
    // Delivery = dishes are submitted here
    // Bin = ingredients can be thrown out here
    // Source = infinite supply of the item specified
    Counter counter;
    counter.CreateCounter();
    counter.GetUnits()[1].SetType("delivery");
    counter.GetUnits()[2].SetType("delivery");
    counter.GetUnits()[16].SetType("bin");
    counter.GetUnits()[17].SetType("bin");
    counter.GetUnits()[0].SetType("source", "plate");
    counter.GetUnits()[3].SetType("source", "plate");
    counter.GetUnits()[6].SetType("source", "sweet crystal");
    counter.GetUnits()[7].SetType("source", "energy particle");
    counter.GetUnits()[12].SetType("source", "spice particle");
    counter.GetUnits()[13].SetType("source", "liquid essence");
    counter.GetUnits()[20].SetType("source", "protein orb");
    counter.GetUnits()[21].SetType("source", "vegetable core");
    counter.GetUnits()[26].SetType("source", "aroma sphere");
    counter.GetUnits()[27].SetType("source", "cooling shard");

    counter.GetUnits()[4].AddItem(items[0]);
    counter.GetUnits()[5].AddItem(items[1]);
    counter.GetUnits()[8].AddItem(items[2]);
    counter.GetUnits()[9].AddItem(items[3]);

    bool running = true;

    // The main game loop
    while (running)
    {
        running = !WindowShouldClose();

        float deltaTime = GetFrameTime();
        float fps = GetFPS();
        UpdateMusicStream(music);

        if (IsKeyPressed(KEY_M))
        {
            muted = !muted;

            if (muted) SetMasterVolume(0.f);
            else SetMasterVolume(0.4f);
        }

        // Displays the framerate as the window title
        std::stringstream ss;
        ss << "FPS: " << fps;
        std::string titleText = ss.str();
        SetWindowTitle(titleText.c_str());
        
        // Logic if the game is on the menu
        if (gameState == "menu") {
            drawMenu(gameState, startButtons, highScore);
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

            // If time runs out, ends and resets the game
            if (gameTimer <= 0.f) {
                if (score > highScore) highScore = score;
                gameState = "end";
                roundScore = score;
                roundTime = globalTime;

                player.SetHolding(nullptr);
                player.SetPos({ static_cast<float>(winWidth) / 2.f, static_cast<float>(winHeight) / 2.f });
                player.SetAngle(0.f);

                for (auto& unit : counter.GetUnits()) {
                    unit.ClearPlaced();
                }

                items.clear();

                items.push_back(new Tool(choppingBoardTextures, "chopping board"));
                items.push_back(new Tool(choppingBoardTextures, "chopping board"));
                items.push_back(new Tool(fryingPanTextures, "frying pan", 120.f, 120.f));
                items.push_back(new Tool(fryingPanTextures, "frying pan", 120.f, 120.f));

                counter.GetUnits()[4].AddItem(items[0]);
                counter.GetUnits()[5].AddItem(items[1]);
                counter.GetUnits()[8].AddItem(items[2]);
                counter.GetUnits()[9].AddItem(items[3]);

                score = 0;
                globalTime = 0.f;
                gameTimer = 120.f;
                timeSinceOrder = 0.f;
                timeToNext = 20.f;

                orders.clear();
                orders.push_back(new Order);
            }
        }

        // Logic if the game is at the end
        else if (gameState == "end") {
            drawEnd(gameState, endButtons, roundScore, roundTime);
        }
    }

    // Unloads textures and closes the window
    BaseItem::UnloadStaticTextures();
    CloseWindow();

    return 0;
}