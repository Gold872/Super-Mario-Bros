#pragma once

#include "ECS/ECS.h"
#include "Math.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <memory>

enum class BackgroundColor
{
   NONE,
   BLACK,
   BLUE
};

class TextureManager {
  public:
   static TextureManager& Get() {
      return instance;
   }

   int Init();
   int Quit();

   SDL_Texture* LoadTexture(const char* path);
   std::shared_ptr<SDL_Texture> LoadSharedTexture(const char* path);
   std::shared_ptr<TTF_Font> LoadSharedFont(const char* path, int fontSize);
   void Draw(SDL_Texture* texture, SDL_Rect destRect);
   void Draw(SDL_Texture* texture, SDL_Rect sourceRect, SDL_Rect destRect);
   void Draw(std::shared_ptr<SDL_Texture>, SDL_Rect sourceRect, SDL_Rect destRect, bool horizontal,
             bool vertical);
   void Draw(std::shared_ptr<TTF_Font> font, const char* text, SDL_Rect position);
   void DrawHorizontalFlipped(std::shared_ptr<SDL_Texture>, SDL_Rect sourceRect, SDL_Rect destRect);
   void DrawVerticalFlipped(std::shared_ptr<SDL_Texture>, SDL_Rect sourceRect, SDL_Rect destRect);
   void SetBackgroundColor(BackgroundColor color);
   void Clear();
   void Display();

   void ResizeWindow();

   SDL_Renderer* getRenderer() {
      return renderer;
   }

   BackgroundColor getBackgroundColor() {
      return currentColor;
   }

  private:
   TextureManager() {}

   TextureManager(const TextureManager&) = delete;

   static TextureManager instance;

   SDL_Window* window;
   SDL_Renderer* renderer;
   BackgroundColor currentColor;
};
