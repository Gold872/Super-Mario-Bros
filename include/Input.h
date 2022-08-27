#pragma once

#include <SDL2/SDL.h>

#include <unordered_map>

enum class Key : int
{
   RIGHT,
   LEFT,
   JUMP,
   DUCK,
   SPRINT,
   FIREBALL,
   MENU_UP,
   MENU_DOWN,
   MENU_LEFT,
   MENU_RIGHT,
   MENU_ACCEPT,  // Select the current option
   PAUSE,
};

class Input {
  public:
   static Input& Get() {
      return instance;
   }

   void initDefault();

   void set(const Key action, SDL_Scancode keyCode);

   void update(const Uint8* keystates);

   bool getRawKey(Key action);

   bool getKeyPressed(Key action);

   bool getKeyHeld(Key action);

  private:
   static Input instance;

   Input() {
      initDefault();
   }

   Input(const Input&) = delete;

   std::unordered_map<Key, SDL_Scancode> keyBindings;
   std::unordered_map<Key, bool> keysPressed;
   std::unordered_map<Key, bool> keysHeld;
};
