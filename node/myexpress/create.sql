CREATE TABLE system_metrics (
    id SERIAL PRIMARY KEY,
    cpu_usage FLOAT,
    mem_usage FLOAT,
    ctim TIMESTAMP DEFAULT NOW()
);

