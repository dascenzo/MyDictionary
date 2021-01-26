#include "StringViewOutput.h"
#include <limits>

int StringViewOutput::size(std::string_view string) noexcept {
  if (string.size() > std::numeric_limits<int>::max()) {
    return std::numeric_limits<int>::max();
  }
  return static_cast<int>(string.size());
}
const char* StringViewOutput::c_str(std::string_view string) noexcept {
  return string.data();
}
