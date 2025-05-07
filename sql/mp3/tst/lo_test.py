#!/usr/bin/env python3
# test.py
import argparse, os, sys, psycopg2
from psycopg2 import sql
from contextlib import closing

DSN = "dbname=gau user=gau"      # ▶ 필요에 맞게 DSN 수정

def get_conn():
    return psycopg2.connect(DSN)

def list_entries():
    with closing(get_conn()) as conn, conn.cursor() as cur:
        cur.execute("SELECT id, filename, audio_oid FROM mp3_files ORDER BY id")
        rows = cur.fetchall()
        if not rows:
            print("No entries.")
        else:
            for r in rows:
                print(f"{r[0]:>4}  {r[1]:<30}  OID={r[2]}")

def write_file(path):
    if isinstance(path, list):
        path = path[0]
    fname = os.path.basename(path)
    print(fname)
    try:
        with closing(get_conn()) as conn, conn.cursor() as cur, open(path, "rb") as f:
            lo_oid = cur.connection.lobject(0, 'wb').oid      # 새 large‑object 생성
            lo_obj = conn.lobject(lo_oid, 'wb')
            lo_obj.write(f.read())
            lo_obj.close()
    
            cur.execute("""
              INSERT INTO mp3_files (filename, audio_oid)
              VALUES (%s, %s) RETURNING id
            """, (fname, lo_oid))
            new_id = cur.fetchone()[0]
            conn.commit()
            print(f"Stored '{fname}'  id={new_id}  oid={lo_oid}")
    except Exception as e:
        print(f"Error : {e}")
        
def read_to_file(key, out_path):
    if isinstance(out_path, list):
        out_path = out_path[0]
    # key 가 숫자면 id, 아니면 filename 으로 검색
    field = "id" if key.isdigit() else "filename"
    try:
        with closing(get_conn()) as conn, conn.cursor() as cur:
            cur.execute(
              sql.SQL("SELECT audio_oid, filename FROM mp3_files WHERE {} = %s")
                  .format(sql.Identifier(field)), [key])
            row = cur.fetchone()
            if not row:
                print("Entry not found."); return
            oid, db_fname = row
            out_path = out_path or db_fname
            lo_obj = conn.lobject(oid, 'rb')
            with open(out_path, "wb") as f:
                f.write(lo_obj.read())
            lo_obj.close()
            print(f"Wrote {out_path}  (oid={oid})")
    except Exception as e:
        print(f"Error : {e}")

def create_table():
    sql="""CREATE TABLE if not exists mp3_files (
id SERIAL PRIMARY KEY,
filename VARCHAR(255) NOT NULL,
audio_oid OID);"""

    try:
        with closing(get_conn()) as conn, conn.cursor() as cur:
            cur.execute(sql)
            conn.commit()
    except Exception as e:
        print(f"Error : {e}")

import getopt

def main():
    cmd = ''
    oid = ''
    try:
        opts, args = getopt.getopt(sys.argv[1:], "clr:w")
    except getopt.GetoptError as err:
        print(err)  # will print something like "option -a not recognized"
        sys.exit(2)
    for o, a in opts:
        if o == "-l":
            cmd = 'l'
        elif o == "-r":
            oid = a
            cmd = 'r'
        elif o == "-c":
            cmd = 'c' 
        elif o == "-w":
            cmd = 'w' 
        else:
            assert False, "unhandled option"
    
    file = args

    get_conn()

    if cmd == "l":
        list_entries()
    elif cmd == "w":
        write_file(file)
    elif cmd == "r":
        read_to_file(oid, file)
    elif cmd == 'c':
        create_table()

if __name__ == "__main__":
    main()

