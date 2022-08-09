#pragma once

#include "command/Command.h"

#include <iostream>
#include <string>

class PrintCommand : public Command {
  public:
   PrintCommand(std::string message) : message{message} {}

   void execute() override {
      std::cout << message << '\n';
   }

  private:
   std::string message;
};
