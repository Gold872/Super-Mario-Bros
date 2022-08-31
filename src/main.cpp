#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#define SDL_MAIN_HANDLED
#include "Core.h"

void runLoop(void* data) {
   Core* core = (Core*)data;
   core->mainLoop();
}

int main(int argc, char** argv) {
   Core core;

   if (core.init() != 0) {
      return -1;
   }
#ifdef __EMSCRIPTEN__
   emscripten_set_main_loop_arg(runLoop, &core, 0, 1);
#else
   core.run();
#endif

   return 0;
}
