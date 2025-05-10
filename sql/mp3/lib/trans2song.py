import os,dotenv
import psycopg2
from mutagen.mp3 import MP3
from mutagen.id3 import ID3, APIC, TPE1, TIT2, TALB, COMM, TCON, TDRC, USLT
from datetime import timedelta

def extract_metadata(file_path):
    try:
        audio = MP3(file_path)
        tags = ID3(file_path)
        duration = timedelta(seconds=int(audio.info.length))

        return {
            'title': tags.get('TIT2', None).text[0] if tags.get('TIT2') else None,
            'artist': tags.get('TPE1', None).text[0] if tags.get('TPE1') else None,
            'composer': tags.get('TCOM', None).text[0] if tags.get('TCOM') else None,
            'album_artist': tags.get('TPE2', None).text[0] if tags.get('TPE2') else None,
            'album': tags.get('TALB', None).text[0] if tags.get('TALB') else None,
            'genre': tags.get('TCON', None).text[0] if tags.get('TCON') else None,
            'track': int(tags.get('TRCK').text[0].split('/')[0]) if tags.get('TRCK') else None,
            'release_date': tags.get('TDRC', None).text[0] if tags.get('TDRC') else None,
            'sample_rate': audio.info.sample_rate,
            'duration': duration,
            'description': tags.get('COMM::eng', None).text[0] if tags.get('COMM::eng') else tags.get('COMM::kor'),
            'lyrics': tags.get('USLT::eng', None).text if tags.get('USLT::eng') else tags.get('USLT::kor'),
            'file_name': os.path.basename(file_path),
            'file_path': os.path.split(os.path.realpath(file_path))[0],
            'file_size': os.path.getsize(file_path),
            'cover_image': tags.get('APIC:').data if tags.get('APIC:') else None
        }
    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return None

from datetime import date

def delete_to_db(rid, conn):
    with conn.cursor() as cur:
        try:
            cur_execute(f"select id, file_name, file_path from mp3_library where id = %{rid}")
            record = cur.fetchone()
            if record:
                cur.execute(f"delete from mp3_library where id = %{rid}")
                conn.commit()
            else:
                return None
            # delete file
            path = record['file_path'] + '/' + record['file_name']
            shutil.os.remove(path)
            return 1
        except Exception as e:
            print(f"Error {e}")
            return None

def insert_to_db(filename, conn):
    metadata = extract_metadata(filename)
    
    metadata['file_name'] = os.path.basename(filename)
    metadata['file_path'] = '/db/mp3/file'

    dest = '/db/mp3/file/' + metadata['file_name']

    if metadata['release_date']:
        rd = metadata['release_date'].text
        try:
            if len(rd) == 4:
                metadata['release_date'] = date(int(rd),1,1).strftime('%Y-%m-%d')
            elif len(rd) == 8:
                metadata['release_date'] = date(int(rd[:4]),int(rd[4:6]),int(rd[6:])).strftime('%Y-%m-%d')
        except:
            metadata['release_date'] = None
    if metadata['track']:
        try:
            metadata['track'] = int(metadata['track'].text) if int(metadata['track'].text) < 20 else None
        except:
            metadata['track'] = None    
    if metadata['lyrics']:
        try:
            metadata['lyrics'] = metadata['lyrics'].text
        except:
            metadata['lyrics'] = None
    if metadata['description']:
        try:
            metadata['description'] = metadata['description'].text
        except:
            metadata['description'] = None
    if metadata['title'] and len(metadata['title']) > 100:
        metadata['title'] = metadata['title'][:100]
    if metadata['artist'] and len(metadata['artist']) > 100:
        metadata['artist'] = metadata['artist'][:100]
    if metadata['album'] and len(metadata['album']) > 100:
        metadata['album'] = metadata['album'][:100]
    if metadata['album_artist'] and len(metadata['album_artist']) > 100:
        metadata['album_artist'] = metadata['album_artist'][:100]
    if metadata['genre'] and len(metadata['genre']) > 40:
        metadata['genre'] = metadata['genre'][:40]

    with conn.cursor() as cur:
        try:
            cur.execute("""
                INSERT INTO mp3_library (
                    title, artist, composer, album_artist, album, genre, track, release_date,
                    sample_rate, duration, description, lyrics,
                    file_name, file_path, file_size, cover_image
                ) VALUES (
                    %(title)s, %(artist)s, %(composer)s, %(album_artist)s, %(album)s, %(genre)s,
                    %(track)s, %(release_date)s, %(sample_rate)s, %(duration)s, %(description)s,
                    %(lyrics)s, %(file_name)s, %(file_path)s, %(file_size)s, %(cover_image)s
                )
                RETURNING id
                ON CONFLICT DO NOTHING
            """, metadata)
            rid = cur.fetchone()[0]

            dest = '/db/mp3/file/' + metadata['file_name']
            try:
                shutil.copy2(filename, dest)
            except Exception as e:
                conn.rollback()
                print(f"Error FIle copy <-insert db : {e}")
                return None
            conn.commit()
            return rid
        except Exception as e:
            print(f"Error insert db : {e}")
            return None


