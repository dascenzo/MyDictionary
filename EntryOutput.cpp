#include "EntryOutput.h"
#include "Interactive.h"
#include <sstream>

static std::string leadingPad(const std::string& text) {
  std::istringstream stream(text);
  std::string line, result;
  std::size_t count = 0;
  while (std::getline(stream, line)) {
    ++count;
    if (count > 1) {
      result += "\n";
    }
    result += ("    " + line);
  }
  return result;
}
static std::string bold(const std::string& text) {
  if (isInteractive(stdout)) {
    return "\e[1m" + text + "\e[0m";
  }
  return text;
}
static std::string boldWord(const Formatted& formatted) {
  if (formatted.format != FORMAT_BRIEF) {
    return bold(formatted.entry.word);
  }
  else {
    return formatted.entry.word;
  }
}

// FMT~WORD~FMT
// tags: TAG1, TAG2 ...
// created (modified)
//
//     DEFINITON
//
static std::ostream& doFormat(std::ostream& o, const Formatted& formatted) {
  o << boldWord(formatted);

  if (formatted.format == FORMAT_ALL) {
    const auto tagCount = formatted.entry.tags.size();
    auto written = Entry::TagList::size_type(0);
    o << "\ntags: ";
    for (const auto& tag : formatted.entry.tags) {
      o << tag;
      if (written++ != tagCount - 1) {
        o << ", ";
      }
    }
    o << "\n" << formatted.entry.created;
    if (!formatted.entry.modified.empty()) {
      o << " (" << formatted.entry.modified << ")";
    }
  }
  switch (formatted.format) {
  case FORMAT_ALL:
    o << "\n\n" << leadingPad(formatted.entry.definition) << "\n\n";
    break;
  case FORMAT_MODEST:
    o << "\n" << leadingPad(formatted.entry.definition) << "\n";
    break;
  case FORMAT_BRIEF:
    o << "\n";
    break;
  }
  return o;
}
Formatted::Formatted(const Entry& entry, EntryFormat format)
  : entry(entry), format(format) {
}

std::ostream& operator<<(std::ostream& o, const Formatted& formatted) {
  return doFormat(o, formatted);
}
