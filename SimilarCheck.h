#pragma once
#include <vector>
#include <string>

enum class SimilarCheckResult { Continue, Abort };

SimilarCheckResult
similarCheck(const std::string& word, const std::vector<std::string>& tags);
