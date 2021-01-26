#include "Error.h"
#include <cstdio>

MissingArgument::MissingArgument(Variant action, std::string_view option) noexcept
  : ActionMisuseNoShorthand(std::move(action)), m_option(option) {
}
const char* MissingArgument::option() const noexcept {
  return m_option;
}
InvalidArgument::InvalidArgument(Variant action, std::string_view option, std::string_view argument) noexcept
  : ActionMisuseNoShorthand(std::move(action)), m_option(option), m_argument(argument) {
}
const char* InvalidArgument::option() const noexcept {
  return m_option;
}
const char* InvalidArgument::argument() const noexcept {
  return m_argument;
}
UsageError::UsageError(Variant variant) noexcept
  : m_variant(std::move(variant)) {
}
const char* UsageError::what() const noexcept {
  return "invalid usage";
}
const UsageError::Variant& UsageError::variant() const noexcept {
  return m_variant;
}
BadEnvVar::BadEnvVar(std::string_view badVariable) noexcept
  : m_badVariable(badVariable) {
}
const char* BadEnvVar::variable() const noexcept {
  return m_badVariable;
}
SetEnvFailed::SetEnvFailed(int errNo) noexcept
  : m_errNo(errNo) {
}
int SetEnvFailed::errNo() const noexcept {
  return m_errNo;
}
EnvironmentSetupError::EnvironmentSetupError(Variant variant) noexcept
  : m_variant(std::move(variant)) {
}
const EnvironmentSetupError::Variant& EnvironmentSetupError::variant() const noexcept {
  return m_variant;
}
const char* EnvironmentSetupError::what() const noexcept {
  return "environment setup error";
};
DataStateError::DataStateError(DataVariant data, ErrorType errorType) noexcept
  : m_data(std::move(data)), m_errorType(errorType) {
}
const DataStateError::DataVariant& DataStateError::data() const noexcept {
  return m_data;
}
DataStateError::ErrorType DataStateError::type() const noexcept {
  return m_errorType;
}
const char* DataStateError::what() const noexcept {
  return "data state error";
}
UpdateSpecificationError::UpdateSpecificationError(std::string_view word) noexcept
  : m_word(word) {
}
const char* UpdateSpecificationError::word() const noexcept {
  return m_word;
}
const char* UpdateSpecificationError::what() const noexcept {
  static_assert(std::is_same_v<decltype(m_word), SafeString>, "buffer size determination not valid");
  static std::array<char, SafeString::maxStringLength + 100 /* for context text */> buf;
  std::sprintf(buf.data(), "nothing specified to be updated for entry (word=%s)",
      static_cast<const char*>(m_word));
  return buf.data();
}
