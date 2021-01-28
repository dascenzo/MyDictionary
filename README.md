# MyDictionary

MyDictionary is a command line personal dictionary for those hard-to-remember words, terms, and acronyms.

If you often find yourself thinking things like, "What does Synergistic Leverage Ratio mean again?", and "I wish I 
remembered what ICBINB stood for," then MyDictionary may be for you. Add words to your dictionary. Give them definitions
and tags. Then use the flexible search and viewing options when you're scratching your head over a piece of terminology
later. MyDictionary's goal is to make life easier by making the information you need closer.

## Building
To build, you'll need to be in a POSIX environment and have the following installed:
* A C++ 17 compiler
* Make
* pkg-config
* [libpqxx](https://github.com/jtv/libpqxx) (tested with version 7.2)

Once the dependencies are in place, from within the top-level directory of the project, run:

    $ make

If all went well, you should have an executable file `mydict` in the same directory. Execute `$ mydict help` to get a
detailed listing of MyDictionary's behavior, and the subcommands available to you.

## Running
Before running, a PostgreSQL connection must be configured. First, attach to the database you want to hold MyDictionary's data, and
run the provided `Tables.sql` script. For example:

    $ psql -U myuser -d mydb -f Tables.sql

Then in your environment, set variables in the form `MYDICT_FOO`. MyDictionary will then set in its environment `PGFOO`.
Notice `PGFOO` is the format of [PostgreSQL environment variables](https://www.postgresql.org/docs/current/libpq-envars.html). So in this manner
MyDictionary's database connection may be controlled.

(Note, above tested with PostgreSQL server version 13.)

## Examples
Add an entry to the dictionary:

    $ mydict add -word "TPS Report" -tag "reports" -tag "quality assurance" \
        -def "Test Procedure Set Report. Standard reporting document for RFC 1149. Must have cover sheet."
        
List all entries with the tag "reports":

    $ mydict find -tag "reports"
    
## Testing
The test script runs a series of `mydict` commands and checks if their output is what's expected:

    # set the database to be used (warning, MyDictionary related tables will be cleared) 
    $ export MYDICTTEST_DB="my_test_database"
    $ cd tests
    $ ./test-cmd-line.sh
    
## License

See [LICENSE](LICENSE.txt).
