#include "TextureManager.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "Constants.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>

TextureManager TextureManager::instance;

int TextureManager::Init() {
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
      std::cerr << "Failed to Initialize SDL2: " << SDL_GetError() << std::endl;
      return -1;
   }
   if (IMG_Init(IMG_INIT_PNG) < 0) {
      std::cerr << "Failed to Initialize SDL2_IMG: " << SDL_GetError() << std::endl;
      return -1;
   }
   if (TTF_Init() != 0) {
      std::cerr << "Failed to Initialize SDL2_TTF: " << SDL_GetError() << std::endl;
      return -1;
   }

   window = SDL_CreateWindow("Super Mario Bros", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             SCREEN_WIDTH, SCREEN_HEIGHT,
                             SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
   if (!window) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to Create Window: %s\n", SDL_GetError());
      std::cerr << "Failed to Create Window: " << SDL_GetError() << std::endl;
      return -1;
   }

   SDL_Surface* iconSurface = IMG_Load("res/sprites/icons/windowicon.png");

   SDL_SetWindowIcon(window, iconSurface);

   SDL_FreeSurface(iconSurface);

   renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
   if (!renderer) {
      SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to Create Renderer: %s", SDL_GetError());
      std::cerr << "Failed to Create Renderer: " << SDL_GetError() << std::endl;
      return -1;
   }

   if (SDL_SetRenderDrawColor(renderer, 92, 148, 252, 255) != 0) {
      std::cerr << "Failed to set Draw Color: " << SDL_GetError() << std::endl;
   }

   SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

   return 0;
}

int TextureManager::Quit() {
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);

   IMG_Quit();
   SDL_Quit();

   return 0;
}

SDL_Texture* TextureManager::LoadTexture(const char* path) {
   SDL_Surface* tempSurface = IMG_Load(path);
   SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
   SDL_FreeSurface(tempSurface);
   return texture;
}

std::shared_ptr<SDL_Texture> TextureManager::LoadSharedTexture(const char* path,
                                                               bool blueTransparent) {
   SDL_Surface* surface = IMG_Load(path);
   if (blueTransparent) {
      SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 147, 187, 236));
   }
   std::shared_ptr<SDL_Texture> texture(SDL_CreateTextureFromSurface(renderer, surface),
                                        SDL_DestroyTexture);
   SDL_FreeSurface(surface);
   return texture;
}

std::shared_ptr<TTF_Font> TextureManager::LoadSharedFont(const char* path, int fontSize) {
   return std::shared_ptr<TTF_Font>(TTF_OpenFont(path, fontSize), TTF_CloseFont);
}

void TextureManager::Draw(SDL_Texture* texture, SDL_Rect destRect) {
   SDL_RenderCopy(renderer, texture, nullptr, &destRect);
}

void TextureManager::Draw(SDL_Texture* texture, SDL_Rect sourceRect, SDL_Rect destRect) {
   SDL_RenderCopy(renderer, texture, &sourceRect, &destRect);
}

void TextureManager::Draw(std::shared_ptr<SDL_Texture> texture, SDL_Rect sourceRect,
                          SDL_Rect destRect, bool horizontal, bool vertical) {
   if (!horizontal && !vertical) {
      SDL_RenderCopyEx(renderer, texture.get(), &sourceRect, &destRect, 0, nullptr, SDL_FLIP_NONE);
      return;
   }
   if (horizontal && vertical) {
      SDL_RenderCopyEx(renderer, texture.get(), &sourceRect, &destRect, 180, nullptr,
                       SDL_FLIP_NONE);
      return;
   } else if (horizontal) {
      SDL_RenderCopyEx(renderer, texture.get(), &sourceRect, &destRect, 0, nullptr,
                       SDL_FLIP_HORIZONTAL);
      return;
   } else if (vertical) {
      SDL_RenderCopyEx(renderer, texture.get(), &sourceRect, &destRect, 0, nullptr,
                       SDL_FLIP_VERTICAL);
      return;
   }
}

void TextureManager::Draw(std::shared_ptr<TTF_Font> font, const char* text, SDL_Rect position) {
   SDL_Color color = {255, 255, 255};
   SDL_Surface* textSurface = TTF_RenderText_Blended(font.get(), text, color);

   SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);

   SDL_RenderCopy(renderer, texture, nullptr, &position);

   SDL_FreeSurface(textSurface);
   SDL_DestroyTexture(texture);
}

void TextureManager::DrawHorizontalFlipped(std::shared_ptr<SDL_Texture> texture,
                                           SDL_Rect sourceRect, SDL_Rect destRect) {
   SDL_RenderCopyEx(renderer, texture.get(), &sourceRect, &destRect, 0, nullptr,
                    SDL_FLIP_HORIZONTAL);
}

void TextureManager::DrawVerticalFlipped(std::shared_ptr<SDL_Texture> texture, SDL_Rect sourceRect,
                                         SDL_Rect destRect) {
   SDL_RenderCopyEx(renderer, texture.get(), &sourceRect, &destRect, 0, nullptr, SDL_FLIP_VERTICAL);
}

void TextureManager::SetBackgroundColor(BackgroundColor color) {
   switch (color) {
      case BackgroundColor::BLUE:
         SDL_SetRenderDrawColor(renderer, 92, 148, 252, 255);
         break;
      case BackgroundColor::BLACK:
         SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
         break;
      default:
         break;
   }
   currentColor = color;
}

void TextureManager::Clear() {
   // Creates black borders
   SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
   SDL_RenderClear(renderer);

   switch (currentColor) {
      case BackgroundColor::BLUE:
         SDL_SetRenderDrawColor(renderer, 92, 148, 252, 255);
         break;
      case BackgroundColor::BLACK:
         SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
         break;
      default:
         break;
   }

   // Draws the background only where the game is supposed to be
   SDL_Rect backgroundRect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
   SDL_RenderFillRect(renderer, &backgroundRect);
}

void TextureManager::Display() {
   SDL_RenderPresent(renderer);
}

#ifdef __EMSCRIPTEN__
EM_JS(int, get_canvas_width, (), { return canvas.width; });
EM_JS(int, get_canvas_height, (), { return canvas.height; });
#endif

void TextureManager::ResizeWindow() {
#ifdef __EMSCRIPTEN__
   int windowWidth = get_canvas_width();
   int windowHeight = get_canvas_height();
#else
   int windowWidth = SDL_GetWindowSurface(window)->w;
   int windowHeight = SDL_GetWindowSurface(window)->h;
#endif
   float scaleWidth = (float)((float)windowWidth / (float)SCREEN_WIDTH);
   float scaleHeight = (float)((float)windowHeight / (float)SCREEN_HEIGHT);

   SDL_RenderSetScale(renderer, scaleWidth, scaleHeight);
}
