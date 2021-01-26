#include <iostream>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "Error.h"
#include "CommandLine.h"
#include "Actions.h"
#include "Options.h"
#include "Environment.h"
#include "ErrorPrintout.h"
#include <pqxx/pqxx>

static void dispatch(const CommandLine& commandLine);
static std::unique_ptr<Action> getAction(std::string_view from);

int main(int argc, char* argv[]) {
  const CommandLine commandLine(argc, argv);
  try {
    setupEnvironment();
    dispatch(commandLine);
    return 0;
  }
  catch (const EnvironmentSetupError& e) {
    ErrorPrintout::handle(commandLine, e);
  }
  catch (const UsageError& e) {
    ErrorPrintout::handle(commandLine, e);
  }
  catch (const DataStateError& e) {
    ErrorPrintout::handle(commandLine, e);
  }
  catch (const pqxx::failure& e) { // database runtime error
    ErrorPrintout::handle(commandLine, e);
  }
  catch (const std::exception& e) {
    ErrorPrintout::handle(commandLine, e);
  }
  catch (...) {
    ErrorPrintout::handle(commandLine, UnknownException());
  }
  return EXIT_FAILURE;
}
static void dispatch(const CommandLine& commandLine) {
  if (commandLine.args.size() == 0) {
    throw UsageError(NoAction());
  }
  auto action = getAction(commandLine.args[0]);
  action->execute(commandLine);
}
static std::unique_ptr<Action> getAction(std::string_view from) {
  if (matches(from, Opt::add)) {
    return std::make_unique<AddAction>();
  }
  else if (matches(from, Opt::rm)) {
    return std::make_unique<RmAction>();
  }
  else if (matches(from, Opt::find)) {
    return std::make_unique<FindAction>();
  }
  else if (matches(from, Opt::tags)) {
    return std::make_unique<TagsAction>();
  }
  else if (matches(from, Opt::edit)) {
    return std::make_unique<EditAction>();
  }
  else if (matches(from, Opt::help)
           || matches(from, Opt::_h)) {
    return std::make_unique<HelpAction>();
  }
  else {
    return std::make_unique<ShorthandFindAction>();
  }
}
