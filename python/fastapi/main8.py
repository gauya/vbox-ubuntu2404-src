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
    for enc in ("utf-8", "euc-kr", "utf-16", "cp949", "latin1"):
        try:
            return b.decode(enc)
        except UnicodeDecodeError:
            continue
    return f"<binary:{len(b)}B>"

def parse_id3v2(data: bytes, filename: str):
    info = {}
    if not data.startswith(b"ID3"):
        return {"error": "ID3v2 header not found"}
        if len(data) > 128:
            head = data[-128:]
            if head[0:3] != "TAG":
                #print("may be not mp3")
                return {"error": "ID3v1,ID3v2 not"}
            info["TIT2"] = head[3:33]
            info["TPE1"] = head[33:63]
            info["TALB"] = head[63:93]
            info["TYER"] = head[93:97]
            info["COMM"] = head[97:127]
            return info


    tag_size = int.from_bytes(data[6:10], byteorder="big") & 0x7F7F7F7F
    pos = 10

    while pos < 10 + tag_size:
        frame_id = data[pos:pos+4].decode("latin1", errors="ignore")
        frame_size = int.from_bytes(data[pos+4:pos+8], byteorder="big")
        if frame_size == 0 or not frame_id.strip():
            break

        content = data[pos+10:pos+10+frame_size]

        # 텍스트 프레임 (예: TIT2, TPE1, TPE2, TCON, TCOP, TENC, TALB, TYER, TRCK, TPOS 등)
        if frame_id.startswith("T") and len(content) > 1:
            value = try_decode(content[1:])
            info[frame_id] = value

        # 설명 (COMM 프레임),  가사 (USLT 프레임)
        elif frame_id in ("COMM","USLT") and len(content) > 5:
            lang = content[1:4].decode("latin1") # ID3v2.4 에서는 utf-8
            desc_end = content.find(b'\x00', 9)
            if desc_end != -1:
                desc = try_decode(content[4:desc_end])
                text = try_decode(content[desc_end+1:])
                info[frame_id] = text

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

def try_decode2(data, encodings=["utf-8", "euc-kr", "cp949", "latin1"]):
    for enc in encodings:
        try:
            return data.decode(enc).strip('\x00')
        except:
            continue
    return "<decode error>"

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
    m = int(duration // 60)
    s = duration % 60
    #info["DURATION"] = f"{duration} sec" if duration > 0 else "N/A"
    info["DURATION"] = f"{m:02}:{s:5.2f}" if duration > 0 else "N/A"

    html = "<h2>MP3 Metadata</h2><ul>"
    for tag, label in {
        "TIT2": "제목", "TPE1": "아티스트", "TPE2": "앨범 아티스트", "TCOM": "작곡가",
        "TALB": "앨범", "TDRC": "발표연도", "TYER": "발표연도", "TCON": "장르",
        "TRCK": "트랙", "COMM": "설명", "USLT": "가사", "DURATION": "재생 시간"
    }.items():
        if tag in info:
            if tag == 'USLT':
              val=info[tag]
              info[tag] = val.replace("\n","<br>")
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

