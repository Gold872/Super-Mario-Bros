#include "systems/OptionsSystem.h"

#include "ECS/Components.h"
#include "TextureManager.h"

#include <string>

std::string OptionsSystem::getKeybindString(Key key) {
   SDL_Keycode keycode = SDL_GetKeyFromScancode(Input::Get().getBoundKey(key));
   return std::string(SDL_GetKeyName(keycode));
}

std::string OptionsSystem::getKeyString(Key key) {
   switch (key) {
      case Key::LEFT:
         return "LEFT";
      case Key::RIGHT:
         return "RIGHT";
      case Key::JUMP:
         return "JUMP";
      case Key::DUCK:
         return "DUCK";
      case Key::SPRINT:
         return "SPRINT";
      case Key::FIREBALL:
         return "FIREBALL";
      default:
         return "";
   }
}

void OptionsSystem::onAddedToWorld(World* world) {
   {  // Cursor
      cursor = world->create();
      cursor->addComponent<PositionComponent>(Vector2f(5, 2) * SCALED_CUBE_SIZE, Vector2i());
      cursor->addComponent<TextComponent>(">", 16);
   }

   {  // Underline
      keyUnderline = world->create();
      keyUnderline->addComponent<PositionComponent>(Vector2f(14, 2) * SCALED_CUBE_SIZE, Vector2i());
      keyUnderline->addComponent<TextComponent>("_", 16, false, false);
   }

   {  // Options Background
      optionsBackground = world->create();
      optionsBackground->addComponent<PositionComponent>(Vector2f(4.5, 1.5) * SCALED_CUBE_SIZE,
                                                         Vector2i(16, 12) * SCALED_CUBE_SIZE);
      optionsBackground->addComponent<TextureComponent>(TextureManager::Get().LoadSharedTexture(
          "res/sprites/icons/optionsbackground.png", false));
      optionsBackground->addComponent<IconComponent>();
   }

   {  // Info text background
      infoBackground = world->create();
      infoBackground->addComponent<PositionComponent>(Vector2f(5.5, 10) * SCALED_CUBE_SIZE,
                                                      Vector2i(14, 3) * SCALED_CUBE_SIZE);
      infoBackground
          ->addComponent<TextureComponent>(TextureManager::Get().LoadSharedTexture(
              "res/sprites/icons/optionsinfobackground.png", false))
          ->setVisible(false);
      infoBackground->addComponent<IconComponent>();
   }

   {  // Left key text
      leftKeybindText = world->create();
      leftKeybindText->addComponent<PositionComponent>(Vector2f(6, 2) * SCALED_CUBE_SIZE,
                                                       Vector2i());
      leftKeybindText->addComponent<TextComponent>("LEFT KEY:", 16);

      leftKeyName = world->create();
      leftKeyName->addComponent<PositionComponent>(Vector2f(14, 2) * SCALED_CUBE_SIZE, Vector2i());
      leftKeyName->addComponent<TextComponent>(getKeybindString(Key::LEFT), 16);
   }

   {  // Right key text
      rightKeybindText = world->create();
      rightKeybindText->addComponent<PositionComponent>(Vector2f(6, 3) * SCALED_CUBE_SIZE,
                                                        Vector2i());
      rightKeybindText->addComponent<TextComponent>("RIGHT KEY:", 16);

      rightKeyName = world->create();
      rightKeyName->addComponent<PositionComponent>(Vector2f(14, 3) * SCALED_CUBE_SIZE, Vector2i());
      rightKeyName->addComponent<TextComponent>(getKeybindString(Key::RIGHT), 16);
   }

   {  // Jump key text
      jumpKeybindText = world->create();
      jumpKeybindText->addComponent<PositionComponent>(Vector2f(6, 4) * SCALED_CUBE_SIZE,
                                                       Vector2i());
      jumpKeybindText->addComponent<TextComponent>("JUMP KEY:", 16);

      jumpKeyName = world->create();
      jumpKeyName->addComponent<PositionComponent>(Vector2f(14, 4) * SCALED_CUBE_SIZE, Vector2i());
      jumpKeyName->addComponent<TextComponent>(getKeybindString(Key::JUMP), 16);
   }

   {  // Duck key text
      duckKeybindText = world->create();
      duckKeybindText->addComponent<PositionComponent>(Vector2f(6, 5) * SCALED_CUBE_SIZE,
                                                       Vector2i());
      duckKeybindText->addComponent<TextComponent>("DUCK KEY:", 16);

      duckKeyName = world->create();
      duckKeyName->addComponent<PositionComponent>(Vector2f(14, 5) * SCALED_CUBE_SIZE, Vector2i());
      duckKeyName->addComponent<TextComponent>(getKeybindString(Key::DUCK), 16);
   }

   {  // Sprint key text
      sprintKeybindText = world->create();
      sprintKeybindText->addComponent<PositionComponent>(Vector2f(6, 6) * SCALED_CUBE_SIZE,
                                                         Vector2i());
      sprintKeybindText->addComponent<TextComponent>("SPRINT KEY:", 16);

      sprintKeyName = world->create();
      sprintKeyName->addComponent<PositionComponent>(Vector2f(14, 6) * SCALED_CUBE_SIZE,
                                                     Vector2i());
      sprintKeyName->addComponent<TextComponent>(getKeybindString(Key::SPRINT), 16);
   }

   {  // Fireball key text
      fireballKeybindText = world->create();
      fireballKeybindText->addComponent<PositionComponent>(Vector2f(6, 7) * SCALED_CUBE_SIZE,
                                                           Vector2i());
      fireballKeybindText->addComponent<TextComponent>("FIREBALL KEY:", 16);

      fireballKeyName = world->create();
      fireballKeyName->addComponent<PositionComponent>(Vector2f(14, 7) * SCALED_CUBE_SIZE,
                                                       Vector2i());
      fireballKeyName->addComponent<TextComponent>(getKeybindString(Key::FIREBALL), 16);
   }

   {  // Go back text
      goBackText = world->create();
      goBackText->addComponent<PositionComponent>(Vector2f(6, 9) * SCALED_CUBE_SIZE, Vector2i());
      goBackText->addComponent<TextComponent>("GO BACK", 16);
   }

   {  // Info text to enter a key
      infoTextEnter = world->create();
      infoTextEnter->addComponent<PositionComponent>(Vector2f(6.5, 10.75) * SCALED_CUBE_SIZE,
                                                     Vector2i());
      infoTextEnter->addComponent<TextComponent>("PRESS KEY FOR ", 16, false, false);
   }

   {  // Info text to press escape
      infoTextEscape = world->create();
      infoTextEscape->addComponent<PositionComponent>(Vector2f(6.5, 11.75) * SCALED_CUBE_SIZE,
                                                      Vector2i());
      infoTextEscape->addComponent<TextComponent>("PRESS ESC TO CANCEL", 16, false, false);
   }

   keyEntityMap = {
       {Key::LEFT, leftKeyName}, {Key::RIGHT, rightKeyName},   {Key::JUMP, jumpKeyName},
       {Key::DUCK, duckKeyName}, {Key::SPRINT, sprintKeyName}, {Key::FIREBALL, fireballKeyName},
   };
}

