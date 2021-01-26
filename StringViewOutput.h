#pragma once
#include <string_view>

namespace StringViewOutput {
  // convert string's size to precision argument for printf
  int size(std::string_view string) noexcept;

  // convert string to c string for printf
  const char* c_str(std::string_view string) noexcept;
}
// intended to be used like:
// printf("%.*s", C_STR(stringView));
#define C_STR(stringView) StringViewOutput::size(stringView), StringViewOutput::c_str(stringView)

