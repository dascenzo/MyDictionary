#pragma once
#include <string>
#include <array>
#include <string_view>

/**
 * Noexcept limited length string.
 */
class SafeString {
private:
  constexpr static char truncationString[] = "...";

public:
  SafeString() noexcept;
  SafeString(std::string_view string) noexcept;
  operator const char*() const noexcept;

  constexpr static std::size_t maxMessageLength = 100;
  constexpr static std::size_t maxStringLength =
      maxMessageLength + std::char_traits<char>::length(truncationString);
private:
  std::array<char, maxStringLength + 1 /*null terminator*/> m_buffer;
};

