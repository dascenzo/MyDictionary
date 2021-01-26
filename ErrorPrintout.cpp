#include "ErrorPrintout.h"
#include "Options.h"
#include "Error.h"
#include "CommandLine.h"
#include "StringViewOutput.h"

///////////////////////////////////////////////////////////////
// Usage errors
///////////////////////////////////////////////////////////////
// error on command: show help for that command
//    => ./path/mydict: invalid usage
//    => xxx usage: mydict xxx blah blah

// error on something more specific: show more specific help
//    => ./path/mydict: too many format flags
//    => ./path/mydict: missing or invalid argument to "-w"

// what to show for mydict "..." error => generic error
//    (since this is the "catch all" action, the user could have been trying anything)
//    => ./mydict: invalid usage
//    => try: mydict help

// what to show for help usage error?
//    => ignore extra arguments, they are doing it wrong anyway, give them help

// --- desired out ---
// $ ./mydict add blah
// ./mydict: invalid usage
// `add' usage: mydict add -w etc.

// $ ./mydict add -w
// ./mydict: missing argument for -w
// `add' usage: my dict add -w etc.

// $ ./mydict add -w -word
// ./mydict: invalid argument for -w
// try: -w -- -word

// $ ./mydict find -w word -f -ff
// ./mydict: multiple formats specified
// try: zero or one of -f, -ff, -fff

// $ ./mydict
// ./mydict: no action specified
// try: mydict help

// bad argument, too many options for shorthand find
// $ ./mydict -word
// ./mydict: invalid usage
// try: mydict help
///////////////////////////////////////////////////////////////

template<typename T>
class GeneratorBase {
public:
  GeneratorBase(const CommandLine& commandLine, const T& error) noexcept
    : commandLine(commandLine), error(error) {
  }
protected:
  ~GeneratorBase() = default;
  const CommandLine& commandLine;
  const T& error;
};

// Generates the first line to be shown on the occurrence of an error,
// generally telling the user what went wrong
template<typename T>
class ErrorLineGenerator;

template<>
class ErrorLineGenerator<EnvironmentSetupError> : public GeneratorBase<EnvironmentSetupError> {
public:
  void operator()() const noexcept {
    try {
      std::visit(*this, error.variant());
    }
    catch (const std::bad_variant_access& ) {
      std::fprintf(stderr, "%.*s: %s\n", C_STR(commandLine.programName), error.what());
    }
  }
  void operator()(const BadEnvVar& e) const noexcept {
    std::fprintf(stderr, "%.*s: invalid env var: %s\n", C_STR(commandLine.programName), e.variable());
  }
  void operator()(const SetEnvFailed& e) const noexcept {
    std::fprintf(stderr, "%.*s: failed to set environment: %s\n", C_STR(commandLine.programName), std::strerror(e.errNo()));
  }
  using GeneratorBase::GeneratorBase;
};
template<>
class ErrorLineGenerator<UsageError> : public GeneratorBase<UsageError> {
public:
  void operator()() const noexcept {
    try {
      std::visit(*this, error.variant());
    }
    catch (const std::bad_variant_access& ) {
      std::fprintf(stderr, "%.*s: %s\n", C_STR(commandLine.programName), error.what());
    }
  }
  void operator()(const GeneralActionMisuse& ) const noexcept {
    std::fprintf(stderr, "%.*s: invalid usage\n", C_STR(commandLine.programName));
  }
  void operator()(const MissingArgument& e) const noexcept {
    std::fprintf(stderr, "%.*s: missing argument for %s\n", C_STR(commandLine.programName), e.option());
  }
  void operator()(const InvalidArgument& e) const noexcept {
    std::fprintf(stderr, "%.*s: invalid argument for %s\n", C_STR(commandLine.programName), e.option());
  }
  void operator()(const MultipleFormats& ) const noexcept {
    std::fprintf(stderr, "%.*s: multiple formats given\n", C_STR(commandLine.programName));
  }
  void operator()(const NoAction& ) const noexcept {
    std::fprintf(stderr, "%.*s: no action given\n", C_STR(commandLine.programName));
  }
  using GeneratorBase::GeneratorBase;
};
template<>
class ErrorLineGenerator<DataStateError> : public GeneratorBase<DataStateError> {
public:
  void operator()() const noexcept {
    try {
      std::visit(*this, error.data());
    }
    catch (const std::bad_variant_access& ) {
      std::fprintf(stderr, "%.*s: %s\n", C_STR(commandLine.programName), error.what());
    }
  }
  void operator()(const DataStateError::WordData& data) const noexcept {
    std::fprintf(stderr, "%.*s: operation failed: word (%s) %s\n",
        C_STR(commandLine.programName), data.value(), problem());
  }
  void operator()(const DataStateError::TagData& data) const noexcept {
    std::fprintf(stderr, "%.*s: operation failed: tag (%s) %s\n",
        C_STR(commandLine.programName), data.value(), problem());
  }
  void operator()(const DataStateError::WordTagData& data) const noexcept {
    std::fprintf(stderr, "%.*s: operation failed: word/tag combination (%s/%s) %s\n",
        C_STR(commandLine.programName), data.first.value(), data.second.value(), problem());
  }
  using GeneratorBase::GeneratorBase;
private:
  const char* problem() const noexcept {
    switch (error.type()) {
    case DataStateError::MISSING: return "doesn't exist";
    case DataStateError::ALREADY_PRESENT: return "already exists";
    }
  }
};
template<>
class ErrorLineGenerator<pqxx::failure> : public GeneratorBase<pqxx::failure> {
public:
  void operator()() const noexcept {
    std::fprintf(stderr, "%.*s: database runtime error: %s\n", C_STR(commandLine.programName), error.what());
  }
  using GeneratorBase::GeneratorBase;
};
template<>
class ErrorLineGenerator<std::exception> : public GeneratorBase<std::exception> {
public:
  void operator()() const noexcept {
    std::fprintf(stderr, "%.*s: internal error: %s\n", C_STR(commandLine.programName), error.what());
  }
  using GeneratorBase::GeneratorBase;
};
template<>
class ErrorLineGenerator<UnknownException> : public GeneratorBase<UnknownException> {
public:
  void operator()() const noexcept {
    std::fprintf(stderr, "%.*s: unknown internal error\n", C_STR(commandLine.programName));
  }
  using GeneratorBase::GeneratorBase;
};

