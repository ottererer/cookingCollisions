#include "recipeBook.h"

RecipeBook::RecipeBook(std::vector<Texture2D> textures) :
	recipeTextures(textures),
	bookLength(textures.size() - 1)
{

}

// Handles logic and drawing
void RecipeBook::Tick()
{
	if (IsKeyPressed(KEY_R)) isShowing = !isShowing;

    // Only performs logic if shown
	if (isShowing) {
        if (IsKeyPressed(KEY_LEFT)) {
            bookPage--;
            if (bookPage < 0) bookPage = bookLength;
        }
        else if (IsKeyPressed(KEY_RIGHT)) {
            bookPage++;
            if (bookPage > bookLength) bookPage = 0;
        }

        // The selects the section of the texture to draw (the whole texture)
        Rectangle source = { 0, 0, (float)recipeTextures[bookPage].width, (float)recipeTextures[bookPage].height };

        // The rect that determines the position and dimensions of the texture
        Rectangle dest = {
            GetPos().x,
            GetPos().y,
            GetSize().x,
            GetSize().y
        };

        // What point on the texture is treated as the origin (the centre)
        Vector2 origin = {
            (GetSize().x) / 2.f,
            (GetSize().y) / 2.f
        };
        DrawTexturePro(recipeTextures[bookPage], source, dest, origin, 0.f, WHITE);
	}
}