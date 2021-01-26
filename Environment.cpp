#include "Environment.h"
#include "Error.h"
#include <cstring>
#include <cstdlib>
#include <map>
#include <string>
#include <type_traits>
#include <iostream>
#include <cerrno>

extern char **environ;
static constexpr auto prefix = "MYDICT_";
static constexpr auto prefixLength = std::char_traits<char>::length(prefix);

void setupEnvironment() {
  auto setEnv = std::map<std::string, std::string>();

  for (char** pp = environ; *pp; ++pp) {
    const auto str = *pp;

    if (std::strncmp(str, prefix, prefixLength) == 0) {
      const auto endIt = str + std::strlen(str);
      const auto equalsIt = std::find(str, endIt, '=');
      // for some reason no '=' (don't really expect this to ever happen)
      if (equalsIt == endIt) {
        throw EnvironmentSetupError(BadEnvVar(str));
      }
      const auto base = std::string(str + prefixLength, equalsIt);
      const auto value = std::string(equalsIt + 1, endIt);
      setEnv["PG" + base] = std::move(value);
    }
  }
  for (const auto& nameValuePair : setEnv) {
    if (setenv(nameValuePair.first.c_str(), nameValuePair.second.c_str(), true) != 0) {
      throw EnvironmentSetupError(SetEnvFailed(errno));
    }
  }
}
