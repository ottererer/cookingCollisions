#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <random>
#include "raylib.h"
#include <unordered_map>
#include "config.h"

class Order
{
public:
	Order();
	void Tick(float deltaTime);
	void DrawDish();

	static void SetupOrders(std::unordered_map<std::string, Texture2D> textures);
	static int RandomNumber(int min, int max);
	static void AddType(std::vector<std::string> types) { for (auto type : types) availableTypes.push_back(type); };

	// Getters
	float GetTime() { return timeRemaining; }
	std::string GetDish() { return dish; }
	std::unordered_map<std::string, Texture2D> GetTextures() { return dishTextures; }
	Vector2 GetPos() { return screenPos; }
	Vector2 GetSize() { return size; }

	// Setters
	void SetOrderNum(int num) { orderNum = num; }

private:
	std::string dish;
	int fontSize = 20;
	float maxTime = 60.f;
	float timeRemaining = maxTime;
	int orderNum = 0;

	Vector2 screenPos{0.f, 0.f};
	Vector2 size{80.f * static_cast<float>(winWidth) / 800.f, 90.f * static_cast<float>(winWidth) / 800.f};

	static std::vector<std::string> availableTypes;
	static std::unordered_map<std::string, Texture2D> dishTextures;
};