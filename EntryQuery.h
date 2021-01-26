#pragma once
#include "EntryFormat.h"
#include "Entry.h"
#include <string>
#include <vector>
#include <pqxx/pqxx>

class EntryQuery {
public:
  EntryQuery(EntryFormat);
  std::string select() const;
  std::vector<Entry> extract(const pqxx::result& result) const;
private:
  EntryFormat m_format;
};
