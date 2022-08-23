#pragma once

#include <SDL2/SDL_mixer.h>

#include <memory>
#include <unordered_map>

enum class SoundID
{
   BLOCK_BREAK,
   BLOCK_HIT,
   BOWSER_FALL,
   BOWSER_FIRE,
   CANNON_FIRE,
   COIN,
   DEATH,
   FIREBALL,
   FLAG_RAISE,
   GAME_OVER,
   JUMP,
   KICK,
   ONE_UP,
   PAUSE,
   PIPE,
   POWER_UP_APPEAR,
   POWER_UP_COLLECT,
   SHRINK,
   STOMP,
   TIMER_TICK,
   CASTLE_CLEAR
};

enum class MusicID
{
   OVERWORLD,
   UNDERGROUND,
   CASTLE,
   UNDERWATER,
   SUPER_STAR,
   GAME_WON
};

class SoundManager {
  public:
   static SoundManager& Get() {
      return instance;
   }

   int Init();
   int Quit();

   std::shared_ptr<Mix_Chunk> loadSound(const char* path);

   std::shared_ptr<Mix_Music> loadMusic(const char* path);

   void playSound(SoundID sound);
   void playMusic(MusicID music);

   void pauseSounds();
   void resumeSounds();

   void pauseMusic();
   void resumeMusic();
   void stopMusic();

  private:
   SoundManager() {}

   SoundManager(const SoundManager&) = delete;

   static SoundManager instance;

   void loadSounds();
   void loadMusics();

   std::unordered_map<SoundID, std::shared_ptr<Mix_Chunk>> sounds;
   std::unordered_map<MusicID, std::shared_ptr<Mix_Music>> musics;
};
