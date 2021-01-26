#pragma once
#include <set>
#include <string>
#include <ostream>

struct Entry {
  using TagList = std::set<std::string>;
  std::string word,
              definition,
              created,
              modified;
  TagList tags;
};

std::ostream& operator<<(std::ostream&, const Entry&);
