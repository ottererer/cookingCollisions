#include "items.h"

// Defines static attributes
std::map<std::string, Texture2D> BaseItem::dishTextures;
RecipeGraph& BaseItem::recipes = RecipeGraph::GetInstance();
std::vector<std::string> BaseItem::orderedDishes;

// Static method to load and process textures
Texture2D BaseItem::LoadTexture(std::string path)
{
    Image plateImage = LoadImage(path.c_str());  // Loads the image from the given path
    ImageAlphaPremultiply(&plateImage);  // Multiplies all RGB values by the alpha channel to ensure smoother blending
    ImageMipmaps(&plateImage);  // Generates images pre-scales to different sizes for scaling
    Texture2D plateTexture = LoadTextureFromImage(plateImage);  // Loads the texture from the image so that it can be used by the GPU
    SetTextureFilter(plateTexture, TEXTURE_FILTER_TRILINEAR);  // Applies trilinear filtering for smooth interpolation for scaled textures
    UnloadImage(plateImage);  // Unloads the image as it is no longer in use

    return plateTexture;
}

// Static method
void BaseItem::SetupClass()
{
    // Stores all available ingredients and dishes as nodes in a DAG
    std::vector<std::string> items = { "tomato", "chopped tomato", "bread", "tomato sandwich", "patty",
        "cooked patty", "burnt patty"};
    for (const auto item : items) {
        recipes.AddNode(item);
    }

    // Adds edges between nodes with an input, which indicate what can be turned into what, and how
    recipes.AddEdge("tomato", "chopped tomato", "chopping board");
    recipes.AddEdge("chopped tomato", "tomato sandwich");
    recipes.AddEdge("bread", "tomato sandwich");
    recipes.AddEdge("patty", "cooked patty", "frying pan");
    recipes.AddEdge("cooked patty", "burnt patty", "frying pan");

    // Marks all objects that can be served
    std::vector<std::string> serveable = { "tomato sandwich" };
    for (int i = 0; i < serveable.size(); i++) {
        recipes.AddEdge(serveable[i], "serving", "serve");
    }

    // Loads static textures to be used for non-spawning dishes
    dishTextures["tomato sandwich"] = LoadTexture("assets/CaramelEnergyCube.png");
}

// Places a new item on a given item
void BaseItem::AddItem(BaseItem* item)
{
    // If there is nothing in the first slot, place it there
    if (itemsPlaced[0] == nullptr) {
        itemsPlaced[0] = item;
    }

    // Otherwise place it in the second slot
    else {
        itemsPlaced[1] = item;
    }
}

// Static method to unload all static textures loaded by the Items class
void BaseItem::UnloadStaticTextures()
{
    for (auto& texturePair : dishTextures) {
        UnloadTexture(texturePair.second);
    }
    dishTextures.clear();
}

// Unloads textures used by a specific instance of the class
void BaseItem::UnloadTextures()
{
    for (auto& texturePair : itemTextures) {
        UnloadTexture(texturePair.second);
    }
    itemTextures.clear();
}


void BaseItem::HandleCooking()
{
    if (GetType() == "chopping board") {
        if (recipes.ApplyInputToNode(GetPlaced()->GetType(), "chopping board") == "NULL") return;
        GetPlaced()->SetState("chopped");
        GetPlaced()->SetType(recipes.ApplyInputToNode(GetPlaced()->GetType(), "chopping board"));
        GetPlaced()->SetDimensions(45.f, 45.f);
    }
}

// Removes the current item and/or anything placed on it
void BaseItem::RemoveItems(bool removeSelf, bool removePlaced)
{
    if (removeSelf) {
        removeItem = true;  // Sets a flag to remove this item
    }

    if (removePlaced) {
        for (auto item : itemsPlaced) {
            if (item != nullptr)
                item->RemoveItems();  // Removes any item placed
        }
        ClearItems();
    }
}

// Handles logic for combining two items
void BaseItem::CombineItems()
{
    if (itemsPlaced[0] == nullptr || itemsPlaced[1] == nullptr) return;  // If either item is empty, exit

    // Get the name of both items
    std::string type1 = itemsPlaced[0]->GetType();
    std::string type2 = itemsPlaced[1]->GetType();

    std::string combinedType = recipes.FindCommonNode(type1, type2);  // Find the result of combining the two items

    // If FindCommonNode returns NULL, the items cannot be combined
    if (combinedType != "NULL") {
        combineItems = true;
        RemoveItems(false);
        ClearItems();
        BaseItem* newItem = CreateCombinedItem(combinedType);  // Creates a new object based on the result of combining the items
        newItem->SetDimensions(50.f, 50.f);
        if (newItem) AddItem(newItem);  // If the new item as created successfully, place it on the current object
    }
}

// Creates a new object of a given type
BaseItem* BaseItem::CreateCombinedItem(std::string& type)
{
    return new Ingredient({ { "default", dishTextures[type] } }, type);
}

