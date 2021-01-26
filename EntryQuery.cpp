#include "EntryQuery.h"
#include <map>

#define DATE_FORMAT "'Mon DD HH24:MI \'\'YY'"

EntryQuery::EntryQuery(EntryFormat format)
  : m_format(format) {
}
std::string EntryQuery::select() const {
  switch (m_format) {
  case FORMAT_ALL:
    return "word, definition, "
           "to_char(created," DATE_FORMAT "), "
           "tag";
  case FORMAT_MODEST:
    return "word, definition, tag";
  case FORMAT_BRIEF:
    return "word";
  }
}
std::vector<Entry> EntryQuery::extract(const pqxx::result& result) const {
  std::vector<Entry> entries;
  std::map<std::string/*word*/, std::size_t> seen;
  for (const auto& row : result) {
    std::size_t idx;
    const std::string word = row[0].c_str();
    auto it = seen.find(word);
    if (it == seen.end()) {
      idx = entries.size();
      seen[word] = idx;
      entries.push_back(Entry());
      switch (m_format) {
      case FORMAT_ALL:
        entries[idx].created = row[2].c_str();
      case FORMAT_MODEST:
        entries[idx].definition = row[1].c_str();
      case FORMAT_BRIEF:
        entries[idx].word = word;
      }
    }
    else {
      idx = it->second;
    }
    std::optional<std::size_t> tagIdx;
    if (m_format == FORMAT_ALL && !row[3].is_null()) {
      tagIdx = 3;
    }
    else if (m_format == FORMAT_MODEST && !row[2].is_null()) {
      tagIdx = 2;
    }
    if (tagIdx) {
      entries[idx].tags.insert(row[*tagIdx].c_str());
    }
  }
  return entries;
}
