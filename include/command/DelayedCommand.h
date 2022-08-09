#pragma once

#include "Constants.h"
#include "command/Command.h"

#include <functional>

class DelayedCommand : public Command {
  public:
   DelayedCommand(std::function<void()> onExecute, float delay) : onExecute{onExecute} {
      if (delay >= 0) {
         ticks = (int)(delay * MAX_FPS);
      } else {
         ticks = 0;
      }
   }

   void execute() override {
      ticks--;

      if (ticks == 0) {
         onExecute();
      }
   }

   bool isFinished() override {
      return ticks <= 0;
   }

  private:
   std::function<void()> onExecute;
   int ticks;
};
