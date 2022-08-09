#pragma once

#include "command/Command.h"
#include "util/Supplier.h"

#include <functional>

class RunCommand : public Command {
  public:
   RunCommand(std::function<void()> execute) : onExecute{execute} {
      finishedSupplier = []() -> bool {
         return true;
      };
   }

   RunCommand(std::function<void()> execute, BooleanSupplier finished)
       : onExecute{execute}, finishedSupplier{finished} {}

   void execute() override {
      onExecute();
   }

   bool isFinished() override {
      return finishedSupplier();
   }

  private:
   std::function<void()> onExecute;
   BooleanSupplier finishedSupplier;
};
