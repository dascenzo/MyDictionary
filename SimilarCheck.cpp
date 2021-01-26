#include "SimilarCheck.h"
#include "MyDictionaryApi.h"
#include "Interactive.h"
#include <iostream>
#include <cctype>
#include <optional>
#include <utility>

static bool isWhiteSpace(char c) {
  return std::isspace(static_cast<unsigned char>(c));
}
// returns iterator pointing to token beginning and token length (TODO: c++20 return string_view)
// e.g. "  abc " returns first == itr to a, second = 3
// "  abc  efg " returns nullopt
static std::optional<std::pair<std::string::const_iterator, std::size_t>> singleToken(const std::string& str) {
  auto it = str.begin();
  while (it != str.end() && isWhiteSpace(*it)) {
    ++it;
  }
  if (it == str.end()) {
    return std::nullopt;
  }
  auto b = it++;
  while (it != str.end() && !isWhiteSpace(*it)) {
    ++it;
  }
  auto e = it;
  if (it == str.end()) {
    return std::pair<std::string::const_iterator, std::size_t>(b, e - b);
  }
  ++it;
  while (it != str.end() && isWhiteSpace(*it)) {
    ++it;
  }
  if (it != str.end()) {
    return std::nullopt;
  }
  return std::pair<std::string::const_iterator, std::size_t>(b, e - b);
}
static bool isYesAnswer(const std::string& answer) {
  const auto pair = singleToken(answer);
  return pair &&
      ((pair->second == 1 && (*pair->first == 'Y' || *pair->first == 'y'))
      || (pair->second == 3 && (*pair->first == 'Y' || *pair->first == 'y')
			    && (*(pair->first + 1) == 'E' || *(pair->first + 1) == 'e')
			    && (*(pair->first + 2) == 'S' || *(pair->first + 2) == 's')));
}
static bool isNoAnswer(const std::string& answer) {
  const auto pair = singleToken(answer);
  return pair &&
      ((pair->second == 1 && (*pair->first == 'N' || *pair->first == 'n'))
      || (pair->second == 2 && (*pair->first == 'N' || *pair->first == 'n')
			    && (*(pair->first + 1) == 'O' || *(pair->first + 1) == 'o')));
}
//static void printIosState(std::ostream& o) {
//  auto flags = o.rdstate();
//  o << '\n'
//    << (bool)(flags & std::ios::eofbit)
//    << (bool)(flags & std::ios::failbit)
//    << (bool)(flags & std::ios::badbit) << std::endl;
//}
static std::optional<bool> readYesNo() {
  std::optional<bool> response;
  bool userNeedsToReanswer;
  std::string line;
  do {
    userNeedsToReanswer = false;
    std::getline(std::cin, line);
    if (std::cin) {
      if (isYesAnswer(line)) {
	response = true;
      }
      else if (isNoAnswer(line)) {
        response = false;
      }
      else if (!std::cin.eof()) {
        std::cout << "? [yes/no] ";
        userNeedsToReanswer = true;
      }
    }
  } while (userNeedsToReanswer);

  return response;
}
// precondition: |list| > 0
static void printList(std::ostream& o, const std::vector<std::string>& list) {
  o << list.front();
  for (auto it = list.begin() + 1; it != list.end(); ++it) {
    o << ", " << *it;
  }
}
// precondition: |tags| > 0 || |words| > 0
static void promptSimilarItemWarning(const std::vector<std::string>& words,
                                     const std::vector<std::string>& tags) {
  std::cout << "warning: ";
  if (words.size() > 1 || tags.size() > 1 || (words.size() > 0 && tags.size() > 0)) {
    std::cout << "some similar items already exist:\n";
  }
  else {
    std::cout << "a similar item already exists:\n";
  }
  if (words.size() > 0) {
    std::cout << "word" << (words.size() > 1 ? "s" : "") << ": ";
    printList(std::cout, words);
    std::cout << std::endl;
  }
  if (tags.size() > 0) {
    std::cout << "tag" << (tags.size() > 1 ? "s" : "") << ": ";
    printList(std::cout, tags);
    std::cout << std::endl;
  }
  std::cout << "proceed? [yes/no] ";
}
// copy the behavior of the prompt for rm:
// $ yes^D yields yes, $ no^D yields no
// but $ ^D or $ abc ^D => no
SimilarCheckResult
similarCheck(const std::string& word, const std::vector<std::string>& tags) {
  if (isInteractive(stdin) && isInteractive(stdout)) {
    const auto similarWords = MD_readSimilarWords(word);
    const auto similarTags = MD_readSimilarTags(tags);
    if (similarWords.size() > 0 || similarTags.size() > 0) {

      promptSimilarItemWarning(similarWords, similarTags);
      const auto response = readYesNo();
      if (response /*user responded at all*/ && *response == true/*yes*/) {
	return SimilarCheckResult::Continue;
      }
      else {
        return SimilarCheckResult::Abort;
      }
    }
    else {
      return SimilarCheckResult::Continue;
    }
  }
  else {
    return SimilarCheckResult::Continue;
  }
}
