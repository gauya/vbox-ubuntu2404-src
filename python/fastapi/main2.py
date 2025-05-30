from fastapi import FastAPI, File, UploadFile, Request
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
import os

app = FastAPI()
app.mount("/static", StaticFiles(directory="static"), name="static")
templates = Jinja2Templates(directory="templates")

def try_decode(content, fallback_encodings=["utf-8", "euc-kr", "cp949", "latin1"]):
    for enc in fallback_encodings:
        try:
            return content.decode(enc)
        except:
            continue
    return "<decode error>"

def parse_id3v2(data, output_prefix="static"):
    def syncsafe_to_int(syncsafe_bytes):
        return (
            (syncsafe_bytes[0] & 0x7F) << 21 |
            (syncsafe_bytes[1] & 0x7F) << 14 |
            (syncsafe_bytes[2] & 0x7F) << 7 |
            (syncsafe_bytes[3] & 0x7F)
        )

    if data[:3] != b'ID3':
        return None, {}

    version = data[3]
    revision = data[4]
    tag_size = syncsafe_to_int(data[6:10])
    frames = {}
    i = 10

    while i < 10 + tag_size:
        frame_id = data[i:i+4].decode(errors='ignore')
        if not frame_id.strip("\x00"):
            break

        frame_size = int.from_bytes(data[i+4:i+8], 'big')
        content = data[i+10:i+10+frame_size]
        if frame_id.startswith("T"):
            if frame_size > 1:
              encoding_byte = content[0]
              raw_text = content[1:]
              if encoding_byte == 0:
                  value = try_decode(raw_text, ["latin1", "cp949", "euc-kr"])
              elif encoding_byte == 1:
                  try:
                      value = raw_text.decode("utf-16")
                  except:
                      value = try_decode(raw_text, ["utf-16", "utf-8", "cp949"])
              elif encoding_byte == 3:
                  value = try_decode(raw_text, ["utf-8", "euc-kr", "cp949"])
              else:
                  value = f"<Unknown encoding byte: {encoding_byte}>"            
            else:
  
              try:
                  encoding = content[0]
                  if encoding == 0:
                      value = content[1:].decode('latin1')
                  elif encoding == 1:
                      value = content[1:].decode('utf-16')
                  elif encoding == 3:
                      value = content[1:].decode('utf-8')
                  else:
                      value = f"<Unknown encoding: {encoding}>"
              except:
                  value = "<Decode error>"
        else:
            os.makedirs(output_prefix, exist_ok=True)
            filename = f"{output_prefix}/{frame_id}.bin"
            with open(filename, "wb") as f:
                f.write(content)
            value = f"<Binary: {len(content)} bytes> → saved: {filename}"

        frames[frame_id] = value
        i += 10 + frame_size

    return f"ID3v2.{version}.{revision}", frames

@app.get("/", response_class=HTMLResponse)
async def upload_form():
    return """
    <html><body>
        <h2>Upload an MP3 file to analyze ID3 tags</h2>
        <form action="/analyze" method="post" enctype="multipart/form-data">
            <input type="file" name="file">
            <input type="submit" value="Analyze">
        </form>
    </body></html>
    """

@app.post("/analyze", response_class=HTMLResponse)
async def analyze(request: Request, file: UploadFile = File(...)):
    content = await file.read()
    version, frames = parse_id3v2(content)
    return templates.TemplateResponse("result.html", {
        "request": request,
        "filename": file.filename,
        "version": version,
        "frames": frames
    })

# uvicorn main:app --reload

