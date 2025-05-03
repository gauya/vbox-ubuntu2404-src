# file_watcher.py with .env support, logging, and retry
import psycopg2, select, json, os, time, logging
from dotenv import load_dotenv

load_dotenv()

# Setup logging
logging.basicConfig(
    filename='/opt/file-watcher/delete.log',
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s'
)

DB_DSN = os.getenv("DB_DSN")  # e.g. "dbname=yourdb user=youruser"
RETRY_LIMIT = 3

conn = psycopg2.connect(DB_DSN)
conn.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_AUTOCOMMIT)
cur = conn.cursor()
cur.execute("LISTEN file_delete;")

logging.info("[INIT] Listening for file_delete events...")

while True:
    if select.select([conn], [], [], 5) == ([], [], []):
        continue  # Timeout

    conn.poll()
    while conn.notifies:
        notify = conn.notifies.pop(0)
        try:
            data = json.loads(notify.payload)
            path = data["path"]
            name = data["name"]
        except Exception as e:
            logging.error(f"Invalid payload: {notify.payload} - {e}")
            continue

        logging.info(f"[RECV] Request to delete {path} (alias: {name})")

        # Retry logic
        success = False
        for attempt in range(1, RETRY_LIMIT + 1):
            try:
                os.remove(path)
                logging.info(f"[OK] Deleted {path} on attempt {attempt}")
                success = True
                break
            except FileNotFoundError:
                logging.warning(f"[SKIP] File not found: {path}")
                success = True  # No retry needed
                break
            except Exception as e:
                logging.error(f"[FAIL] Attempt {attempt}: {e}")
                time.sleep(1)

        if not success:
            logging.error(f"[GIVEUP] Failed to delete {path} after {RETRY_LIMIT} attempts")

