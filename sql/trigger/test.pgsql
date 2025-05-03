-- TRIGGER TEST

CREATE EXTENSION IF NOT EXISTS plpythonu;

-- 예제 테이블
CREATE TABLE dfn (
    name TEXT PRIMARY KEY,
    path VARCHAR(120) NOT NULL unique
);

-----------------------------------------------

-- 1.
CREATE OR REPLACE FUNCTION delete_file_entry(nm TEXT)
RETURNS VOID AS $$
    import os

    # 커서 가져오기
    plpy.execute("BEGIN")
    result = plpy.execute("SELECT path FROM dfn WHERE name = %s", [nm])

    if not result:
        plpy.error("파일 별명 '{}'을(를) 찾을 수 없습니다.".format(nm))

    path = result[0]["path"]

    # 파일 존재 여부 확인
    if not os.path.exists(path):
        plpy.error("파일 '{}' 이 존재하지 않습니다.".format(path))

    try:
        os.remove(path)
    except Exception as e:
        plpy.error("파일 삭제 실패: " + str(e))

    plpy.execute("DELETE FROM dfn WHERE name = %s", [nm])
    plpy.execute("COMMIT")
$$ LANGUAGE plpythonu;

-- INSERT INTO dfn VALUES ('foo', '/tmp/testfile.mp3');

-- SELECT delete_file_entry('foo'); -- o/s상의 파일과 DB속의 데이타 삭제



-- 2
CREATE OR REPLACE FUNCTION dfn_file_delete_trigger()
RETURNS trigger AS $$
    import os

    path = TD["old"]["path"]
    name = TD["old"]["name"]

    if not os.path.exists(path):
        plpy.error(f"[삭제 중단] 파일이 존재하지 않음: {path}")

    try:
        os.remove(path)
        plpy.info(f"[삭제 성공] 파일: {path}")
    except Exception as e:
        plpy.error(f"[삭제 실패] {e}")

    return TD["old"]
$$ LANGUAGE plpythonu;

DROP TRIGGER IF EXISTS trg_dfn_delete ON dfn;

CREATE TRIGGER trg_dfn_delete
BEFORE DELETE ON dfn
FOR EACH ROW
EXECUTE FUNCTION dfn_file_delete_trigger();

//////////////////////////////////////

-- DELETE FROM dfn WHERE name = 'foo';  -- DB데이타를 삭제하면 파일도 자동삭제

-- 3.

CREATE OR REPLACE FUNCTION notify_file_delete()
RETURNS trigger AS $$
DECLARE
    payload JSON;
BEGIN
    payload := json_build_object(
        'name', OLD.name,
        'path', OLD.path
    );

    PERFORM pg_notify('file_delete', payload::text);
    RETURN OLD;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_notify_delete
AFTER DELETE ON dfn
FOR EACH ROW
EXECUTE FUNCTION notify_file_delete();

-- 나머지 파일삭제는 외부시스템에서 처리
-- DELETE FROM dfn WHERE name = 'foo';  -- 데몬이동작 - 

