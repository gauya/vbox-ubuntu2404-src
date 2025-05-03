# file_watcher.py
import psycopg2, select, json, os

conn = psycopg2.connect("dbname=YOUR_DB user=YOUR_USER")
conn.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_AUTOCOMMIT)

cur = conn.cursor()
cur.execute("LISTEN file_delete;")
print("🔄 파일 삭제 감시 중 (채널: file_delete)...")

while True:
    if select.select([conn], [], [], 5) == ([], [], []):
        continue  # timeout
    conn.poll()
    while conn.notifies:
        notify = conn.notifies.pop(0)
        data = json.loads(notify.payload)
        path = data["path"]
        print(f"📣 삭제 요청: {path}")
        try:
            os.remove(path)
            print(f"✅ 삭제 완료: {path}")
        except Exception as e:
            print(f"❌ 삭제 실패: {e}")

