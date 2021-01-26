#include "Entry.h"

// WORD
//    DEFINITON
// [TAG1, TAG2 ...]
// Created: ADSAFDA, Modified: ADFFS
std::ostream& operator<<(std::ostream& o, const Entry& entry) {
  o << entry.word << "\n";

  if (entry.definition != "") {
    o << "\t" << entry.definition << "\n";
  }
  const auto tagCount = entry.tags.size();

  if (tagCount != 0) {
    auto written = Entry::TagList::size_type(0);
    o << "[";
    for (const auto& tag : entry.tags) {
      o << tag;
      if (written++ != tagCount - 1) {
	o << ", ";
      }
    }
    o << "]\n";
  }
  if (entry.created != "" && entry.modified != "") {
    o << "Created: " << entry.created
      << ", Modified: " << entry.modified << "\n";
  }
  return o;
}
