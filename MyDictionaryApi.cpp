#include "MyDictionaryApi.h"
#include "EntryQuery.h"
#include "SetMatcher.h"
#include "Error.h"
#include <iostream>
#include <algorithm>
#include <pqxx/pqxx>

// XXX: The pqxx::transaction constructor sends SQL BEGIN [...], commit()
// sends COMMIT, destroying without committing sends ROLLBACK.

// Use Repeatable Read isolation and lock tables before first "real" SQL command
// to ensure the transaction 1) is serialized, 2) sees the latest data state.

#define LOCK_TABLES "LOCK TABLE entries, tags, word_tag, word_modified"

// TODO: perhaps use less restrictive lock mode for reads
#define R_TXN(CONN_VAR, TXN_VAR) \
  pqxx::connection CONN_VAR{}; \
  pqxx::transaction<pqxx::repeatable_read, pqxx::write_policy::read_only> TXN_VAR{CONN_VAR}; \
  TXN_VAR.exec0(LOCK_TABLES)

#define RW_TXN(CONN_VAR, TXN_VAR) \
  pqxx::connection CONN_VAR{}; \
  pqxx::transaction<pqxx::repeatable_read, pqxx::write_policy::read_write> TXN_VAR{CONN_VAR}; \
  TXN_VAR.exec0(LOCK_TABLES)

// TODO: what's the best similarity number? use incorporate phonetic checking?
static const std::string similarityThreshold = "0.65";

