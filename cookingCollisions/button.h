#pragma once

#include "config.h"
#include "raylib.h"
#include <string>
#include <iostream>

class Button
{
public:
	Button(const std::string& name, const Vector2& pos, const Vector2& dimensions, float thickness = 0);
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