import os
import psycopg2
from datetime import timedelta

def insert_to_db(filename, conn):
    with open(filename,'rb') as f:
        mp3 = f.read()

    ins = { "filename":filename, "mp3":mp3}
    with conn.cursor() as cur:
        try:
            cur.execute("""
                INSERT INTO tt ( name, song )
                ) VALUES ( %(filename)s, %(mp3)s)
                RETURNING id
                ON CONFLICT DO NOTHING
            """, ins)
            rid = cur.fetchone()[0]
            return rid
        finally:
            print(f"insert {filename}")

import sys,shutil
if __name__ == '__main__':
    path = sys.argv[1]

    conn = psycopg2.connect(dbname='gau',user='gau')
    rid = insert_to_db( path, conn)
    if rid:
        print(f"rid = {rid}")

