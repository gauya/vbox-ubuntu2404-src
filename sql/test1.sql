CREATE TABLE if not exists person (
    id SERIAL PRIMARY KEY,
    이름 TEXT NOT NULL,
    나이 INTEGER,
    성별 CHAR(1) DEFAULT 'n' CHECK (성별 IN ('f', 'm', 'n')),
    주소 TEXT,
    전화 TEXT,
    
    -- 이름, 주소, 전화 각각에 대해 인덱스 생성 (CREATE TABLE과 동시에)
    UNIQUE (이름, 나이, 전화)
);

create index if not exists i_person_name on person (이름);
create index if not exists i_person_주소 on person (주소);
