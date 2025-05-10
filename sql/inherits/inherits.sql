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


create table if not exists worker (
    position varchar(12),
    salary int
) inherits(person);

create index if not exists i_worker on worker (position);

-- insert into worker(이름,나이,성별,주소,전화,position,salary) values ( '고구마',23,'f','함평','352-3242','꼰대',1200000 );
-- insert into person(이름,나이,성별,주소,전화) values ( '감자',56,'f','서울','010-3344-2233' );

-- select * from only person;
-- select * from person;
-- select * from only worker;
-- select * from worker;
-- select * from worker*;


