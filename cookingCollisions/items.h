#pragma once

#include "raylib.h"
#include "raymath.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>
#include "config.h"
#include "recipeGraph.h"

class BaseItem
{
public:
	BaseItem(const std::map<std::string, Texture2D>& textures, std::string type): itemTextures(textures), itemType(type) {}
	virtual ~BaseItem() = default;
	static void SetupClass();

	void Tick(float deltaTime);
	void AddItem(BaseItem* item);
	void ClearItems() { itemsPlaced = { nullptr, nullptr }; }
	void ClearTemps() { tempItems = { nullptr, nullptr }; }
	void ResetTimer() { itemTimer = 0.f; }
	virtual bool CanPickup() const = 0;
	virtual bool CanPlace(const std::string& type) const = 0;

	virtual bool CanChop(const std::string& type) const { return 0; }
	virtual bool CanFry(const std::string& type) const { return 0; }
	virtual bool CanBoil(const std::string& type) const { return 0; }
	virtual bool CanServe() const { return 0; }
	virtual bool CanServeSuccessfully(const std::string& type) const { return 0; }

	void HandleCooking();
	void RemoveItems(bool removeSelf = true, bool removePlaced = true);
	void CombineItems();
	BaseItem* CreateCombinedItem(const std::string& type);
	void ResetFlags() { removeItem = false; combineItems = false; serveItem = false; }
	

	static Texture2D LoadTexture(std::string path);
	static void UnloadStaticTextures();
	void UnloadTextures();


	// Getters
	Vector2 GetPos() const { return screenPos;  }
	std::string GetType() const { return itemType; }
	std::string GetState() const { return itemState; }
	std::map<std::string, Texture2D> GetTexture() const { return itemTextures; }
	BaseItem* GetPlaced() const { return itemsPlaced[0];  }
	std::vector<BaseItem*> GetItems() const { return itemsPlaced; }
	std::vector<BaseItem*> GetTemps() const { return tempItems; }
	std::vector<float> GetDimensions() const { return std::vector<float>{itemWidth, itemHeight}; }
	bool GetCombine() { return combineItems; }
	bool GetRemove() { return removeItem; }
	bool GetServing() { return serveItem; }
	float GetTime() { return itemTimer; }
	static std::unordered_map<std::string, std::map<std::string, Texture2D>> GetDishTextures() { return dishTextures; }

	// Setters
	void SetState(const std::string& state) { itemState = state; }
	void SetType(const std::string& type) { itemType = type; }

	void SetPos(const Vector2& pos) { screenPos = pos; }
	void MovePos(const Vector2& pos) { screenPos = Vector2Add(screenPos, pos); }
	void SetRot(float angle) { itemAngle = angle; }
	void SetDimensions(float width, float height) { itemWidth = width * static_cast<float>(winWidth) / 800.f; itemHeight = height * static_cast<float>(winWidth) / 800.f; }
	void SetWidth(float width) { itemWidth = width * static_cast<float>(winWidth) / 800.f; }
	void SetHeight(float height) { itemHeight = height * static_cast<float>(winWidth) / 800.f; }
	void SetServing(bool serving) { serveItem = serving; }

	static void SetOrderedDishes(const std::vector<std::string>& dishes) { orderedDishes = dishes; }

protected:
	std::vector<std::string> itemFilter;
	std::vector<BaseItem*> itemsPlaced = { nullptr, nullptr };
	std::vector<BaseItem*> tempItems = { nullptr, nullptr };

	static std::unordered_map<std::string, std::map<std::string, Texture2D>> dishTextures;
	static RecipeGraph& recipes;
	static std::vector<std::string> orderedDishes;

private:
	bool combineItems = false;
	bool removeItem = false;
	bool serveItem = false;
	std::map<std::string, Texture2D> itemTextures;
	std::string itemType;
	std::string itemState = "default";
	float itemTimer = 0.f;

	Vector2 screenPos{};
	float itemWidth = 75.f * static_cast<float>(winWidth) / 800.f;
	float itemHeight = 75.f * static_cast<float>(winWidth) / 800.f;
	float itemAngle = 0.f;
};

class Plate : public BaseItem
{
public:
	Plate(const std::map<std::string, Texture2D>& textures, float width = 0.f, float height = 0.f);

	bool CanPickup() const override { return 1; }
	bool CanPlace(const std::string& type) const override;
};

class Ingredient : public BaseItem
{
public:
	Ingredient(std::map<std::string, Texture2D> textures, std::string type, float width = 0.f, float height = 0.f);

	bool CanPickup() const override { return 1; }
	bool CanPlace(const std::string& type) const override { return 0; }
	bool CanChop(const std::string& type) const override;
	bool CanFry(const std::string& type) const override;
	bool CanBoil(const std::string& type) const override;
	bool CanServe() const override;
	bool CanServeSuccessfully(const std::string& type) const override;
};

class Tool : public BaseItem
{
public:
	Tool(const std::map<std::string, Texture2D>& textures, const std::string& type, float width = 0.f, float height = 0.f);

	bool CanPickup() const override { return 0; }
	bool CanPlace(const std::string& type) const override;
};
