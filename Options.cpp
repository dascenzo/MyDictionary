#include "Options.h"
#include "CommandLine.h"
#include "Error.h"
#include <type_traits>
#include <string>
#include <sstream>

// This file does some compile time validation to ensure the set of options have certain
// properties. Namely that a given string can match one and only one option.
// This validation depends on the options being in a certain format which is also statically confirmed.

#define CHECK_OPT(s, opt) ([&]() {\
  constexpr auto _idx = opt_indexOf((opt)); \
  static_assert(_idx != opt_npos); \
  if constexpr (_idx < opt_abbrevStartIdx) return ((s) == (opt)); \
  else return isPrefix<_idx>((s)); }())

static constexpr const char* opts[] = {
  // can't abbreviate
  "add", "rm", "find", "tags", "edit", "help", "-f", "-ff", "-fff", "-h",
  // can abbreviate
  "-word", "-def", "-nocheck", "-tag", "-all", "-pat", "-edit", "-rmtag"
};
static constexpr auto opt_count = std::extent_v<decltype(opts)>;

// static string equals
static constexpr bool static_strncmp(const char* s1, const char* s2, std::size_t n1 = -1, std::size_t n2 = -1) {
  const char* e1 = s1;
  for (std::size_t i = 0; i < n1 && *e1; ++i) {
    ++e1;
  }
  const char* e2 = s2;
  for (std::size_t i = 0; i < n2 && *e2; ++i) {
    ++e2;
  }
  for ( ; s1 != e1 && s2 != e2; ++s1, ++s2) {
    if (*s1 != *s2) {
      return false;
    }
  }
  return s1 == e1 && s2 == e2;
}

static constexpr std::size_t static_strlen(const char* s) {
  std::size_t len = 0;
  while (*s++) {
    ++len;
  }
  return len;
}
static constexpr std::size_t opt_npos = -1;
static constexpr std::size_t opt_indexOf(const char* s) {
  for (std::size_t i = 0; i < opt_count; ++i) {
    if (static_strncmp(opts[i], s)) {
      return i;
    }
  }
  return opt_npos;
}

static constexpr auto opt_abbrevStartIdx = opt_indexOf("-word");
static_assert(opt_abbrevStartIdx != opt_npos);

template<std::size_t Idx>
static bool isPrefix(std::string_view pre) {
  static_assert(Idx >= opt_abbrevStartIdx && Idx < opt_count);
  if (pre.size() < 2 || pre[0] != '-') {
    return false;
  }
  std::size_t i = 1;
  for ( ; i < pre.size() && opts[Idx][i]; ++i) {
    if (pre[i] != opts[Idx][i]) {
      break;
    }
  }
  return i == pre.size();
}


// return if set of all options and all prefixes of all abbreviatable options has no duplicate elements
// prefixes start after dash
static constexpr bool opt_validate_unique() {
  for (std::size_t i = 0; i < opt_count - 1; ++i) {
    auto m = (i >= opt_abbrevStartIdx) ? 2 : static_strlen(opts[i]);
    for ( ; m <= static_strlen(opts[i]); ++m) {
      for (std::size_t j = i + 1; j < opt_count; ++j) {
        auto n = (j >= opt_abbrevStartIdx) ? 2 : static_strlen(opts[j]);
        for ( ; n <= static_strlen(opts[j]); ++n) {
	  if (static_strncmp(opts[i], opts[j], m, n)) {
	    return false;
	  }
        }
      }
    }
  }
  return true;
}
// return if all non-abbreviatable options are at letter 1 letter long and
// abbreviatable are at least two and start with a dash
static constexpr bool opt_validate_format() {
  for (std::size_t i = 0; i < opt_count; ++i) {
    if (i < opt_abbrevStartIdx) {
      if (static_strlen(opts[i]) < 1) {
	return false;
      }
    }
    else {
      if (static_strlen(opts[i]) < 2 || opts[i][0] != '-') {
        return false;
      }
    }
  }
  return true;
}

// validate the options
static_assert(opt_validate_format() && opt_validate_unique());

bool matches(std::string_view s, Opt opt) {
  switch (opt) {
  case Opt::add: return CHECK_OPT(s,"add");
  case Opt::rm: return CHECK_OPT(s,"rm");
  case Opt::find: return CHECK_OPT(s,"find");
  case Opt::tags: return CHECK_OPT(s,"tags");
  case Opt::edit: return CHECK_OPT(s,"edit");
  case Opt::help: return CHECK_OPT(s,"help");
  case Opt::_word: return CHECK_OPT(s,"-word");
  case Opt::_def: return CHECK_OPT(s,"-def");
  case Opt::_nocheck: return CHECK_OPT(s,"-nocheck");
  case Opt::_tag: return CHECK_OPT(s,"-tag");
  case Opt::_all: return CHECK_OPT(s,"-all");
  case Opt::_pat: return CHECK_OPT(s,"-pat");
  case Opt::_edit: return CHECK_OPT(s,"-edit");
  case Opt::_rmtag: return CHECK_OPT(s,"-rmtag");
  case Opt::_h: return CHECK_OPT(s,"-h");
  case Opt::_f: return CHECK_OPT(s,"-f");
  case Opt::_ff: return CHECK_OPT(s,"-ff");
  case Opt::_fff: return CHECK_OPT(s,"-fff");
  }
}
