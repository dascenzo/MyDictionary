#pragma once
#include <string_view>
#include <string>

/**
 * Program command line options
 */

// represents command line options e.g. "add", "rm", ... "-word", "-def", etc.
enum class Opt {
  add, rm, find, tags, edit, help,
  _word, _def, _nocheck, _tag, _all, _pat, _edit, _rmtag, _h, _f, _ff, _fff
};

/** returns whether s matches opt.
 * if s matches, then it will not match any other Opt enumerator.
 */
bool matches(std::string_view s, Opt opt);

#define ADD_USAGE \
"`add' usage: %.*s add -word \"...\" -def \"...\" [-nocheck] [-tag \"...\"] ..."

#define RM_USAGE \
"`rm' usage: %.*s rm -word \"...\" | -tag \"...\""

#define FIND_USAGE \
"`find' usage: %.*s find (-all | -word \"...\" | -pat \"...\" | -tag \"...\" ...) FORMAT"

#define TAGS_USAGE \
"`tags' usage: %.*s tags"

#define EDIT_USAGE \
"`edit' usage: %.*s edit -word \"...\" ([-edit \"...\"] [-def \"...\"] [-tag \"...\" ...] [-rmtag \"...\" ...])\n" \
"    | -tag \"...\" -edit \"...\""

#define HELP_USAGE \
"`help' usage: %.*s (help | -h) [add | rm | find | tags | edit | help]\n" \
"(add | rm | find | tags | edit | help) -h"

// a d e f n p r t w
#define USAGE \
"usage: %.*s ARGS\n" \
"  Flags (-xyz) may be truncated, and given in any order. To\n" \
"  begin \"...\" with a - character, pass -- first: -- -something.\n\n" \
"  ARGS\n" \
"      add  -word \"...\" -def \"...\" [-nocheck] [-tag \"...\"] ...\n" \
"          Add the word with the given definition and tags. If -nocheck,\n" \
"          don't prompt when similar words or tags already exist.\n\n" \
"      rm   -word \"...\" | -tag \"...\"\n" \
"          Delete the given word or tag.\n\n" \
"      find (-all | -word \"...\" | -pat \"...\" | -tag \"...\" ...) FORMAT\n" \
"          List all words, or words matching a pattern, or words having tags.\n\n" \
"      tags\n" \
"          List all tags.\n\n" \
"      edit -word \"...\" ([-edit \"...\"] [-def \"...\"] [-tag \"...\" ...] [-rmtag \"...\" ...])\n" \
"          | -tag \"...\" -edit \"...\"\n" \
"          Update a word, definition, add tags, or remove tags. Or update a tag.\n\n" \
"      \"...\" FORMAT\n" \
"          Shorthand for: find -word \"...\" FORMAT\n\n" \
"      (help | -h) [add | rm | find | tags | edit | help]\n" \
"      (add | rm | find | tags | edit | help) -h\n" \
"          Show help, optionally for a particular command.\n\n" \
"  FORMAT\n" \
"      [-f | -ff | -fff] Levels of detail in the results. Default, i.e. not specified means -ff.\n\n" \
"  ENVIRONMENT\n" \
"      To control the database connection, set environment variables with format MYDICT_xyz.\n" \
"      They will be converted to Postgres environment variables PGxyz. E.g. MYDICT_HOST => PGHOST."
