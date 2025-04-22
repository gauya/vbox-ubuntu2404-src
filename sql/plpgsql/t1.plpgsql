DO $$
BEGIN
    BEGIN -- TRANSACTION ISOLATION LEVEL SERIALIZABLE;

    -- 테이블 생성
    CREATE TABLE test_table (id SERIAL PRIMARY KEY, value INTEGER);

    -- 데이터 삽입 (오류 발생 가능성 있는 코드)
    INSERT INTO test_table (value) VALUES (10);
    INSERT INTO test_table (value) VALUES ('abc'); -- 데이터 타입 오류 발생

    -- 모든 작업 성공 시 커밋
    COMMIT; -- TRANSACTION;
EXCEPTION
    WHEN others THEN
        -- 오류 발생 시 롤백
        ROLLBACK; -- TRANSACTION;
        RAISE NOTICE '트랜잭션 롤백: %', SQLERRM;
END;
$$ LANGUAGE plpgsql;


