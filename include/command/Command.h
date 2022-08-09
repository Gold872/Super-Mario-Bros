#pragma once

class Command {
  public:
   Command() {}
   virtual ~Command() {}

   virtual void execute() {}
   virtual bool isFinished() {
      return true;
   }
};
