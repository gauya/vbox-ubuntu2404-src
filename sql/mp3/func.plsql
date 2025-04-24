CREATE OR REPLACE FUNCTION delete_mp3_file()
RETURNS trigger AS $$
BEGIN
    PERFORM pg_catalog.pg_stat_file(OLD.file_path);
    PERFORM pg_catalog.pg_ls_dir(dirname(OLD.file_path)); -- ensure path is safe
    PERFORM pg_catalog.pg_read_file(OLD.file_path, 0, 1); -- ensure readable
    EXECUTE format('rm -f %L', OLD.file_path);
    RETURN OLD;
EXCEPTION WHEN OTHERS THEN
    RAISE WARNING 'Could not delete file: %', SQLERRM;
    RETURN OLD;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_delete_mp3 AFTER DELETE ON mp3_library
FOR EACH ROW EXECUTE FUNCTION delete_mp3_file();
