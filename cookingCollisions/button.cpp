#include "button.h"

// Constructor for the button class
Button::Button(std::string name, Vector2 pos, Vector2 dimensions, float thickness) :
label(name),
screenPos(pos),
size(dimensions),
outlineThickness(thickness)
{
	fontSize = size.y - 5.f;
}

// Check if the mouse cursor is currently hovering the button
bool Button::CheckHovering()
{
	Vector2 mousePos = GetMousePosition();
	if (mousePos.x >= screenPos.x && mousePos.x <= screenPos.x + size.x &&
		mousePos.y >= screenPos.y && mousePos.y <= screenPos.y + size.y) {
		return true;
	}
	return false;
}

// Handles logic and drawing the button
void Button::Tick()
{
	Color color = WHITE; // By default sets the colour to white
	isPressed = false;
	if (CheckHovering()) {
		color = GRAY;  // If the button is hovered, update its colour
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) isPressed = true; // If the button is pressed, set a flag
	}

	// The position and dimensions of the main body of the button
	Rectangle innerRect = { screenPos.x, screenPos.y, size.x, size.y };

	// The position and dimensions of the rectangle that makes up the outline
	Rectangle outerRect = { screenPos.x - outlineThickness,
		screenPos.y - outlineThickness,
		size.x + outlineThickness * 2.f,
		size.y + outlineThickness * 2.f };

	DrawRectangleRounded(outerRect, 0.5, 5, BLACK);
	DrawRectangleRounded(innerRect, 0.5, 5, color);

	// Draws text centred on the button
	DrawText(label.c_str(),
		screenPos.x + (size.x - MeasureText(label.c_str(),
		fontSize))/2.f, screenPos.y + (size.y - fontSize)/2.f,
		fontSize, BLACK);
}