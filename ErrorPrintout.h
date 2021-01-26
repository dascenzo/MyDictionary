#pragma once
#include <pqxx/pqxx>
#include <exception>
#include "Error.h"

// prints out a message to the user appropriate to the kind of error that occurred
namespace ErrorPrintout {
  void handle(const CommandLine&, const EnvironmentSetupError& e) noexcept;
  void handle(const CommandLine&, const UsageError& e) noexcept;
  void handle(const CommandLine&, const DataStateError& e) noexcept;
  void handle(const CommandLine&, const pqxx::failure& e) noexcept;
  void handle(const CommandLine&, const std::exception& e) noexcept;
  void handle(const CommandLine&, const UnknownException& e) noexcept;
}
