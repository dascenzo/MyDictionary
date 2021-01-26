#!/bin/sh

# README: define env var MYDICTTEST_DB to be the name of the test database.

DB_INIT_SCRIPT="../Tables.sql"
MYDICT="../mydict"
ACT="actual-cmd-line.txt"
EXP="expected-cmd-line.txt"

if ! psql -d "$MYDICTTEST_DB" -f "$DB_INIT_SCRIPT" > /dev/null 2>&1; then
  echo "failed to initialize test database"
  exit 1
fi

md()
{
  echo "> $1" >> "$ACT"
  eval "$MYDICT" $1 >> "$ACT" 2>> "$ACT"
}
# clear output file
: > "$ACT"

# test env
export MYDICT_DATABASE="DOES_NOT_EXIST"
md "find -all"
export MYDICT_DATABASE="$MYDICTTEST_DB"

# add entires
#   => coffee(drink, hot)
#   => strawberry(cold, fruit),
#   => toast(grain, hot)
#   => water(drink)
#   => whoops()
md "add -word Coffee   -def 'Stimulating drink made from coffee beans' -tag Hot -tag Drink"
md "add -w    toast    -t    Hot -d 'The result of heating a slice of bread' -t Grain"
md "add -tag  drink    -def  H2O -w  water"
md "add -w    Whoops   -d    abc"
md "add -de  'Small red fruit with seeds on the outside' -ta Cold -ta Fruit -wor Strawberry"

# find
md "find -all"
md "find -a"
md "find -word Toast"
md "find -pat '^w.*'"
md "find -tag Hot"
md "find -tag Drink -tag hot"
md "find -all -f"
md "find -all -ff"
#md "find -all -fff"

# shorthand find
md "strawberry"
md "strawberry -f"
md "strawberry -ff"
#md "strawberry -fff"

# tags
md "tags"

# remove word
md "rm -w whoops"
md "find -all -f"

# remove tag globally
md "rm -t Hot"
md "tags"

# edit word: change definition and add tag
md "edit -word Coffee -d 'A morning beverage' -t 'Brewed'"
md "coffee"
md "tags"

# edit word: remove tag
md "edit -rmtag 'Brewed' -w Coffee"
md "tags"

# edit word: change word
md "edit -word Coffee -edit 'Orange juice'"
md "'Orange juice'"

# edit tag
md "edit -tag drink -ed beverage"
md "tags"

# test "--"
md "edit -w 'Orange juice' -tag -- '-state=FL'"
md "tags"
md "-- -Orange"

# add similar: -nocheck
md "add -w 'Orange juce' -def 'Misspelling of a certain citrus refreshment' -nocheck"
md "'Orange juce'"

# data error
md "add -w 'Orange juice' -def 'Again orange juice'"
md "rm -w nonexistent"
md "rm -t nonexistent"
md "edit -w nonexistent -def 'new definition'"
md "edit -w 'Orange juice' -rmtag 'noexistent'"
md "edit -tag 'nonexistent' -edit 'new tag'"

# usage error
md "add"
md "add -x"
md "add -word"
md "add -word -something"
  # call this the test of equivalence partition for "nothing following --"
md "add -word --"
md "add -word  something -def"
md "add -word  something -def X -tag"
  # missing -def
md "add -word something"
  # unique
md "add -word abc -word xyz -def 123"
md "add -word abc -def xyz -def 123"
md "add -word abc -def xyz -nocheck -nocheck"

md "rm"
md "rm -x"
md "rm -word -something"
md "rm -word"
md "rm -tag"
  # unique
md "rm -word something -tag abc"
md "rm -word abc -word xyz"
md "rm -tag abc -tag xyz"

md "find"
md "find -x"
md "find -word -something"
md "find -word"
md "find -pat"
md "find -tag"
md "find -word X -f -ff"
  # unique
md "find -all -all"
md "find -all -word abc"
md "find -all -pat abc"
md "find -all -tag abc"
md "find -word abc -word xyz"
md "find -word abc -pat xyz"
md "find -word abc -tag xyz"
md "find -pat abc -pat xyz"
md "find -pat abc -tag xyz"

md "tags xyz"

md "edit"
md "edit -x"
md "edit -word -something"
md "edit -word"
md "edit -word X"
md "edit -word X -edit"
md "edit -word X -def"
md "edit -word X -tag"
md "edit -word X -rmtag"
md "edit -tag -something"
md "edit -tag"
md "edit -tag X"
md "edit -tag X -edit"
  # unique
md "edit -word X -word Y"
md "edit -word X -edit Y -edit Z"
md "edit -word X -def Y -def Z"
md "edit -tag X -tag Y -edit Z"
md "edit -tag X -edit Y -edit Z"

md "--"
md "-Orange"
md "'Orange juice' -x"
md "'Orange juice' -f abc"
md "'Orange juice' -f -ff"

  # no action specified
md ""

# help flexible usage
md "help abc 123"
md "-h abc 123"
md "help add abc 123"
md "-h add abc 123"
md "help help abc 123"
md "-h help abc 123"

md "add -h abc 123"
md "rm -h abc 123"
md "find -h  abc 123"
md "tags -h abc 123"
md "edit -h abc 123"
md "help -h abc 123"

# help - expected usage
md "help add"
md "help rm"
md "help find"
md "help tags"
md "help edit"
md "help help"

md "-h add"
md "-h rm"
md "-h find"
md "-h tags"
md "-h edit"
md "-h help"

md "add -h"
md "rm -h"
md "find -h"
md "tags -h"
md "edit -h"
md "help -h"

md "help"
md "-h"

echo "diffing actual and expected out..."

# give user a chance to be aware results are coming
sleep 2

if ! diff --brief --report-identical-files "$ACT" "$EXP"; then
  diff --side-by-side "$ACT" "$EXP"
fi
