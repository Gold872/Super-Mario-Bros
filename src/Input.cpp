#include "Input.h"

Input Input::instance;

void Input::initDefault() {
   keyBindings = std::unordered_map<Key, SDL_Scancode>{
       {Key::NONE, SDL_SCANCODE_UNKNOWN},
       {Key::RIGHT, SDL_SCANCODE_D},
       {Key::LEFT, SDL_SCANCODE_A},
       {Key::JUMP, SDL_SCANCODE_SPACE},
       {Key::DUCK, SDL_SCANCODE_S},
       {Key::SPRINT, SDL_SCANCODE_LSHIFT},
       {Key::FIREBALL, SDL_SCANCODE_Q},
       {Key::MENU_UP, SDL_SCANCODE_UP},
       {Key::MENU_DOWN, SDL_SCANCODE_DOWN},
       {Key::MENU_LEFT, SDL_SCANCODE_LEFT},
       {Key::MENU_RIGHT, SDL_SCANCODE_RIGHT},
       {Key::MENU_ACCEPT, SDL_SCANCODE_RETURN},
       {Key::MENU_ESCAPE, SDL_SCANCODE_ESCAPE},
       {Key::PAUSE, SDL_SCANCODE_ESCAPE},
   };

   for (auto key : keyBindings) {
      keysPressed.insert({key.first, false});
      keysHeld.insert({key.first, false});
   }
}

void Input::set(Key action, SDL_Scancode keyCode) {
   keyBindings.insert_or_assign(action, keyCode);
   keysPressed.insert_or_assign(action, false);
   keysHeld.insert_or_assign(action, false);
}

void Input::update(const Uint8* keystates) {
   currentRawKeys.clear();

   for (auto key : keyBindings) {
      // If key pressed
      if (keystates[key.second]) {
         // Sets the key to be held if it is already pressed
         keysHeld.insert_or_assign(key.first, keysPressed.at(key.first));
         // Sets the key to be pressed
         keysPressed.insert_or_assign(key.first, true);
      } else {
         // Sets pressed and held to false if the key is not being currently pressed
         keysPressed.insert_or_assign(key.first, false);
         keysHeld.insert_or_assign(key.first, false);
      }
   }
}

SDL_Scancode Input::getBoundKey(Key action) {
   return keyBindings.at(action);
}

bool Input::getRawKey(Key action) {
   return keysPressed.at(action);
}

bool Input::getKeyPressed(Key action) {
   return keysPressed.at(action) && !keysHeld.at(action);
}

bool Input::getKeyHeld(Key action) {
   return keysHeld.at(action);
}

std::vector<SDL_Scancode>& Input::getCurrentRawKeys() {
   return currentRawKeys;
}
