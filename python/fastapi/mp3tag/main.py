from fastapi import FastAPI, UploadFile, File, Request
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from typing import List
from pathlib import Path
import os
import io
from pydub import AudioSegment
import struct
import base64

app = FastAPI()

# 템플릿 디렉토리 설정
templates = Jinja2Templates(directory="templates")

# 파일 저장 디렉토리 설정 및 생성
upload_dir = Path("uploads")
thumb_dir = Path("thumbs")
upload_dir.mkdir(exist_ok=True)
thumb_dir.mkdir(exist_ok=True)

# 정적 파일 서빙 (썸네일 이미지용)
app.mount("/thumbs", StaticFiles(directory=thumb_dir), name="thumbs")

# --- ID3v2 파싱 로직 ---

def synchsafe_int(data: bytes) -> int:
    """ID3v2 동기 안전 정수 (Synchsafe Integer)를 일반 정수로 변환합니다."""
    value = 0
    for byte in data:
        value = (value << 7) | (byte & 0x7F)
    return value

def try_decode(b: bytes, encoding_byte: int = 0):
    """한글 인코딩 깨짐 방지용 디코더."""
    for enc in ("utf-8", "euc-kr", "utf-16", "cp949", "latin1"):
        try:
            return b.decode(enc)
        except UnicodeDecodeError:
            #print(f"dec {enc} not")
            continue
    return f"<binary:{len(b)}B>"

def try_decode_1(b: bytes, encoding_byte: int = 0) -> str:
    """
    바이트 데이터를 지정된 인코딩 또는 여러 인코딩으로 디코딩합니다.
    ID3v2 텍스트 프레임의 인코딩 바이트를 고려합니다.
    """
    if encoding_byte == 0x00:
        encodings_to_try = ["iso-8859-1"]
    elif encoding_byte == 0x01:
        encodings_to_try = ["utf-16"] # BOM이 있을 경우 자동으로 처리
    elif encoding_byte == 0x02:
        encodings_to_try = ["utf-16-be"]
    elif encoding_byte == 0x03:
        encodings_to_try = ["utf-8"]
    else:
        encodings_to_try = ["utf-8", "euc-kr", "cp949", "latin1"]

    for enc in encodings_to_try:
        try:
            return b.decode(enc)
        except (UnicodeDecodeError, LookupError):
            continue
    return f"<binary:{len(b)}B>"

