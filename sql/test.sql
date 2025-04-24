\c gau

CREATE TABLE person (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    age INTEGER,
    gender CHAR(1) DEFAULT 'n' CHECK (gender IN ('f', 'm', 'n')),
    address VARCHAR(200),
    phone VARCHAR(20),
    etc TEXT,
    
    -- 이름에 대한 인덱스
    CONSTRAINT idx_name INDEX (name),
    
    -- 주소에 대한 인덱스
    CONSTRAINT idx_address INDEX (address),
    
    -- 전화번호에 대한 인덱스
    CONSTRAINT idx_phone INDEX (phone),
    
    -- 이름, 나이, 전화번호의 유니크 제약조건
    CONSTRAINT uniq_name_age_phone UNIQUE (name, age, phone)
);

create table ttt (
    id serial,
    name text,
    dat date default now()::date,
    tim timestamp default now()
);


