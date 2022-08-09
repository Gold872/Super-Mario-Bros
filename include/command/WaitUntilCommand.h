#pragma once

#include "command/Command.h"
#include "util/Supplier.h"

class WaitUntilCommand : public Command {
  public:
   WaitUntilCommand(BooleanSupplier condition) : condition{condition} {}

   void execute() override {}

   bool isFinished() override {
      return condition();
   }

  private:
   BooleanSupplier condition;
};
