DROP TABLE IF EXISTS word_modified;
DROP TABLE IF EXISTS word_tag;
DROP TABLE IF EXISTS tags;
DROP TABLE IF EXISTS entries;

CREATE EXTENSION IF NOT EXISTS citext;
CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE TABLE entries (
    word       citext PRIMARY KEY,
    definition text NOT NULL,
    created    timestamp with time zone NOT NULL
);
CREATE TABLE tags (
    tag citext PRIMARY KEY
);
CREATE TABLE word_tag (
    word citext NOT NULL REFERENCES entries ON DELETE CASCADE ON UPDATE CASCADE,
    tag  citext NOT NULL REFERENCES tags ON DELETE CASCADE ON UPDATE CASCADE,
    PRIMARY KEY(word, tag)
);
-- TODO: update modified time in response to changes to entries & word_tag
CREATE TABLE word_modified (
  word     citext NOT NULL REFERENCES entries ON DELETE CASCADE ON UPDATE CASCADE,
  modified timestamp with time zone,
  PRIMARY KEY(word)
);
-- BEFORE INSERT ON entries
CREATE OR REPLACE FUNCTION init_created_time() RETURNS trigger AS $$
    BEGIN
        NEW.created := current_timestamp;
        RETURN NEW;
    END;
$$ LANGUAGE plpgsql;

-- AFTER DELETE ON word_tag
CREATE OR REPLACE FUNCTION delete_last_tag() RETURNS trigger AS $$
    BEGIN
        PERFORM tag FROM word_tag WHERE tag = OLD.tag;
        IF NOT FOUND THEN
            DELETE FROM tags WHERE tag = OLD.tag;
        END IF;
        RETURN NULL;
    END
$$ LANGUAGE plpgsql;

CREATE TRIGGER init_created_time
    BEFORE INSERT ON entries
    FOR EACH ROW
    EXECUTE FUNCTION init_created_time();

CREATE TRIGGER delete_last_tag
    AFTER DELETE ON word_tag
    FOR EACH ROW
    EXECUTE FUNCTION delete_last_tag();