def trans2songs(conn):
    try:
        with conn.cursor() as cur:
            cur.execute("""
                select id, file_name 
                from songs 
                where song is null and date_part('year',release_date) < 2015
                for update skip locked"""
            )
            updated = 0
            for record in cur.fetchall():
                id, file_name = record
                
                with open('/db/mp3/file/' + file_name,'rb') as f:
                    song = f.read()
    
                bsize = len(song)
    
                cur.execute("""
                    update songs set song = %s, dsize = %s
                    where id = %s
                    """, ( song, bsize, id,)
                )
                updated += 1
                print(f"{updated} file={bsize} : {file_name}")
    #            tt,fn = cur.fetchone()
    #            print(f"{tt},{fn} update")
            conn.commit()
    except Exception as e:
        if conn: conn.rollback()
        print(f"{e}")
    finally:
        if conn: conn.close()

def _trans2songs(conn):
    try:
        with conn.cursor() as cur:
            cur.execute("""
                select id, file_name 
                from songs 
                where song is null
                for update skip locked"""
            )
            updated = 0
            while True:
                record = cur.fetchone()
                if not record: 
                    break
                id, file_name = record
                
                with open('/db/mp3/file/' + file_name,'rb') as f:
                    song = f.read()
    
                bsize = len(song)
    
                cur.execute("""
                    update songs set song = %s, dsize = %s
                    where id = %s
                    """, ( song, bsize, id,)
                )
                conn.commit()
                updated += 1
                print(f"{updated} file={bsize} : {file_name}")
#            tt,fn = cur.fetchone()
#            print(f"{tt},{fn} update")
    except Exception as e:
        if conn: conn.rollback()
        print(f"{e}")
    finally:
        if conn: conn.close()

import sys,shutil

def main(conn,path):
    flist = []
    if os.path.isfile(path):
        flist[0] = path
    elif os.path.isdir(path):
        fl = os.listdir(path)
        for f in fl:
            if len(f) > 4 and f[-4:] == '.mp3':
                flist.append(f)

    elist=[]
    for fl in flist:
        print(fl)
        for r in rr.keys():
            if r == 'cover_image' and False:
                try:
                    nfn = os.path.basename(fl).split('.')[0] + '.jpeg'
                    with open(nfn,'wb') as f:
                        f.write(rr[r])
                except:
                    pass
                continue
            #print(f"{r} : {rr[r]}")
        
        rid = insert_to_db( path + '/' + fl, conn)
        if not rid:
            elist.append(fl)
#            conn = psycopg2.connect(dbname=os.getenv("DB_NAME"),user=os.getenv("DB_USER"))

    print('*'*50)
    for e in elist:
        print(e)

if __name__ == '__main__':
    dotenv.load_dotenv()
    conn = psycopg2.connect(dbname=os.getenv("DB_NAME"),user=os.getenv("DB_USER"))
    
    path = "./"
    if len(sys.argv) > 1:
        path = sys.argv[1]

#    main(conn,path)
    trans2songs(conn)
