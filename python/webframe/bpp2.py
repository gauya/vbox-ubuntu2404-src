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
        raise HTTPException(status_code=404, detail="파일이 존재하지 않습니다.")

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

