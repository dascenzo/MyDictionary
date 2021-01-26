#include "SafeString.h"
#include <cstdio>
#include <cstdarg>

SafeString::SafeString() noexcept {
  m_buffer.front() = '\0';
}
SafeString::SafeString(std::string_view string) noexcept {
  if (string.size() > maxMessageLength) {
    std::copy(string.begin(), string.begin() + maxMessageLength, m_buffer.begin());
    std::strcpy(m_buffer.data() + maxMessageLength, truncationString);
  }
  else {
    std::copy(string.begin(), string.end(), m_buffer.begin());
    *(m_buffer.begin() + string.size()) = '\0';
  }
}
SafeString::operator const char*() const noexcept {
  return m_buffer.data();
}
