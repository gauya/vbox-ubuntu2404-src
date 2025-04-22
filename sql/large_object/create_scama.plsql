CREATE TABLE if not exists mp3_library (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    title VARCHAR(255) NOT NULL,
    artist VARCHAR(255) NOT NULL,
    composer VARCHAR(255),
    sample_rate INT CHECK (sample_rate > 0),
    file_size BIGINT CHECK (file_size > 0),
    duration INTERVAL NOT NULL,
    album VARCHAR(255),
    release_date DATE,
    genre1 VARCHAR(50),
    genre2 VARCHAR(50),
    file_path VARCHAR(512) UNIQUE NOT NULL,
    audio_data BYTEA,  -- 선택적: 실제 오디오 데이터 저장
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 인덱스 생성
CREATE INDEX idx_mp3_title ON mp3_library(title);
CREATE INDEX idx_mp3_artist ON mp3_library(artist);
CREATE INDEX idx_mp3_composer ON mp3_library(composer);
CREATE INDEX idx_mp3_album ON mp3_library(album);
CREATE INDEX idx_mp3_genres ON mp3_library(genre1, genre2);
