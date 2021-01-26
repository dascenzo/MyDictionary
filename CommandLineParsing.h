#pragma once
#include "Error.h"
#include "EntryFormat.h"
#include "CommandLine.h"
#include <string>
#include <string_view>
#include <optional>

template<typename CallingAction>
EntryFormat flag2format(const CallingAction* callingAction, bool f, bool ff, bool fff) {
  if (!f && !ff && !fff) {
    return FORMAT_MODEST;
  }
  else if (f && !ff && !fff) {
    return FORMAT_BRIEF;
  }
  else if (!f && ff && !fff) {
    return FORMAT_MODEST;
  }
  else if (!f && !ff && fff) {
    return FORMAT_ALL;
  }
  else {
    throw actionUsageError<MultipleFormats>(callingAction);
  }
}
struct TextParseReceiver {
  virtual void text(std::string_view text) = 0;
  virtual void missingArgument(std::string_view option) = 0;
  virtual void invalidArgument(std::string_view option, std::string_view arg) = 0;
};
/// parses:
/// [--] bar
///  calls 1 of receiver's member functions 1 time to handle results
void _parseCommandLineText(TextParseReceiver& receiver, int& i, const CommandLine& commandLine) {

  const auto option = (i == 0 ? commandLine.programName : commandLine.args[i - 1]);

  if (i < commandLine.args.size()) {
    if (commandLine.args[i] == "--") {
      if (i + 1 < commandLine.args.size()) {
        const std::size_t idx = i + 1;
        i += 2;
        receiver.text(commandLine.args[idx]);
      }
      else {
        receiver.missingArgument(option);
      }
    }
    else {
      if (commandLine.args[i].size() > 0 && commandLine.args[i][0] == '-') {
        receiver.invalidArgument(option, commandLine.args[i]);
      }
      else {
        receiver.text(commandLine.args[i++]);
      }
    }
  }
  else {
    receiver.missingArgument(option);
  }
}
// a copy is returned rather than string_view since technically the lifetime of commandLine isn't known

// throws exceptions
template<typename CallingAction>
std::string parseCommandLineText(const CallingAction* callingAction, int& i, const CommandLine& commandLine) {
  struct ExceptionReceiver : TextParseReceiver {
    ExceptionReceiver(const CallingAction* callingAction) : _callingAction(callingAction) {}
    void text(std::string_view text) override {
      _text = text;
    }
    void missingArgument(std::string_view option) override {
      throw actionUsageError<MissingArgument>(_callingAction, option);
    }
    void invalidArgument(std::string_view option, std::string_view arg) override {
      throw actionUsageError<InvalidArgument>(_callingAction, option, arg);
    }
    std::string _text;
    const CallingAction* _callingAction;
  };
  ExceptionReceiver receiver(callingAction);
  _parseCommandLineText(receiver, i, commandLine);
  return receiver._text;
}

// doesn't throw exceptions
std::optional<std::string> tryParseCommandLineText(int& i, const CommandLine& commandLine) {
  struct IgnoreReceiver : TextParseReceiver {
    void text(std::string_view text) override {
      _text = std::string(text);
    }
    void missingArgument(std::string_view /*option*/) override { }
    void invalidArgument(std::string_view /*option*/, std::string_view /*arg*/) override { }
    std::optional<std::string> _text;
  };
  IgnoreReceiver receiver;
  _parseCommandLineText(receiver, i, commandLine);
  return receiver._text;
}