// Generates second line to be shown on occurrence of error
// generally giving the user a suggestion to correct their problem
template<typename T>
class SuggestionLineGenerator : public GeneratorBase<T> {
public:
  using GeneratorBase<T>::GeneratorBase;
  void operator()() const noexcept { }
};

template<>
class SuggestionLineGenerator<UsageError> : public GeneratorBase<UsageError> {
public:
  void operator()() const noexcept {
    try {
      std::visit(*this, error.variant());
    }
    catch (const std::bad_variant_access& ) {
      printGeneralHelp();
    }
  }
  void operator()(const GeneralActionMisuse& e) const noexcept {
    try {
      std::visit(PrintActionHelp(*this), e.action());
    }
    catch (const std::bad_variant_access& ) {
      printGeneralHelp();
    }
  }
  void operator()(const MissingArgument& e) const noexcept {
    try {
      std::visit(PrintActionHelp(*this), e.action());
    }
    catch (const std::bad_variant_access& ) {
      printGeneralHelp();
    }
  }
  void operator()(const InvalidArgument& e) const noexcept {
    std::fprintf(stderr, "try: %s -- %s\n", e.option(), e.argument());
  }
  void operator()(const MultipleFormats& ) const noexcept {
    std::fprintf(stderr, "try: zero or one of -f, -ff, -fff\n");
  }
  void operator()(const NoAction& ) const noexcept {
    printGeneralHelp();
  }
  using GeneratorBase::GeneratorBase;
private:
  void printGeneralHelp() const noexcept {
    std::fprintf(stderr, "try: %.*s help\n",
	C_STR(commandLine.baseProgramName));
  }
  void printActionHelp(const FailedAdd&) const noexcept {
    std::fprintf(stderr, ADD_USAGE "\n",
	C_STR(commandLine.baseProgramName));
  }
  void printActionHelp(const FailedRm&) const noexcept {
    std::fprintf(stderr, RM_USAGE "\n",
	C_STR(commandLine.baseProgramName));
  }
  void printActionHelp(const FailedFind&) const noexcept {
    std::fprintf(stderr, FIND_USAGE "\n",
	C_STR(commandLine.baseProgramName));
  }
  void printActionHelp(const FailedTags&) const noexcept {
    std::fprintf(stderr, TAGS_USAGE "\n",
	C_STR(commandLine.baseProgramName));
  }
  void printActionHelp(const FailedEdit&) const noexcept {
    std::fprintf(stderr, EDIT_USAGE "\n",
	C_STR(commandLine.baseProgramName));
  }
  void printActionHelp(const FailedShorthandFind&) const noexcept {
    printGeneralHelp();
  }
  class PrintActionHelp {
  public:
    PrintActionHelp(const SuggestionLineGenerator& generator) noexcept
      : m_generator(generator) {}
    template<typename FailedAction>
    void operator()(const FailedAction& failedAction) const noexcept
      { m_generator.printActionHelp(failedAction); }
  private:
    const SuggestionLineGenerator& m_generator;
  };
};

template<typename T>
static void doPrintout(const CommandLine& commandLine, const T& e) noexcept {
  ErrorLineGenerator<T>{commandLine, e}();
  SuggestionLineGenerator<T>{commandLine, e}();
}
void ErrorPrintout::handle(const CommandLine& commandLine, const EnvironmentSetupError& e) noexcept {
  doPrintout(commandLine, e);
}
void ErrorPrintout::handle(const CommandLine& commandLine, const UsageError& e) noexcept {
  doPrintout(commandLine, e);
}
void ErrorPrintout::handle(const CommandLine& commandLine, const DataStateError& e) noexcept {
  doPrintout(commandLine, e);
}
void ErrorPrintout::handle(const CommandLine& commandLine, const pqxx::failure& e) noexcept {
  doPrintout(commandLine, e);
}
void ErrorPrintout::handle(const CommandLine& commandLine, const std::exception& e) noexcept {
  doPrintout(commandLine, e);
}
void ErrorPrintout::handle(const CommandLine& commandLine, const UnknownException& e) noexcept {
  doPrintout(commandLine, e);
}
