#pragma once
#include "Action.h"

class AddAction : public Action {
public:
  void execute(const CommandLine&) override;
};
class RmAction : public Action {
public:
  void execute(const CommandLine&) override;
};
class FindAction : public Action {
public:
  void execute(const CommandLine&) override;
};
class TagsAction : public Action {
public:
  void execute(const CommandLine&) override;
};
class EditAction : public Action {
public:
  void execute(const CommandLine&) override;
};
class HelpAction : public Action {
public:
  void execute(const CommandLine&) override;
};
class ShorthandFindAction : public Action {
public:
  void execute(const CommandLine&) override;
};
