#include "Core.h"

int main(int argc, char** argv) {
   Core core;

   if (core.init() != 0) {
      return -1;
   }

   core.run();

   return 0;
}