void OptionsSystem::onRemovedFromWorld(World* world) {
   world->destroy(cursor);
   world->destroy(keyUnderline);
   world->destroy(optionsBackground);
   world->destroy(infoBackground);
   world->destroy(leftKeybindText);
   world->destroy(leftKeyName);
   world->destroy(rightKeybindText);
   world->destroy(rightKeyName);
   world->destroy(jumpKeybindText);
   world->destroy(jumpKeyName);
   world->destroy(duckKeybindText);
   world->destroy(duckKeyName);
   world->destroy(sprintKeybindText);
   world->destroy(sprintKeyName);
   world->destroy(fireballKeybindText);
   world->destroy(fireballKeyName);
   world->destroy(goBackText);
   world->destroy(infoTextEnter);
   world->destroy(infoTextEscape);
}

void OptionsSystem::tick(World* world) {
   if (currentWaitingKey == Key::NONE) {
      return;
   }
   SDL_Event event;

#ifdef __EMSCRIPTEN__
   while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
         break;
      }
   }
#else
   SDL_WaitEvent(&event);
#endif

   if (event.type != SDL_KEYDOWN) {
      return;
   }
   // Updates the input since the WaitEvent would cause a delay
   Input::Get().update(SDL_GetKeyboardState(NULL));

   if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
      hideKeySelectEntities();
      currentWaitingKey = Key::NONE;
      return;
   }

   Input::Get().set(currentWaitingKey, event.key.keysym.scancode);

   Entity* keyText = keyEntityMap.at(currentWaitingKey);

   keyText->getComponent<TextComponent>()->destroyTexture();
   keyText->getComponent<TextComponent>()->text = getKeybindString(currentWaitingKey);

   hideKeySelectEntities();

   currentWaitingKey = Key::NONE;
}

