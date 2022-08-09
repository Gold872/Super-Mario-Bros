#include "command/CommandScheduler.h"

#include <algorithm>

CommandScheduler CommandScheduler::instance;

CommandScheduler& CommandScheduler::getInstance() {
   return instance;
}

void CommandScheduler::addCommand(Command* command) {
   commandQueue.emplace_back(std::move(command));
}

void CommandScheduler::run() {
   for (auto* command : commandQueue) {
      command->execute();

      if (command->isFinished()) {
         commandQueue.erase(std::find(commandQueue.begin(), commandQueue.end(), command));

         delete command;
      }
   }

   commandQueue.shrink_to_fit();
}