def parse_id3v2(data: bytes, filename: str) -> dict:
    info = {}
    
    if not data.startswith(b"ID3"):
        if len(data) >= 128:
            head = data[-128:]
            if head[0:3] == b"TAG":
                info["TIT2"] = try_decode(head[3:33].strip(b'\x00'), 0)
                info["TPE1"] = try_decode(head[33:63].strip(b'\x00'), 0)
                info["TALB"] = try_decode(head[63:93].strip(b'\x00'), 0)
                info["TYER"] = try_decode(head[93:97].strip(b'\x00'), 0)
                info["COMM"] = try_decode(head[97:127].strip(b'\x00'), 0)
                info["EXPL"] = "ID3v1"
                return info
        return {"error": "ID3 tag (v1 or v2) not found or invalid format."}

    major_version = data[3]
    revision_version = data[4]
    flags = data[5]

    if major_version == 0x04: # ID3v2.4
        tag_size = synchsafe_int(data[6:10])
        info["EXPL"] = f"ID3v2.4.{revision_version}"
    elif major_version == 0x03: # ID3v2.3
        tag_size = int.from_bytes(data[6:10], byteorder="big") & 0x7F7F7F7F
        info["EXPL"] = f"ID3v2.3.{revision_version}"
    elif major_version == 0x02: # ID3v2.2
        tag_size = int.from_bytes(data[6:10], byteorder="big")
        info["EXPL"] = f"ID3v2.2.{revision_version}"
    else:
        return {"error": f"Unsupported ID3v2 version: {major_version}. Only v2.2, v2.3, v2.4 are supported."}

    pos = 10

    if flags & 0x40: # Extended header flag
        if major_version == 0x04:
            if len(data) < pos + 4: return {"error": "Incomplete ID3v2.4 extended header size."}
            ext_header_size = synchsafe_int(data[pos:pos+4])
            pos += ext_header_size
        elif major_version == 0x03:
            if len(data) < pos + 4: return {"error": "Incomplete ID3v2.3 extended header size."}
            ext_header_size_raw = int.from_bytes(data[pos:pos+4], byteorder="big")
            pos += ext_header_size_raw + 4
    
    while pos < 10 + tag_size and pos + 10 <= len(data):
        frame_id_raw = data[pos:pos+4]
        
        if not frame_id_raw.strip(b'\x00') or not frame_id_raw.isalnum():
            break

        try:
            frame_id = frame_id_raw.decode("latin1", errors="ignore")
        except UnicodeDecodeError:
            break

        if major_version == 0x04:
            frame_size = synchsafe_int(data[pos+4:pos+8])
        else:
            frame_size = int.from_bytes(data[pos+4:pos+8], byteorder="big")
        
        if frame_size < 0 or pos + 10 + frame_size > len(data):
            break

        content_start = pos + 10
        content_end = content_start + frame_size
        content = data[content_start:content_end]

        if not content:
            pos += 10 + frame_size
            continue

        if frame_id.startswith("T") and frame_id not in ["TXXX", "TIPL", "TMCL", "TMOO", "TDEN", "TDOR"]:
            if len(content) > 0:
                encoding_byte = content[0]
                value = try_decode(content[1:], encoding_byte)
                if major_version == 0x04 and '\x00' in value:
                     value = value.replace('\x00', ' / ')
                info[frame_id] = value
        
        elif frame_id == "TXXX" and len(content) > 1:
            encoding_byte = content[0]
            null_byte_seq = b'\x00' if encoding_byte != 0x01 and encoding_byte != 0x02 else b'\x00\x00'
            
            null_byte_index = content.find(null_byte_seq, 1)
            if null_byte_index != -1:
                desc = try_decode(content[1:null_byte_index], encoding_byte)
                value = try_decode(content[null_byte_index+len(null_byte_seq):], encoding_byte)
                
                if "TXXX" not in info:
                    info["TXXX"] = {}
                info["TXXX"][desc] = value
            else:
                info[frame_id] = try_decode(content[1:], encoding_byte)

        elif frame_id in ("COMM", "USLT") and len(content) > 4:
            encoding_byte = content[0]
            lang = content[1:4].decode("latin1")
            
            null_byte_seq = b'\x00' if encoding_byte != 0x01 and encoding_byte != 0x02 else b'\x00\x00'
            
            desc_end_idx = content.find(null_byte_seq, 4)
            if desc_end_idx != -1:
                description = try_decode(content[4:desc_end_idx], encoding_byte)
                text_start_idx = desc_end_idx + len(null_byte_seq)
                text = try_decode(content[text_start_idx:], encoding_byte)
                
                info[frame_id] = text 
                if description:
                    info[f"{frame_id}_DESC"] = description
            else:
                 info[frame_id] = try_decode(content[4:], encoding_byte)

        elif frame_id == "APIC":
            if len(content) > 1:
                encoding_byte = content[0]
                
                mime_end_idx = content.find(b'\x00', 1)
                if mime_end_idx != -1:
                    mime_type = content[1:mime_end_idx].decode("latin1", errors="ignore")
                    
                    picture_type = content[mime_end_idx + 1]
                    
                    desc_start_idx = mime_end_idx + 2
                    null_byte_seq = b'\x00' if encoding_byte != 0x01 and encoding_byte != 0x02 else b'\x00\x00'
                    desc_end_idx = content.find(null_byte_seq, desc_start_idx)
                    
                    if desc_end_idx != -1:
                        description = try_decode(content[desc_start_idx:desc_end_idx], encoding_byte)
                        image_data_start = desc_end_idx + len(null_byte_seq)
                        image_data = content[image_data_start:]

                        ext = ".jpg" if "jpeg" in mime_type else ".png" if "png" in mime_type else ".bin"
                        img_path = thumb_dir / (Path(filename).stem + ext)
                        try:
                            with open(img_path, "wb") as f:
                                f.write(image_data)
                            info["APIC"] = f"/thumbs/{img_path.name}"
                        except Exception as e:
                            print(f"Error saving image: {e}")
                            info["APIC_ERROR"] = "Failed to save image"
                    else:
                        info["APIC_ERROR"] = "APIC description null byte not found"
                else:
                    info["APIC_ERROR"] = "APIC MIME type null byte not found"

        elif frame_id == "PRIV":
            null_byte_idx = content.find(b'\x00')
            if null_byte_idx != -1:
                owner_id = try_decode(content[:null_byte_idx], 0)
                private_data = content[null_byte_idx+1:]
                info[frame_id] = f"Owner: {owner_id}, Data (Base64): {base64.b64encode(private_data).decode('ascii')}"
            else:
                info[frame_id] = f"Data (Base64): {base64.b64encode(content).decode('ascii')}"

        elif frame_id == "WXXX":
            if len(content) > 1:
                encoding_byte = content[0]
                null_byte_seq = b'\x00' if encoding_byte != 0x01 and encoding_byte != 0x02 else b'\x00\x00'
                
                null_byte_index = content.find(null_byte_seq, 1)
                if null_byte_index != -1:
                    desc = try_decode(content[1:null_byte_index], encoding_byte)
                    url = try_decode(content[null_byte_index+len(null_byte_seq):], encoding_byte)
                    info[frame_id] = url
                    if desc:
                        info[f"{frame_id}_DESC"] = desc
                else:
                    info[frame_id] = try_decode(content[1:], encoding_byte)
        
        pos += 10 + frame_size

    return info

def get_duration(file_data: bytes) -> float:
    try:
        audio = AudioSegment.from_file(io.BytesIO(file_data), format="mp3")
        return round(audio.duration_seconds, 2)
    except Exception as e:
        print(f"Error getting duration: {e}")
        return -1

@app.post("/upload-multiple") # 여러 파일을 받을 수 있는 새로운 엔드포인트
async def upload_multiple_files(request: Request, files: List[UploadFile] = File(...)):
    results = []
    
    for file in files:
        contents = await file.read()
        file_path = upload_dir / file.filename
        file_path.write_bytes(contents)

        info = parse_id3v2(contents, file.filename)
        duration = get_duration(contents)
        m = int(duration // 60)
        s = duration % 60
        info["DURATION"] = f"{m:02}:{s:05.2f}" if duration > 0 else "N/A"
        
        # 각 파일의 메타데이터를 HTML로 렌더링
        rendered_html = templates.TemplateResponse(
            "results_content.html",
            {"request": request, "info": info}
        ).body.decode('utf-8')
        
        results.append({
            "filename": file.filename,
            "html_content": rendered_html
        })

    # 모든 파일의 결과를 JSON으로 클라이언트에 반환
    return JSONResponse(content={"message": f"총 {len(results)}개의 파일 분석 완료", "results": results})

@app.get("/", response_class=HTMLResponse)
async def upload_form(request: Request):
    return templates.TemplateResponse("base.html", {"request": request})

