#pragma once
#include <stdexcept>
#include <string>
#include <variant>
#include <type_traits>
#include "SafeString.h"
#include "Actions.h"

// XXX check that the variant is nothrow move constructible, which confirms
// every alternative of the variant can be moved into the parent class
// constructor, and the ctx param can be moved into the member variable.

// XXX what() returns as much information as practical. Error data is exposed
// through getters on the exception object. Code that prints out or whatever
// can use those if it needs to. The purpose of this is to keep the exception
// objects themselves from possibly throwing. For example a call to new might
// be needed to create the "full" what() message.

// === UnknownException, to signal catch(...) ===
struct UnknownException {};

// === UpdateSpecificationError ===
class UpdateSpecificationError : public std::exception {
public:
  UpdateSpecificationError(std::string_view word) noexcept;
  const char* word() const noexcept;
  const char* what() const noexcept override;
private:
  SafeString m_word;
};

// === DataStateError ===
struct WordData {};
struct TagData {};
template<typename DataKind>
class ErrorData {
public:
  using kind = DataKind;
  ErrorData(std::string_view value) noexcept
    : m_value(value) {}
  const char* value() const noexcept
    { return m_value; }
private:
  SafeString m_value;
};
class DataStateError : public std::exception {
public:
  enum ErrorType { MISSING, ALREADY_PRESENT };
  using WordData = ErrorData<WordData>;
  using TagData = ErrorData<TagData>;
  using WordTagData = std::pair<WordData, TagData>;
  using DataVariant = std::variant<WordData, TagData, WordTagData>;
  static_assert(std::is_nothrow_move_constructible<DataVariant>::value);
  DataStateError(DataVariant data, ErrorType) noexcept;
  const DataVariant& data() const noexcept;
  ErrorType type() const noexcept;
  const char* what() const noexcept override;
private:
  DataVariant m_data;
  ErrorType m_errorType;
};

// === EnvironmentSetupError ===
class BadEnvVar {
public:
  BadEnvVar(std::string_view badVariable) noexcept;
  const char* variable() const noexcept;
private:
  SafeString m_badVariable;
};
class SetEnvFailed {
public:
  SetEnvFailed(int errNo) noexcept;
  int errNo() const noexcept;
private:
  int m_errNo;
};
class EnvironmentSetupError : public std::exception {
public:
  using Variant = std::variant<BadEnvVar, SetEnvFailed>;
  static_assert(std::is_nothrow_move_constructible<Variant>::value);
  EnvironmentSetupError(Variant variant) noexcept;
  const Variant& variant() const noexcept;
  const char* what() const noexcept override;
private:
  Variant m_variant;
};

// === UsageError ===
struct FailedAdd {};
struct FailedRm {};
struct FailedFind {};
struct FailedTags {};
struct FailedEdit {};
struct FailedShorthandFind {};

template<typename ... Actions>
class ActionMisuseBase {
public:
  using Variant = std::variant<Actions...>;
  static_assert(std::is_nothrow_move_constructible<Variant>::value);
  ActionMisuseBase(Variant action) noexcept : m_action(std::move(action))
    { }
  const Variant& action() const noexcept
    { return m_action; }
private:
  Variant m_action;
};
using ActionMisuseAll = ActionMisuseBase<
  FailedAdd, FailedRm, FailedFind, FailedTags, FailedEdit, FailedShorthandFind
>;
using ActionMisuseNoShorthand = ActionMisuseBase<
  FailedAdd, FailedRm, FailedFind, FailedTags, FailedEdit
>;
class GeneralActionMisuse : public ActionMisuseAll {
  using ActionMisuseAll::ActionMisuseAll;
};
class MissingArgument : public ActionMisuseNoShorthand {
public:
  MissingArgument(Variant action, std::string_view option) noexcept;
  const char* option() const noexcept;
private:
  SafeString m_option;
};
class InvalidArgument : public ActionMisuseNoShorthand {
public:
  InvalidArgument(Variant action, std::string_view option, std::string_view argument) noexcept;
  const char* option() const noexcept;
  const char* argument() const noexcept;
private:
  SafeString m_option, m_argument;
};
class MultipleFormats : public ActionMisuseAll {
  using ActionMisuseAll::ActionMisuseAll;
};
struct NoAction {};

class UsageError : public std::exception {
public:
  using Variant = std::variant<GeneralActionMisuse, MissingArgument, InvalidArgument,
                                   MultipleFormats, NoAction>;
  static_assert(std::is_nothrow_move_constructible<Variant>::value);
  UsageError(Variant variant) noexcept;
  const Variant& variant() const noexcept;
  const char* what() const noexcept override;
private:
  Variant m_variant;
  SafeString m_message;
};

template<typename T>
struct GetFailedAction
{ };
template<>
struct GetFailedAction<AddAction> {
  using type = FailedAdd;
};
template<>
struct GetFailedAction<RmAction> {
  using type = FailedRm;
};
template<>
struct GetFailedAction<FindAction> {
  using type = FailedFind;
};
template<>
struct GetFailedAction<TagsAction> {
  using type = FailedTags;
};
template<>
struct GetFailedAction<EditAction> {
  using type = FailedEdit;
};
template<>
struct GetFailedAction<ShorthandFindAction> {
  using type = FailedShorthandFind;
};
template<typename ErrorType, typename CallerAction, typename ... Args>
UsageError actionUsageError(const CallerAction*, Args&& ... args) {
  return UsageError(ErrorType(typename GetFailedAction<CallerAction>::type(), std::forward<Args>(args)...));
}
