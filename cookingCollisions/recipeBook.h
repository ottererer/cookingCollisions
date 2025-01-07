#pragma once

#include "config.h"
#include <vector>
#include <string>

class RecipeBook
{
public:
	RecipeBook(const std::vector<Texture2D>& textures);
	void Tick();

	// Getters
	Vector2 GetPos() { return screenPos; }
	Vector2 GetSize() { return size; }

private:
	std::vector<Texture2D> recipeTextures;
	int bookPage = 0;
	int bookLength;
	bool isShowing = false;
	Vector2 screenPos = { winWidth / 2.f, winHeight / 2.f };
	Vector2 size = { winWidth, winHeight };
};