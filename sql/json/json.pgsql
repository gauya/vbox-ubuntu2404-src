-- JSON과 JSONB 컬럼을 포함한 테이블 생성
CREATE TABLE if not exists products (
    id SERIAL PRIMARY KEY,
    name TEXT,
    details_json JSON,
    details_jsonb JSONB
);

-- 데이터 삽입
INSERT INTO products (name, details_json, details_jsonb)
VALUES (
    'Laptop',
    '{"brand": "Dell", "specs": {"cpu": "i7", "ram": 16}, "tags": ["electronics", "portable"]}',
    '{"brand": "Dell", "specs": {"cpu": "i7", "ram": 16}, "tags": ["electronics", "portable"]}'
),
(
    'Phone',
    '{"brand": "Apple", "specs": {"cpu": "A14", "ram": 4}, "tags": ["smartphone", "mobile"]}',
    '{"brand": "Apple", "specs": {"cpu": "A14", "ram": 4}, "tags": ["smartphone", "mobile"]}'
);

-- brand 값 추출
SELECT 
    name,
    details_json -> 'brand' AS json_brand,
    details_jsonb -> 'brand' AS jsonb_brand
FROM products;

-- 결과:
-- name   | json_brand | jsonb_brand
-- Laptop | "Dell"     | "Dell"
-- Phone  | "Apple"    | "Apple"

-- 텍스트로 brand 값 추출
SELECT 
    name,
    details_json ->> 'brand' AS json_brand_text,
    details_jsonb ->> 'brand' AS jsonb_brand_text
FROM products;

-- 결과:
-- name   | json_brand_text | jsonb_brand_text
-- Laptop | Dell            | Dell
-- Phone  | Apple           | Apple

-- 중첩된 specs.cpu 값 추출
SELECT 
    name,
    details_jsonb #>> '{specs, cpu}' AS cpu
FROM products;

-- 결과:
-- name   | cpu
-- Laptop | i7
-- Phone  | A14

-- ->: 키로 값을 추출(객체 반환).
-- ->>: 키로 값을 텍스트로 추출.
-- #>: 경로로 값을 추출.

-- #>> 지정된 경로(path)를 따라 값을 텍스트로 추출
-- ? 키 가 있는가
-- jsonb를 써라. JSON은 @>, ? 같은 연산자를 지원하지 않고 인덱스가 못 만들고 입력하는 그대로만 저장 

SELECT name
FROM products
WHERE details_jsonb ? 'brand';

-- GIN 인덱스 생성
CREATE INDEX if not exists idx_products_specs ON products USING GIN (details_jsonb);

-- 쿼리 (방법 3 사용)
SELECT details_jsonb->'specs'->'cpu' AS cpu
FROM products
WHERE details_jsonb->'specs' @> '{"cpu": "i7"}';

SELECT details_jsonb->'specs'->'cpu' AS cpu
FROM products
WHERE details_jsonb->'specs' IS NOT NULL
  AND details_jsonb->'specs'->>'cpu' = 'i7';

SELECT
    name,
    details_jsonb->'specs'->>'cpu' AS using_arrow,
    details_jsonb#>>'{specs, cpu}' AS using_path
FROM products
WHERE details_jsonb->'specs'->>'cpu' = details_jsonb#>>'{specs, cpu}';

SELECT name, details_jsonb #>> '{specs, cpu}' AS cpu
FROM products;


