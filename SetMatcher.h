#pragma once
#include <string>
#include <algorithm>
#include <pqxx/pqxx>

template<typename Set>
class SetMatcher {
public:
  SetMatcher(const pqxx::dbtransaction& txn, const Set& set, std::string pattern, std::string replaceToken = "VALUE")
    : m_txn(txn), m_set(set), m_pattern(std::move(pattern)), m_replaceToken(std::move(replaceToken)) {

  }
  const std::string condition() const {
    return std::accumulate(m_set.begin(), m_set.end(), std::string("1 = 0"), // false
      [this](const std::string& left, const std::string& right) {
        return left + " OR " + substitute(right);
      });
  }
private:
  // returns m_pattern with all occurrences of m_replaceToken replaced with value.
  std::string substitute(const std::string& value) const {
    auto pos = m_pattern.find(m_replaceToken);
    if (pos != std::string::npos) {
      auto pattern = m_pattern;
      do {
        const auto replacement = m_txn.quote(value);
        pattern.replace(pos, m_replaceToken.size(), replacement);
        pos = pattern.find(m_replaceToken, pos + replacement.size());
      } while (pos != std::string::npos);
      return pattern;
    }
    return m_pattern;
  }
  const pqxx::dbtransaction& m_txn;
  const Set& m_set;
  std::string m_pattern,
              m_replaceToken;
};