void MD_createEntry(const Entry& entry) {
  RW_TXN(c, t);
  std::string q;
  q =
      "INSERT INTO entries (word, definition)"
      "    VALUES (" + t.quote(entry.word) + ", " + t.quote(entry.definition) + ")";
  try {
    t.exec0(q);
  }
  catch (const pqxx::unique_violation& ) {
    throw DataStateError(DataStateError::WordData(entry.word),
                             DataStateError::ALREADY_PRESENT);
  }
  for (const auto& tag : entry.tags) {
    q =
        "INSERT INTO tags (tag)"
        "    VALUES (" + t.quote(tag) + ")"
        "    ON CONFLICT DO NOTHING";
    t.exec0(q);
    q =
        "INSERT INTO word_tag (word, tag)"
        "    VALUES (" + t.quote(entry.word) + ", " + t.quote(tag) + ")";
    try {
      t.exec0(q);
    }
    // the word and tag must exist so foreign_key_violation shouldn't be possible.
    // if the tag was listed twice in the tag list passed in, there'll be unique_violation
    // (could happen bc tag list is case sensitive whereas database tag is not)
    catch (const pqxx::unique_violation& e) {
      throw DataStateError(DataStateError::WordTagData(entry.word, tag),
			       DataStateError::ALREADY_PRESENT);
    }
  }
  q =
      "INSERT INTO word_modified (word)"
      "    VALUES (" + t.quote(entry.word) + ")";
  t.exec0(q);

  t.commit();
}
std::optional<Entry> MD_readEntry(std::string_view word, EntryFormat format) {
  R_TXN(c, t);
  EntryQuery eq(format);
  auto q =
      "SELECT " + eq.select() +
      "    FROM entries LEFT OUTER JOIN word_tag"
      "    USING (word)"
      "    WHERE word = " + t.quote(word);
  const auto result = t.exec(q); t.commit();
  const auto entries = eq.extract(result);
  if (entries.size() == 0) {
    return std::nullopt;
  }
  else {
    return entries.front(); // entries.size() == 1
  }
}
std::vector<Entry> MD_readEntries(EntryFormat format) {
  R_TXN(c, t);
  EntryQuery eq(format);
  auto q =
      "SELECT " + eq.select() +
      "    FROM entries LEFT OUTER JOIN word_tag"
      "    USING (word)"
      "    ORDER BY word";
  const auto result = t.exec(q); t.commit();
  return eq.extract(result);
}
std::vector<Entry> MD_readEntriesWithTags(const Entry::TagList& tags, EntryFormat format) {
  R_TXN(c, t);
  EntryQuery eq(format);
  SetMatcher sm(t, tags, "tag = TAG", "TAG");
  const auto q =
      "SELECT " + eq.select() +
      "    FROM entries LEFT OUTER JOIN word_tag wt"
      "    USING (word)"
      "    WHERE word IN"
      "        (SELECT word FROM word_tag"
      "            WHERE " + sm.condition() + ")"
      "    ORDER BY word";
  const auto result = t.exec(q); t.commit();
  return eq.extract(result);
}
std::vector<Entry> MD_readEntriesWordMatch(std::string_view pattern, EntryFormat format) {
  R_TXN(c, t);
  EntryQuery eq(format);
  const auto q =
      "SELECT " + eq.select() +
      "    FROM entries LEFT OUTER JOIN word_tag"
      "    USING (word)"
      "    WHERE word ~ " + t.quote(pattern) +
      "    ORDER BY word";
  const auto result = t.exec(q); t.commit();
  return eq.extract(result);
}
std::vector<std::string> MD_readSimilarWords(std::string_view word) {
  R_TXN(c, t);
  std::vector<std::string> resultWords;
  auto q =
      "SELECT word FROM entries"
      "    WHERE (word <> " + t.quote(word) + ")"
      "    AND (similarity(word, " + t.quote(word) + ") > " + similarityThreshold + ")";
  const auto result = t.exec(q); t.commit();
  resultWords.reserve(result.size());
  for (const auto& row : result) {
    resultWords.push_back(row[0].c_str());
  }
  return resultWords;
}
std::vector<std::string> MD_readTags() {
  R_TXN(c, t);
  std::vector<std::string> tags;
  auto q =
      "SELECT tag FROM tags ORDER by tag";
  const auto result = t.exec(q); t.commit();
  tags.reserve(result.size());
  for (const auto& row : result) {
    tags.push_back(row[0].c_str());
  }
  return tags;
}
std::vector<std::string> MD_readSimilarTags(const std::vector<std::string>& tags) {
  R_TXN(c, t);
  SetMatcher eq(t, tags, "tag = TAG", "TAG");
  SetMatcher sm(t, tags, "similarity(tag, TAG) > " + similarityThreshold, "TAG");
  std::vector<std::string> resultTags;
  auto q =
      "SELECT tag FROM tags WHERE"
      "    NOT(" + eq.condition() + ") AND (" + sm.condition() + ")";
  const auto result = t.exec(q); t.commit();
  resultTags.reserve(result.size());
  for (const auto& row : result) {
    resultTags.push_back(row[0].c_str());
  }
  return resultTags;
}
void MD_updateEntry(std::string_view word, const std::optional<std::string>& newWord,
                    const std::optional<std::string>& newDefinition,
                    const std::vector<std::string>& newTags,
                    const std::vector<std::string>& rmTags) {
  if (!newWord && !newDefinition && newTags.size() == 0 && rmTags.size() == 0) {
    throw UpdateSpecificationError(word);
  }
  RW_TXN(c, t);
  // XXX order important, don't change the name till the end so the previous work
  for (const auto& tag : newTags) {
    std::string q;
    q = "INSERT INTO tags (tag) VALUES (" + t.quote(tag) + ")"
	"    ON CONFLICT DO NOTHING";
    t.exec0(q);

    q = "INSERT INTO word_tag (word, tag)"
	"    VALUES (" + t.quote(word) + ", " + t.quote(tag) + ")";
    try {
      t.exec0(q);
    }
    catch (const pqxx::unique_violation& ) {
      // word/tag combo already exists
      throw DataStateError(DataStateError::WordTagData(word, tag),
			       DataStateError::ALREADY_PRESENT);
    }
    catch (const pqxx::foreign_key_violation& ) {
      // word doesn't exist (tag must exist since it was just added)
      throw DataStateError(DataStateError::WordData(word),
			       DataStateError::MISSING);
    }
  }
  for (const auto& tag : rmTags) {
    std::string q;
    q = "DELETE FROM word_tag"
        "    WHERE word = " + t.quote(word) + " AND tag = " + t.quote(tag) +
        "    RETURNING word";
    try {
      t.exec1(q);
    }
    catch (const pqxx::unexpected_rows& ) {
      throw DataStateError(DataStateError::WordTagData(word, tag),
			       DataStateError::MISSING);
    }
  }
  if (newWord || newDefinition) {
    std::string q;
    q += "UPDATE entries SET ";
    if (newWord) {
      q += "word = " + t.quote(*newWord);
    }
    if (newWord && newDefinition) {
      q += ", ";
    }
    if (newDefinition) {
      q += "definition = " + t.quote(*newDefinition);
    }
    q += "  WHERE word = " + t.quote(word);
    q += "  RETURNING word";
    try {
      t.exec1(q);
    }
    catch (const pqxx::unexpected_rows& ) {
      throw DataStateError(DataStateError::WordData(word),
			       DataStateError::MISSING);
    }
  }
  t.commit();
}
void MD_updateTag(std::string_view old, std::string_view next) {
  RW_TXN(c, t);
  auto q =
      "UPDATE tags SET tag = " + t.quote(next) + " WHERE tag = " + t.quote(old) +
      "    RETURNING tag";
  try {
    t.exec1(q);
  }
  catch (const pqxx::unexpected_rows& ) {
      throw DataStateError(DataStateError::TagData(old),
			       DataStateError::MISSING);
  }
  t.commit();
}
void MD_deleteEntry(std::string_view word) {
  RW_TXN(c, t);
  auto q =
      "DELETE FROM entries"
      "    WHERE word = " + t.quote(word) +
      "    RETURNING word";
  try {
    t.exec1(q);
  }
  catch (const pqxx::unexpected_rows& ) {
      throw DataStateError(DataStateError::WordData(word),
			       DataStateError::MISSING);
  }
  t.commit();
}
void MD_deleteTag(std::string_view tag) {
  RW_TXN(c, t);
  auto q =
      "DELETE FROM tags"
      "    WHERE tag = " + t.quote(tag) +
      "    RETURNING tag";
  try {
    t.exec1(q);
  }
  catch (const pqxx::unexpected_rows& ) {
      throw DataStateError(DataStateError::TagData(tag),
			       DataStateError::MISSING);
  }
  t.commit();
}