// Handles drawing and main logic for items
void BaseItem::Tick(float deltaTime)
{
    CombineItems();  // Checks if there are items that can be combined

    // If the current item is a frying pan with something placed on it
    if (itemType == "frying pan" && itemsPlaced[0] != nullptr) {
        itemTimer += deltaTime;

        if (GetPlaced() != nullptr) {
            // If the placed item can be fried, and 5 seconds have passed
            if (itemTimer >= 5.f && GetPlaced()->CanFry(GetPlaced()->GetType())) {
                // If the placed item has not been processed yet, cook it
                if (GetPlaced()->GetState() == "default") {
                    GetPlaced()->SetState("cooked");
                    GetPlaced()->SetType(recipes.ApplyInputToNode(GetPlaced()->GetType(), "frying pan"));
                    ResetTimer();
                }

                // Otherwise extend the timer, and burn it
                else if (itemTimer >= 8.f) {
                    GetPlaced()->SetState("burnt");
                    GetPlaced()->SetType(recipes.ApplyInputToNode(GetPlaced()->GetType(), "frying pan"));;
                    ResetTimer();
                }
            }
            // If the item cannot be fried, reset the timer
            else if (!GetPlaced()->CanFry(GetPlaced()->GetType())) ResetTimer();
        }
    }

    // The selects the section of the texture to draw (the whole texture)
    Rectangle source = { 0, 0, (float)GetTexture()[itemState].width, (float)GetTexture()[itemState].height};

    // The rect that determines the position and dimensions of the texture
    Rectangle dest = {
        GetPos().x,
        GetPos().y,
        itemWidth,
        itemHeight
    };

    // What point on the texture is treated as the origin (the centre)
    Vector2 origin = {
        (itemWidth) / 2.f,
        (itemHeight) / 2.f
    };

    DrawTexturePro(GetTexture()[itemState], source, dest, origin, itemAngle, WHITE);

    // Updates the position of any placed items to match itself
    if (itemsPlaced[0] != nullptr) {
        itemsPlaced[0]->SetPos(screenPos);
    }
}

// Constructor for the Plate subclass
Plate::Plate(std::map<std::string, Texture2D> textures, float width, float height) :
    BaseItem(textures, "plate")
{
    if (width != 0.f) SetWidth(width);
    if (height != 0.f) SetHeight(height);
}

// Checks if an item can be placed on a given plate
bool Plate::CanPlace(std::string type) const
{
    // If the item being placed is not a plate and the plate is empty,
    // or the item being placed is compatible with what is already placed
    if (type != "plate") {
        if (itemsPlaced[0] == nullptr) return 1;
        else {
            if (recipes.FindCommonNode(GetPlaced()->GetType(), type) != "NULL") return 1;
        }
    }

    return 0;
}

// Constructor for the Ingredient subclass
Ingredient::Ingredient(std::map<std::string, Texture2D> textures, std::string type, float width, float height) :
    BaseItem(textures, type)
{
    if (width != 0.f) SetWidth(width);
    if (height != 0.f) SetHeight(height);
}

// Checks if an item can be chopped
bool Ingredient::CanChop(std::string type) const
{
    // If applying the chopping board input to the item yields a valid result, it can be chopped
    if (recipes.ApplyInputToNode(type, "chopping board") != "NULL") return true;
    return false;
}

// Checks if an item can be fried
bool Ingredient::CanFry(std::string type) const
{
    // If applying the frying pan input to the item yields a valid result, it can be fried
    if (recipes.ApplyInputToNode(type, "frying pan") != "NULL") return true;
    return false;
}

// Checks if an item can be boiled
bool Ingredient::CanBoil(std::string type) const
{
    // If applying the saucepan input to the item yields a valid result, it can be boiled
    if (recipes.ApplyInputToNode(type, "saucepan") != "NULL") return true;
    return false;
}

// Checks if an item can be served
bool Ingredient::CanServe() const
{
    // If applying the serve input to the item yields a valid result, it can be served
    if (recipes.ApplyInputToNode(GetType(), "serve") != "NULL") return true;
    return false;
}

// Checks if an item can be served, and it fulfills an order
bool Ingredient::CanServeSuccessfully(std::string type) const
{
    // If the current dish is present in the list of ordered dishes
    if (std::count(orderedDishes.begin(), orderedDishes.end(), type)) {
        return true;
    }
    return false;
}

// Constructor for the Tool subclass
Tool::Tool(std::map<std::string, Texture2D> textures, std::string type, float width, float height) :
    BaseItem(textures, type) 
{
    if (width != 0.f) SetWidth(width);
    if (height != 0.f) SetHeight(height);

    if (type == "chopping board") {
        itemFilter = {"tomato"};
    }
    if (type == "frying pan") {
        itemFilter = { "patty", "cooked patty" };
    }
}

bool Tool::CanPlace(std::string type) const
{   
    bool itemAllowed = false;
    for (auto item : itemFilter) {
        if (item == type) itemAllowed = true;
    }
    if (itemAllowed && itemsPlaced[0] == nullptr) return 1;

    return 0;
}
