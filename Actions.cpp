#include "Actions.h"
#include "CommandLine.h"
#include "MyDictionaryApi.h"
#include "Error.h"
#include "Entry.h"
#include "EntryOutput.h"
#include "SimilarCheck.h"
#include "CommandLineParsing.h"
#include "Options.h"
#include "StringViewOutput.h"
#include <optional>
#include <utility>
#include <iostream>

void AddAction::execute(const CommandLine& commandLine) {
  std::optional<std::string> word, definition;
  bool nocheck = false;
  std::vector<std::string> tag;

  for (int i = 1; i < commandLine.args.size(); ) {

    if (i == 1 && matches(commandLine.args[i], Opt::_h)) {
      std::fprintf(stdout, ADD_USAGE "\n", C_STR(commandLine.baseProgramName));
      return;
    }
    else if (matches(commandLine.args[i], Opt::_word) && !word) {
      ++i;
      word = parseCommandLineText(this, i, commandLine);
    }
    else if (matches(commandLine.args[i], Opt::_def) && !definition) {
      ++i;
      definition = parseCommandLineText(this, i, commandLine);
    }
    else if (matches(commandLine.args[i], Opt::_nocheck) && !nocheck) {
      ++i;
      nocheck = true;
    }
    else if (matches(commandLine.args[i], Opt::_tag)) {
      ++i;
      tag.push_back(parseCommandLineText(this, i, commandLine));
    }
    else {
      throw actionUsageError<GeneralActionMisuse>(this);
    }
  }
  if (word && definition) {
    if (!nocheck) {
      if (similarCheck(*word, tag) == SimilarCheckResult::Abort) {
        return;
      }
    }
    Entry newEntry;
    newEntry.word = std::move(*word);
    newEntry.definition = std::move(*definition);
    newEntry.tags.insert(std::make_move_iterator(tag.begin()),
                         std::make_move_iterator(tag.end()));
    MD_createEntry(newEntry);
  }
  else {
    throw actionUsageError<GeneralActionMisuse>(this);
  }
}
void RmAction::execute(const CommandLine& commandLine) {
  std::optional<std::string> word, tag;

  for (int i = 1; i < commandLine.args.size(); ) {
    if (i == 1 && matches(commandLine.args[i], Opt::_h)) {
      std::fprintf(stdout, RM_USAGE "\n", C_STR(commandLine.baseProgramName));
      return;
    }
    else if (matches(commandLine.args[i], Opt::_word) && !word) {
      ++i;
      word = parseCommandLineText(this, i, commandLine);
    }
    else if (matches(commandLine.args[i], Opt::_tag) && !tag) {
      ++i;
      tag = parseCommandLineText(this, i, commandLine);
    }
    else {
      throw actionUsageError<GeneralActionMisuse>(this);
    }
  }
  if (word && !tag) {
    MD_deleteEntry(*word);
  }
  else if (!word && tag) {
    MD_deleteTag(*tag);
  }
  else {
    throw actionUsageError<GeneralActionMisuse>(this);
  }
}
void FindAction::execute(const CommandLine& commandLine) {
  bool all = false;
  std::optional<std::string> word, pattern;
  std::vector<std::string> tag;
  bool f = false, ff = false, fff = false;

  for (int i = 1; i < commandLine.args.size(); ) {
    if (i == 1 && matches(commandLine.args[i], Opt::_h)) {
      std::fprintf(stdout, FIND_USAGE "\n", C_STR(commandLine.baseProgramName));
      return;
    }
    else if (matches(commandLine.args[i], Opt::_all) && !all) {
      ++i;
      all = true;
    }
    else if (matches(commandLine.args[i], Opt::_word) && !word) {
      ++i;
      word = parseCommandLineText(this, i, commandLine);
    }
    else if (matches(commandLine.args[i], Opt::_pat) && !pattern) {
      ++i;
      pattern = parseCommandLineText(this, i, commandLine);
    }
    else if (matches(commandLine.args[i], Opt::_tag)) {
      ++i;
      tag.push_back(parseCommandLineText(this, i, commandLine));
    }
    else if (matches(commandLine.args[i], Opt::_f) && !f) {
      ++i;
      f = true;
    }
    else if (matches(commandLine.args[i], Opt::_ff) && !ff) {
      ++i;
      ff = true;
    }
    else if (matches(commandLine.args[i], Opt::_fff) && !fff) {
      ++i;
      fff = true;
    }
    else {
      throw actionUsageError<GeneralActionMisuse>(this);
    }
  }
  const auto format = flag2format(this, f, ff, fff);

  if (all && !word && !pattern && tag.size() == 0) {
    for (const auto& entry : MD_readEntries(format)) {
      std::cout << Formatted(entry, format);
    }
  }
  else if (!all && word && !pattern && tag.size() == 0) {
    const auto entry = MD_readEntry(*word, format);
    if (entry) {
      std::cout << Formatted(*entry, format);
    }
  }
  else if (!all && !word && pattern && tag.size() == 0) {
    for (const auto& entry : MD_readEntriesWordMatch(*pattern, format)) {
      std::cout << Formatted(entry, format);
    }
  }
  else if (!all && !word && !pattern && tag.size() != 0) {
    const auto entries = MD_readEntriesWithTags(Entry::TagList(
         std::make_move_iterator(tag.begin()),
         std::make_move_iterator(tag.end())), format);
    for (const auto& entry : entries) {
      std::cout << Formatted(entry, format);
    }
  }
  else {
    throw actionUsageError<GeneralActionMisuse>(this);
  }
}
void TagsAction::execute(const CommandLine& commandLine) {
  if (commandLine.args.size() > 1 && matches(commandLine.args[1], Opt::_h)) {
      std::fprintf(stdout, TAGS_USAGE "\n", C_STR(commandLine.baseProgramName));
    return;
  }
  else if (commandLine.args.size() == 1) {
    const auto tags = MD_readTags();
    for (const auto& tag : tags) {
      std::cout << tag << std::endl;
    }
  }
  else {
    throw actionUsageError<GeneralActionMisuse>(this);
  }
}
void EditAction::execute(const CommandLine& commandLine) {
  std::optional<std::string> word, edit, definition;
  std::vector<std::string> tag, rmtag;

  int i = 1;

  for ( ; i < commandLine.args.size(); ) {
    if (i == 1 && matches(commandLine.args[i], Opt::_h)) {
      std::fprintf(stdout, EDIT_USAGE "\n", C_STR(commandLine.baseProgramName));
      return;
    }
    else if (matches(commandLine.args[i], Opt::_word) && !word) {
      ++i;
      word = parseCommandLineText(this, i, commandLine);
    }
    else if (matches(commandLine.args[i], Opt::_edit) && !edit) {
      ++i;
      edit = parseCommandLineText(this, i, commandLine);
    }
    else if (matches(commandLine.args[i], Opt::_def) && !definition) {
      ++i;
      definition = parseCommandLineText(this, i, commandLine);
    }
    else if (matches(commandLine.args[i], Opt::_tag)) {
      ++i;
      tag.push_back(parseCommandLineText(this, i, commandLine));
    }
    else if (matches(commandLine.args[i], Opt::_rmtag)) {
      ++i;
      rmtag.push_back(parseCommandLineText(this, i, commandLine));
    }
    else {
      throw actionUsageError<GeneralActionMisuse>(this);
    }
  }
  if (!word && edit && !definition && tag.size() == 1 && rmtag.size() == 0) {
    MD_updateTag(tag.front(), *edit);
  }
  else if (word && (edit || definition || tag.size() > 0 || rmtag.size() > 0)) {
    MD_updateEntry(*word, edit, definition, tag, rmtag);
  }
  else {
    throw actionUsageError<GeneralActionMisuse>(this);
  }
}
// already showing help so just ignore any extra/wrong arguments
void HelpAction::execute(const CommandLine& commandLine) {
  if (commandLine.args.size() > 1) {
    if (matches(commandLine.args[1], Opt::add)) {
      std::fprintf(stdout, ADD_USAGE "\n", C_STR(commandLine.baseProgramName));
    }
    else if (matches(commandLine.args[1], Opt::rm)) {
      std::fprintf(stdout, RM_USAGE "\n", C_STR(commandLine.baseProgramName));
    }
    else if (matches(commandLine.args[1], Opt::find)) {
      std::fprintf(stdout, FIND_USAGE "\n", C_STR(commandLine.baseProgramName));
    }
    else if (matches(commandLine.args[1], Opt::tags)) {
      std::fprintf(stdout, TAGS_USAGE "\n", C_STR(commandLine.baseProgramName));
    }
    else if (matches(commandLine.args[1], Opt::edit)) {
      std::fprintf(stdout, EDIT_USAGE "\n", C_STR(commandLine.baseProgramName));
    }
    else if (matches(commandLine.args[1], Opt::help)
             || matches(commandLine.args[1], Opt::_h)) {
      std::fprintf(stdout, HELP_USAGE "\n", C_STR(commandLine.baseProgramName));
    }
    else {
      std::fprintf(stdout, USAGE "\n\n", C_STR(commandLine.baseProgramName));
    }
  }
  else {
    std::fprintf(stdout, USAGE "\n\n", C_STR(commandLine.baseProgramName));
  }
}
void ShorthandFindAction::execute(const CommandLine& commandLine) {
  bool f = false, ff = false, fff = false;

  int i = 0;

  const auto word = tryParseCommandLineText(i, commandLine);
  if (!word) {
    throw actionUsageError<GeneralActionMisuse>(this);
  }
  if (i < commandLine.args.size()) {
    if (matches(commandLine.args[i], Opt::_f)) {
      f = true;
    } else if (matches(commandLine.args[i], Opt::_ff)) {
      ff = true;
    } else if (matches(commandLine.args[i], Opt::_fff)) {
      fff = true;
    }
    else {
      throw actionUsageError<GeneralActionMisuse>(this);
    }
    ++i;
    if (i < commandLine.args.size()) {
      throw actionUsageError<GeneralActionMisuse>(this);
    }
  }
  const auto format = flag2format(this, f, ff, fff);
  const auto entry = MD_readEntry(*word, format);
  if (entry) {
    std::cout << Formatted(*entry, format);
  }
}
