# file_delete_worker.py
import psycopg2, os, time, logging
from datetime import datetime

logging.basicConfig(
    filename='/opt/file-watcher/delete.log',
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s'
)

DB_DSN = os.getenv("DB_DSN")  # e.g., "dbname=mydb user=postgres"
POLL_INTERVAL = 2  # seconds

conn = psycopg2.connect(DB_DSN)
conn.autocommit = True
cur = conn.cursor()

logging.info("[INIT] 파일 삭제 큐 워커 시작")

while True:
    cur.execute("""
        SELECT id, name, path FROM file_delete_queue
        WHERE status = 'pending'
        ORDER BY requested_at ASC
        LIMIT 1
    """)
    row = cur.fetchone()

    if not row:
        time.sleep(POLL_INTERVAL)
        continue

    file_id, name, path = row
    logging.info(f"[PENDING] {name}: {path}")

    try:
        if os.path.exists(path):
            os.remove(path)
            status = 'done'
            error = None
            logging.info(f"[OK] 삭제 성공: {path}")
        else:
            status = 'skipped'
            error = 'File not found'
            logging.warning(f"[SKIP] 파일 없음: {path}")
    except Exception as e:
        status = 'error'
        error = str(e)
        logging.error(f"[FAIL] 삭제 실패: {path} - {e}")

    cur.execute("""
        UPDATE file_delete_queue
        SET status = %s,
            processed_at = %s,
            error = %s
        WHERE id = %s
    """, (status, datetime.now(), error, file_id))

