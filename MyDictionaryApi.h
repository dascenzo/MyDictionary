#pragma once
#include "Entry.h"
#include "EntryFormat.h"
#include <optional>
#include <vector>
#include <string>

// Functions for interacting with the dictionary. The dictionary consists
// of a set of entries, where the "word" field is the unique key.

void MD_createEntry(const Entry& entry);
auto MD_readEntry(std::string_view word, EntryFormat) -> std::optional<Entry>;
auto MD_readEntries(EntryFormat) -> std::vector<Entry>;
auto MD_readEntriesWithTags(const Entry::TagList&, EntryFormat) -> std::vector<Entry>;
auto MD_readEntriesWordMatch(std::string_view pattern, EntryFormat) -> std::vector<Entry>;
auto MD_readSimilarWords(std::string_view word) -> std::vector<std::string>;
auto MD_readTags() -> std::vector<std::string>;
auto MD_readSimilarTags(const std::vector<std::string>& tags) -> std::vector<std::string>;
// update and delete functions throw if their action cannot actually be performed
// (e.g. no such item to update/delete exists)
void MD_updateEntry(std::string_view word,
                    const std::optional<std::string>& newWord = std::nullopt,
                    const std::optional<std::string>& newDefinition = std::nullopt,
                    const std::vector<std::string>& newTags = std::vector<std::string>(),
                    const std::vector<std::string>& rmTags  = std::vector<std::string>());
void MD_updateTag(std::string_view old,std::string_view next);
void MD_deleteEntry(std::string_view word);
void MD_deleteTag(std::string_view tag);
