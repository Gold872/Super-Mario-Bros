#pragma once

#include "command/Command.h"

#include <vector>

class CommandScheduler {
  public:
   CommandScheduler() {}

   void addCommand(Command* command);
   void run();

   static CommandScheduler& getInstance();

  private:
   CommandScheduler(const CommandScheduler&) = delete;

   static CommandScheduler instance;

   std::vector<Command*> commandQueue;
};
