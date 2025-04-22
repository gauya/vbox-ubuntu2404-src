-- MP3 파일 등록 함수
CREATE OR REPLACE FUNCTION add_mp3(
    p_title VARCHAR,
    p_artist VARCHAR,
    p_composer VARCHAR,
    p_sample_rate INT,
    p_file_size BIGINT,
    p_duration INTERVAL,
    p_album VARCHAR,
    p_release_date DATE,
    p_genre1 VARCHAR,
    p_genre2 VARCHAR,
    p_file_path VARCHAR
) RETURNS UUID AS $$
DECLARE
    new_id UUID;
BEGIN
    INSERT INTO mp3_library (
        title, artist, composer, sample_rate, 
        file_size, duration, album, release_date,
        genre1, genre2, file_path
    ) VALUES (
        p_title, p_artist, p_composer, p_sample_rate,
        p_file_size, p_duration, p_album, p_release_date,
        p_genre1, p_genre2, p_file_path
    ) RETURNING id INTO new_id;
    
    RETURN new_id;
END;
$$ LANGUAGE plpgsql;

-- 음원 검색 함수
CREATE OR REPLACE FUNCTION search_mp3(
    search_term VARCHAR,
    limit_rows INT DEFAULT 100
) RETURNS SETOF mp3_library AS $$
BEGIN
    RETURN QUERY
    SELECT *
    FROM mp3_library
    WHERE title ILIKE '%' || search_term || '%'
       OR artist ILIKE '%' || search_term || '%'
       OR album ILIKE '%' || search_term || '%'
    LIMIT limit_rows;
END;
$$ LANGUAGE plpgsql;

-- trigger, view
-- 자동 갱신 타임스탬프 트리거
CREATE OR REPLACE FUNCTION update_timestamp()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_mp3_update
BEFORE UPDATE ON mp3_library
FOR EACH ROW EXECUTE FUNCTION update_timestamp();

-- 통계 정보 뷰
CREATE VIEW mp3_stats AS
SELECT 
    COUNT(*) AS total_songs,
    SUM(EXTRACT(EPOCH FROM duration)/3600) AS total_hours,
    genre1,
    genre2
FROM mp3_library
GROUP BY genre1, genre2;

-- 
-- 테이블 파티셔닝 (대량 데이터용)
CREATE TABLE mp3_library_2023 PARTITION OF mp3_library
    FOR VALUES FROM ('2023-01-01') TO ('2024-01-01');

-- 전문 검색 인덱스 (제목, 아티스트 등)
CREATE EXTENSION pg_trgm;
CREATE INDEX idx_mp3_search ON mp3_library 
    USING gin (title gin_trgm_ops, artist gin_trgm_ops);

-- 연결 풀 설정 (Python 애플리케이션에서)
ALTER SYSTEM SET max_connections = 100;



CREATE OR REPLACE FUNCTION export_mp3_to_file(
    p_mp3_id UUID,
    p_output_path TEXT
) RETURNS TEXT AS $$
DECLARE
    v_file_path TEXT;
    v_title TEXT;
    v_artist TEXT;
    v_oid OID;
    v_export_path TEXT;
BEGIN
    -- 메타데이터 조회
    SELECT title, artist, file_path, lo_oid
    INTO v_title, v_artist, v_file_path, v_oid
    FROM mp3_library
    WHERE id = p_mp3_id;
    
    IF v_title IS NULL THEN
        RAISE EXCEPTION 'MP3 with ID % not found', p_mp3_id;
    END IF;
    
    -- 안전한 파일명 생성
    v_export_path := p_output_path || '/' || 
                    regexp_replace(v_artist || ' - ' || v_title, '[\/:*?"<>|]', '_', 'g') || 
                    '.mp3';
    
    -- 저장 방식에 따라 내보내기
    IF v_oid IS NOT NULL THEN
        -- Large Object 방식
        PERFORM lo_export(v_oid, v_export_path);
    ELSIF v_file_path IS NOT NULL THEN
        -- 하이브리드 방식 (파일 복사)
        EXECUTE format('COPY %L TO %L', v_file_path, v_export_path);
    ELSE
        -- BYTEA 방식 (PL/pgSQL에서는 직접 구현 어려움)
        RAISE EXCEPTION 'BYTEA export requires client-side processing';
    END IF;
    
    RETURN v_export_path;
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

