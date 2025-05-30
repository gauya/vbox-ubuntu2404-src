from fastapi import FastAPI, UploadFile, File
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from pathlib import Path
import os
import io
from pydub import AudioSegment  # duration 계산용

app = FastAPI()
upload_dir = Path("uploads")
thumb_dir = Path("thumbs")
upload_dir.mkdir(exist_ok=True)
thumb_dir.mkdir(exist_ok=True)

def try_decode(b: bytes):
    """한글 인코딩 깨짐 방지용 디코더."""
    for enc in ("utf-8", "euc-kr", "utf-16", "latin1"):
        try:
            return b.decode(enc)
        except UnicodeDecodeError:
            continue
    return f"<binary:{len(b)}B>"

def parse_id3v2(data: bytes, filename: str):
    info = {}
    if not data.startswith(b"ID3"):
        return {"error": "ID3v2 header not found"}

    tag_size = int.from_bytes(data[6:10], byteorder="big") & 0x7F7F7F7F
    pos = 10

    while pos < 10 + tag_size:
        frame_id = data[pos:pos+4].decode("latin1", errors="ignore")
        frame_size = int.from_bytes(data[pos+4:pos+8], byteorder="big")
        if frame_size == 0 or not frame_id.strip():
            break

        content = data[pos+10:pos+10+frame_size]

        # 텍스트 프레임 (예: TIT2, TPE1, TCON, TRCK 등)
        if frame_id.startswith("T") and len(content) > 1:
            value = try_decode(content[1:])
            info[frame_id] = value

        # 설명 (COMM 프레임)
        elif frame_id == "COMM" and len(content) > 5:
            lang = content[1:4].decode("latin1")
            desc_end = content.find(b'\x00', 4)
            if desc_end != -1:
                desc = try_decode(content[4:desc_end])
                text = try_decode(content[desc_end+1:])
                info[f"COMM::{lang}"] = f"{desc}: {text}"

        # 가사 (USLT 프레임)
        elif frame_id == "USLT" and len(content) > 5:
            lang = content[1:4].decode("latin1")
            desc_end = content.find(b'\x00', 4)
            if desc_end != -1:
                desc = try_decode(content[4:desc_end])
                lyrics = try_decode(content[desc_end+1:])
                info[f"USLT::{lang}"] = lyrics

        # 썸네일
        elif frame_id == "APIC":
            parts = content[1:].split(b'\x00', 2)
            if len(parts) >= 3:
                mime = parts[0].decode(errors="ignore")
                image_data = parts[2]
                ext = ".jpg" if "jpeg" in mime else ".png"
                img_path = thumb_dir / (Path(filename).stem + ext)
                with open(img_path, "wb") as f:
                    f.write(image_data)
                info["APIC"] = f"/thumbs/{img_path.name}"

        pos += 10 + frame_size

    # ✅ 요약 키 채워넣기 (언어 우선순위)
    if "USLT::kor" in info:
        info["USLT"] = info["USLT::kor"]
    elif "USLT::eng" in info:
        info["USLT"] = info["USLT::eng"]

    if "COMM::kor" in info:
        info["COMM"] = info["COMM::kor"]
    elif "COMM::eng" in info:
        info["COMM"] = info["COMM::eng"]

    return info

def try_decode2(data, encodings=["utf-8", "euc-kr", "cp949", "latin1"]):
    for enc in encodings:
        try:
            return data.decode(enc).strip('\x00')
        except:
            continue
    return "<decode error>"

