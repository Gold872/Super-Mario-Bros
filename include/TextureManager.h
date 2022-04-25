#pragma once

#include "ECS/ECS.h"
#include "Math.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <memory>

enum class BackgroundColor {
	NONE,
	BLACK,
	BLUE
};

class TextureManager {
  public:
   static int Init();
   static int Quit();

   static SDL_Texture* LoadTexture(const char* path);
   static std::shared_ptr<SDL_Texture> LoadSharedTexture(const char* path);
   static std::shared_ptr<TTF_Font> LoadSharedFont(const char* path, int fontSize);
   static void Draw(SDL_Texture* texture, SDL_Rect destRect);
   static void Draw(SDL_Texture* texture, SDL_Rect sourceRect, SDL_Rect destRect);
   static void Draw(std::shared_ptr<SDL_Texture>, SDL_Rect sourceRect, SDL_Rect destRect, bool horizontal, bool vertical);
   static void Draw(std::shared_ptr<TTF_Font> font, const char* text, SDL_Rect position);
   static void DrawHorizontalFlipped(std::shared_ptr<SDL_Texture>, SDL_Rect sourceRect, SDL_Rect destRect);
   static void DrawVerticalFlipped(std::shared_ptr<SDL_Texture>, SDL_Rect sourceRect, SDL_Rect destRect);
   static void SetBackgroundColor(BackgroundColor color);
   static void Clear();
   static void Display();

   static void ResizeWindow();

   static SDL_Renderer* GetRenderer() {
   	return renderer;
   }

  private:
   static SDL_Window* window;
   static SDL_Renderer* renderer;
   static BackgroundColor currentColor;
};
