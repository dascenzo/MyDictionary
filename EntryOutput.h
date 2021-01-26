#pragma once
#include "Entry.h"
#include "EntryFormat.h"
#include <ostream>

struct Formatted {
  Formatted(const Entry&, EntryFormat);
  const Entry& entry;
  EntryFormat format;
};


std::ostream& operator<<(std::ostream&, const Formatted&);
