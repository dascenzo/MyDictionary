#pragma once

class CommandLine;

class Action {
public:
  virtual void execute(const CommandLine&) = 0;
  virtual ~Action() = default;
};
