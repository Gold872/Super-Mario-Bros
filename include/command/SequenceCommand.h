#pragma once

#include "command/Command.h"

#include <vector>

class SequenceCommand : public Command {
  public:
   SequenceCommand() {
      sequenceSize = 0;
      currentIndex = 0;
      sequenceFinished = false;
   }

   SequenceCommand(std::vector<Command*> commandList) {
      sequenceSize = 0;
      currentIndex = 0;
      sequenceFinished = false;

      addCommands(commandList);
   }

   void addCommands(std::vector<Command*> commandList) {
      for (Command* command : commandList) {
         commandSequence.emplace_back(std::move(command));
      }
      sequenceSize += commandList.size();
   }

   void execute() override {
      commandSequence[currentIndex]->execute();

      if (!commandSequence[currentIndex]->isFinished()) {
         return;
      }

      delete commandSequence[currentIndex];
      currentIndex++;

      if (currentIndex == sequenceSize) {
         sequenceFinished = true;
      }
   }

   bool isFinished() override {
      return sequenceFinished;
   }

  private:
   std::vector<Command*> commandSequence;

   int currentIndex;
   int sequenceSize;
   bool sequenceFinished;
};
