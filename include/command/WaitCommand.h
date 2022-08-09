#pragma once

#include "Constants.h"
#include "command/Command.h"

/*
 * The WaitCommand is used to add a delay in a command sequence, it's executed for a certain amount
 * of time before being finished
 * */

class WaitCommand : public Command {
  public:
   WaitCommand(float seconds) {
      if (seconds >= 0) {
         ticks = (int)(seconds * MAX_FPS);
      } else {
         ticks = 0;
      }
   }

   void execute() override {
      ticks--;
   }

   bool isFinished() override {
      return ticks <= 0;
   }

  private:
   int ticks;
};