void OptionsSystem::handleInput() {
   if (currentWaitingKey != Key::NONE) {
      return;
   }

   Input& input = Input::Get();

   if (input.getKeyPressed(Key::MENU_DOWN)) {
      if (currentSelectedOption < totalOptions) {
         currentSelectedOption++;
         if (currentSelectedOption == totalOptions) {
            cursor->getComponent<PositionComponent>()->position.y = 9 * SCALED_CUBE_SIZE;
         } else {
            cursor->getComponent<PositionComponent>()->position.y += SCALED_CUBE_SIZE;
         }
      }
   }

   if (input.getKeyPressed(Key::MENU_UP)) {
      if (currentSelectedOption > 0) {
         currentSelectedOption--;
         if (currentSelectedOption == totalOptions - 1) {
            cursor->getComponent<PositionComponent>()->position.y =
                (2 + totalOptions - 1) * SCALED_CUBE_SIZE;
         } else {
            cursor->getComponent<PositionComponent>()->position.y -= SCALED_CUBE_SIZE;
         }
      }
   }

   if (input.getKeyPressed(Key::MENU_ACCEPT)) {
      if (currentSelectedOption != 6) {
         currentWaitingKey = optionsKeyMap.at(currentSelectedOption);

         Entity* keyText = keyEntityMap.at(currentWaitingKey);

         keyUnderline->getComponent<PositionComponent>()->position.y =
             (currentSelectedOption + 2) * SCALED_CUBE_SIZE + 2;

         keyUnderline->getComponent<PositionComponent>()->scale.x =
             keyText->getComponent<PositionComponent>()->scale.x + SCALED_CUBE_SIZE / 4;

         showKeySelectEntities();

         infoTextEnter->getComponent<TextComponent>()->destroyTexture();
         infoTextEnter->getComponent<TextComponent>()->text =
             "PRESS KEY FOR " + getKeyString(currentWaitingKey);
      } else {
         finished = true;
      }
   }

   if (input.getKeyPressed(Key::MENU_ESCAPE)) {
      finished = true;
   }
}

void OptionsSystem::hideKeySelectEntities() {
   infoBackground->getComponent<TextureComponent>()->setVisible(false);
   keyUnderline->getComponent<TextComponent>()->setVisible(false);
   infoTextEnter->getComponent<TextComponent>()->setVisible(false);
   infoTextEscape->getComponent<TextComponent>()->setVisible(false);
}

void OptionsSystem::showKeySelectEntities() {
   infoBackground->getComponent<TextureComponent>()->setVisible(true);
   keyUnderline->getComponent<TextComponent>()->setVisible(true);
   infoTextEnter->getComponent<TextComponent>()->setVisible(true);
   infoTextEscape->getComponent<TextComponent>()->setVisible(true);
}
