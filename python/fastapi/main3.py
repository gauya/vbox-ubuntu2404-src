from fastapi import FastAPI, UploadFile, File
from fastapi.responses import HTMLResponse
from pathlib import Path
import os

app = FastAPI()
upload_dir = Path("uploads")
thumb_dir = Path("thumbs")
upload_dir.mkdir(exist_ok=True)
thumb_dir.mkdir(exist_ok=True)

def try_decode(data, encodings=["utf-8", "euc-kr", "cp949", "latin1"]):
    for enc in encodings:
        try:
            return data.decode(enc).strip('\x00')
        except:
            continue
    return "<decode error>"

def parse_id3v2(data: bytes, filename: str):
    info = {}
    if not data.startswith(b"ID3"):
        return {"error": "ID3v2 header not found"}

    version = data[3]
    flags = data[5]
    tag_size = int.from_bytes(data[6:10], "big")
    pos = 10

    while pos < 10 + tag_size:
        frame_id = data[pos:pos+4].decode("latin1", errors="ignore")
        frame_size = int.from_bytes(data[pos+4:pos+8], "big")
        frame_flags = data[pos+8:pos+10]
        content = data[pos+10:pos+10+frame_size]

        if frame_size == 0:
            break

        # TEXT frames
        if frame_id.startswith("T") and len(content) > 1:
            encoding = content[0]
            raw_text = content[1:]
            value = try_decode(raw_text)
            info[frame_id] = value

        # Comments (COMM) and Lyrics (USLT)
        elif frame_id in ["COMM", "USLT"] and len(content) > 4:
            encoding = content[0]
            lang = content[1:4].decode(errors="ignore")
            text_start = content.find(b'\x00', 4) + 1
            value = try_decode(content[text_start:])
            info[frame_id] = value

        # Album cover image (APIC)
        elif frame_id == "APIC":
            encoding = content[0]
            parts = content[1:].split(b'\x00', 2)
            if len(parts) >= 3:
                mime_type = parts[0].decode(errors="ignore")
                image_data = parts[2]
                ext = ".jpg" if "jpeg" in mime_type else ".png"
                img_path = thumb_dir / (Path(filename).stem + ext)
                with open(img_path, "wb") as f:
                    f.write(image_data)
                info["APIC"] = str(img_path)

        pos += 10 + frame_size

    return info

@app.post("/upload")
async def upload(file: UploadFile = File(...)):
    contents = await file.read()
    path = upload_dir / file.filename
    path.write_bytes(contents)
    meta = parse_id3v2(contents, file.filename)

    html = "<h2>MP3 Metadata</h2><ul>"
    for key in ["TIT2", "TPE1", "TALB", "TDRC", "TYER", "USLT", "COMM"]:
        if key in meta:
            html += f"<li><b>{key}</b>: {meta[key]}</li>"
    if "APIC" in meta:
        img_file = os.path.basename(meta["APIC"])
        html += f'<li><b>Album Cover:</b><br><img src="/thumbs/{img_file}" width="200"></li>'
    html += "</ul>"
    return HTMLResponse(html)

@app.get("/", response_class=HTMLResponse)
def form():
    return """
    <h1>Upload MP3 File</h1>
    <form action="/upload" enctype="multipart/form-data" method="post">
      <input type="file" name="file"><br><br>
      <input type="submit">
    </form>
    """

from fastapi.staticfiles import StaticFiles
app.mount("/thumbs", StaticFiles(directory="thumbs"), name="thumbs")

