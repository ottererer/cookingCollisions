#pragma once

#include "config.h"
#include <raylib.h>
#include <string>
#include <iostream>

class Button
{
public:
	Button(std::string name, Vector2 pos, Vector2 dimensions, float thickness = 0);
	void Tick();

	bool CheckHovering();

	// Getters
	bool GetPressed() { return isPressed; }
	std::string GetLabel() { return label; }

private:
	std::string label;
	Vector2 screenPos{};
	Vector2 size{};
	float outlineThickness;
	int fontSize;

	bool isPressed = false;
};