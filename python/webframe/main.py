from fastapi import FastAPI, HTTPException
from fastapi.responses import JSONResponse
from mutagen.mp3 import MP3
from mutagen.id3 import ID3, ID3NoHeaderError, TIT2, TPE1, TALB, TRCK, TYER, TCON
import chardet
import os

app = FastAPI()


def clean_string(value):
    if not value:
        return None
    value = value.strip()
    if '-' in value:
        parts = [p.strip() for p in value.split('-')]
        if len(set(parts)) == 1:
            value = parts[0]
    return value

@app.post("/metadata/clean/{filepath:path}")
def clean_metadata(filepath: str):
    if not os.path.exists(filepath):
        raise HTTPException(status_code=404, detail=f"파일이 존재하지 않습니다.y {filepath}")

    try:
        audio = MP3(filepath)
        if audio.tags is None:
            raise HTTPException(status_code=404, detail="ID3 태그가 없습니다.")

        updated = False

        def fix(tag_key, mutagen_tag, default=None, is_track=False):
            nonlocal updated
            if tag_key in audio.tags:
                text = decode_id3_text(audio.tags[tag_key])
                cleaned = clean_string(text)
                if is_track and cleaned and cleaned.startswith("0"):
                    cleaned = str(int(cleaned))  # '01' → '1'
                if cleaned and cleaned != text:
                    audio.tags[tag_key] = mutagen_tag(encoding=3, text=cleaned)
                    updated = True
            elif default:
                audio.tags.add(mutagen_tag(encoding=3, text=default))
                updated = True

        fix('TIT2', TIT2)
        fix('TPE1', TPE1, default="Unknown Artist")
        fix('TALB', TALB, default="Unknown Album")
        fix('TRCK', TRCK, is_track=True)
        fix('TYER', TYER)
        fix('TCON', TCON, default="Unknown")

        if updated:
            audio.save()
            return {"message": "메타데이터가 정리되었습니다."}
        else:
            return {"message": "수정할 항목이 없습니다."}

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


def decode_id3_text(frame):
    try:
        text = frame.text[0]
        if isinstance(text, str):
            return text
        raw_bytes = bytes(text)
    except:
        raw_bytes = bytes(frame)

    result = chardet.detect(raw_bytes)
    encoding = result['encoding'] or 'utf-8'
    try:
        return raw_bytes.decode(encoding)
    except:
        return raw_bytes.decode('utf-8', errors='replace')

@app.get("/metadata/{filepath:path}")
def get_metadata(filepath: str):
    if not os.path.exists(filepath):
        raise HTTPException(status_code=404, detail="파일이 존재하지 않습니다.")

    try:
        audio = MP3(filepath)
        tags = audio.tags
        if tags is None:
            raise HTTPException(status_code=404, detail="ID3 태그가 없습니다.")

        metadata = {
            "title": decode_id3_text(tags.get('TIT2')) if 'TIT2' in tags else None,
            "artist": decode_id3_text(tags.get('TPE1')) if 'TPE1' in tags else None,
            "album": decode_id3_text(tags.get('TALB')) if 'TALB' in tags else None,
            "track": decode_id3_text(tags.get('TRCK')) if 'TRCK' in tags else None,
            "year": decode_id3_text(tags.get('TYER')) if 'TYER' in tags else None,
            "genre": decode_id3_text(tags.get('TCON')) if 'TCON' in tags else None,
        }
        return JSONResponse(content=metadata, media_type="application/json; charset=utf-8")

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/metadata/{filepath:path}")
def update_metadata(filepath: str, data: dict):
    if not os.path.exists(filepath):
        raise HTTPException(status_code=404, detail="파일이 존재하지 않습니다.")

    try:
        audio = MP3(filepath)
        if audio.tags is None:
            audio.add_tags()

        if 'title' in data:
            audio.tags.add(TIT2(encoding=3, text=data['title']))
        if 'artist' in data:
            audio.tags.add(TPE1(encoding=3, text=data['artist']))
        if 'album' in data:
            audio.tags.add(TALB(encoding=3, text=data['album']))
        if 'track' in data:
            audio.tags.add(TRCK(encoding=3, text=data['track']))
        if 'year' in data:
            audio.tags.add(TYER(encoding=3, text=data['year']))
        if 'genre' in data:
            audio.tags.add(TCON(encoding=3, text=data['genre']))

        audio.save()
        return JSONResponse(content={"message": "메타데이터가 성공적으로 업데이트되었습니다."}, media_type="application/json; charset=utf-8")

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
