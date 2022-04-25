#include "Core.h"

Core core;

int main(int argc, char** argv) {
   if (core.init() != 0) {
      return -1;
   }

   core.run();

   return 0;
}
