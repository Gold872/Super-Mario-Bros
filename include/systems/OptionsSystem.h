#pragma once

#include "ECS/ECS.h"
#include "Input.h"

#include <string>

class OptionsSystem : public System {
  public:
   void onAddedToWorld(World* world) override;

   void onRemovedFromWorld(World* world) override;

   void tick(World* world) override;

   void handleInput() override;

   bool isFinished() {
      return finished;
   }

  private:
   void hideKeySelectEntities();
   void showKeySelectEntities();

   std::string getKeybindString(Key key);
   std::string getKeyString(Key key);

   Entity* optionsBackground;
   Entity* leftKeybindText;
   Entity* leftKeyName;
   Entity* rightKeybindText;
   Entity* rightKeyName;
   Entity* jumpKeybindText;
   Entity* jumpKeyName;
   Entity* duckKeybindText;
   Entity* duckKeyName;
   Entity* sprintKeybindText;
   Entity* sprintKeyName;
   Entity* fireballKeybindText;
   Entity* fireballKeyName;
   Entity* goBackText;

   Entity* infoBackground;
   Entity* infoTextEnter;
   Entity* infoTextEscape;

   Entity* cursor;
   Entity* keyUnderline;

   const int totalOptions = 6;

   int currentSelectedOption = 0;
   Key currentWaitingKey = Key::NONE;

   std::unordered_map<int, Key> optionsKeyMap = {
       {0, Key::LEFT}, {1, Key::RIGHT},  {2, Key::JUMP},
       {3, Key::DUCK}, {4, Key::SPRINT}, {5, Key::FIREBALL},
   };

   std::unordered_map<Key, Entity*> keyEntityMap;

   bool finished = false;
};