def parse_id3v2_2(data: bytes, filename: str):
    info = {}
    if not data.startswith(b"ID3"):
        return {"error": "ID3v2 header not found"}

    tag_size = int.from_bytes(data[6:10], byteorder="big") & 0x7F7F7F7F
    pos = 10

    while pos < 10 + tag_size:
        frame_id = data[pos:pos+4].decode("latin1", errors="ignore")
        frame_size = int.from_bytes(data[pos+4:pos+8], byteorder="big")
        if frame_size == 0 or not frame_id.strip():
            break

        content = data[pos+10:pos+10+frame_size]

        # 텍스트 프레임
        if frame_id.startswith("T") and len(content) > 1:
            value = try_decode2(content[1:])
            info[frame_id] = value

        # 설명 (COMM)
        elif frame_id == "COMM" and len(content) > 5:
            enc = content[0]
            lang = content[1:4].decode("latin1")
            desc_end = content.find(b'\x00', 4)
            if desc_end != -1:
                desc = try_decode2(content[4:desc_end])
                text = try_decode2(content[desc_end+1:])
                key = f"COMM::{lang}"
                info[key] = f"{desc}: {text}"

        # 가사 (USLT)
        elif frame_id == "USLT" and len(content) > 5:
            enc = content[0]
            lang = content[1:4].decode("latin1")
            desc_end = content.find(b'\x00', 4)
            if desc_end != -1:
                desc = try_decode2(content[4:desc_end])
                lyrics = try_decode2(content[desc_end+1:])
                key = f"USLT::{lang}"
                info[key] = lyrics

        # 썸네일
        elif frame_id == "APIC":
            parts = content[1:].split(b'\x00', 2)
            if len(parts) >= 3:
                mime = parts[0].decode(errors="ignore")
                image_data = parts[2]
                ext = ".jpg" if "jpeg" in mime else ".png"
                img_path = thumb_dir / (Path(filename).stem + ext)
                with open(img_path, "wb") as f:
                    f.write(image_data)
                info["APIC"] = f"/thumbs/{img_path.name}"

        pos += 10 + frame_size

    # 언어 우선순위 반영
    if "USLT::kor" in info:
        info["USLT"] = info["USLT::kor"]
    elif "USLT::eng" in info:
        info["USLT"] = info["USLT::eng"]

    if "COMM::kor" in info:
        info["COMM"] = info["COMM::kor"]
    elif "COMM::eng" in info:
        info["COMM"] = info["COMM::eng"]

    return info

def parse_id3v2_(data: bytes, filename: str):
    info = {}
    if not data.startswith(b"ID3"):
        return {"error": "ID3v2 header not found"}

    tag_size = int.from_bytes(data[6:10], byteorder="big") & 0x7F7F7F7F
    pos = 10

    while pos < 10 + tag_size:
        frame_id = data[pos:pos+4].decode("latin1", errors="ignore")
        frame_size = int.from_bytes(data[pos+4:pos+8], byteorder="big")
        if frame_size == 0 or not frame_id.strip():
            break

        content = data[pos+10:pos+10+frame_size]

        # 텍스트 프레임
        if frame_id.startswith("T") and len(content) > 1:
            enc = content[0]
            value = try_decode(content[1:])
            info[frame_id] = value

        # 가사 / 설명
        elif frame_id in ["USLT", "COMM"] and len(content) > 4:
            text_start = content.find(b'\x00', 4) + 1
            value = try_decode(content[text_start:])
            info[frame_id] = value

        # 썸네일
        elif frame_id == "APIC":
            parts = content[1:].split(b'\x00', 2)
            if len(parts) >= 3:
                mime = parts[0].decode(errors="ignore")
                image_data = parts[2]
                ext = ".jpg" if "jpeg" in mime else ".png"
                img_path = thumb_dir / (Path(filename).stem + ext)
                with open(img_path, "wb") as f:
                    f.write(image_data)
                info["APIC"] = f"/thumbs/{img_path.name}"

        pos += 10 + frame_size

    return info

def get_duration(file_data: bytes) -> float:
    try:
        audio = AudioSegment.from_file(io.BytesIO(file_data), format="mp3")
        return round(audio.duration_seconds, 2)
    except:
        return -1

@app.post("/upload")
async def upload(file: UploadFile = File(...)):
    contents = await file.read()
    path = upload_dir / file.filename
    path.write_bytes(contents)

    info = parse_id3v2(contents, file.filename)
    duration = get_duration(contents)
    info["DURATION"] = f"{duration} sec" if duration > 0 else "N/A"

    html = "<h2>MP3 Metadata</h2><ul>"
    for tag, label in {
        "TIT2": "제목", "TPE1": "아티스트", "TPE2": "앨범 아티스트", "TCOM": "작곡가",
        "TALB": "앨범", "TDRC": "발표연도", "TYER": "발표연도", "TCON": "장르",
        "TRCK": "트랙", "COMM": "설명", "USLT": "가사", "DURATION": "재생 시간"
    }.items():
        if tag in info:
            html += f"<li><b>{label}</b>: {info[tag]}</li>"

    if "APIC" in info:
        html += f'<li><b>썸네일</b>:<br><img src="{info["APIC"]}" width="200"></li>'
    html += "</ul>"

    return HTMLResponse(html)

@app.get("/", response_class=HTMLResponse)
def upload_form():
    return """
    <h1>MP3 파일 업로드</h1>
    <form action="/upload" enctype="multipart/form-data" method="post">
      <input type="file" name="file"><br><br>
      <input type="submit" value="분석">
    </form>
    """

app.mount("/thumbs", StaticFiles(directory="thumbs"), name="thumbs")

