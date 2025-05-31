from fastapi import FastAPI, UploadFile, File, Request
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates # 템플릿 사용을 위해 추가
from pathlib import Path
import os
import io
from pydub import AudioSegment
import base64
import struct # ID3v2.4 동기 안전 정수 처리를 위해 추가

app = FastAPI()

# 템플릿 디렉토리 설정 (templates 폴더 생성 필요)
templates = Jinja2Templates(directory="templates")

upload_dir = Path("uploads")
thumb_dir = Path("thumbs")
upload_dir.mkdir(exist_ok=True)
thumb_dir.mkdir(exist_ok=True)

# StaticFiles를 사용하여 `thumbs` 디렉토리를 `/thumbs` URL로 서빙합니다.
# `uploads` 디렉토리도 필요하다면 유사하게 추가할 수 있습니다.
app.mount("/thumbs", StaticFiles(directory=thumb_dir), name="thumbs")


# --- ID3v2.4 파싱 로직 개선 (동기 안전 정수 및 UTF-8 처리) ---
def synchsafe_int(data: bytes) -> int:
    """ID3v2 동기 안전 정수를 일반 정수로 변환합니다."""
    value = 0
    for byte in data:
        value = (value << 7) | (byte & 0x7F)
    return value

def try_decode(b: bytes, encoding_byte: int = 0):
    """한글 인코딩 깨짐 방지용 디코더 (ID3v2.4 인코딩 바이트 고려)."""
    # ID3v2.4 text encoding byte:
    # $00 – ISO-8855-1
    # $01 – UTF-16 (with BOM)
    # $02 – UTF-16 BE (without BOM)
    # $03 – UTF-8
    
    if encoding_byte == 0x00:
        encodings_to_try = ["iso-8859-1"] # ID3v2.3 기본
    elif encoding_byte == 0x01:
        encodings_to_try = ["utf-16"] # UTF-16 with BOM
    elif encoding_byte == 0x02:
        encodings_to_try = ["utf-16-be"] # UTF-16 BE without BOM
    elif encoding_byte == 0x03:
        encodings_to_try = ["utf-8"] # ID3v2.4 기본
    else: # Fallback for unknown/invalid encoding bytes or non-text frames
        encodings_to_try = ["utf-8", "euc-kr", "utf-16", "cp949", "latin1"]

    for enc in encodings_to_try:
        try:
            return b.decode(enc)
        except (UnicodeDecodeError, LookupError): # LookupError for unknown enc
            continue
    return f"<binary:{len(b)}B>"

def parse_id3v2(data: bytes, filename: str):
    info = {}
    if not data.startswith(b"ID3"):
        # ID3v1 태그는 파일 끝 128바이트에 있을 수 있습니다.
        if len(data) >= 128:
            head = data[-128:]
            if head[0:3] == b"TAG": # ID3v1.x 태그 확인
                info["TIT2"] = try_decode(head[3:33].strip(b'\x00'))
                info["TPE1"] = try_decode(head[33:63].strip(b'\x00'))
                info["TALB"] = try_decode(head[63:93].strip(b'\x00'))
                info["TYER"] = try_decode(head[93:97].strip(b'\x00'))
                info["COMM"] = try_decode(head[97:127].strip(b'\x00'))
                info["EXPL"] = "ID3v1"
                return info
        return {"error": "ID3v2 tag not found or invalid format"} # ID3v1도 없으면 오류

    major_version = data[3]
    revision_version = data[4]
    
    # ID3v2.3은 태그 크기만 동기 안전 정수, 프레임 크기는 일반 정수
    # ID3v2.4는 모든 크기가 동기 안전 정수
    if major_version == 0x04: # ID3v2.4
        tag_size = synchsafe_int(data[6:10])
        info["EXPL"] = f"ID3v2.4.{revision_version}"
    elif major_version == 0x03: # ID3v2.3
        tag_size = int.from_bytes(data[6:10], byteorder="big") & 0x7F7F7F7F # 일반 32비트 정수 (상위 비트 0으로 마스킹)
        info["EXPL"] = f"ID3v2.3.{revision_version}"
    elif major_version == 0x02: # ID3v2.2 (v2.3과 유사하게 파싱)
        tag_size = int.from_bytes(data[6:10], byteorder="big") # 2.2는 6~9 바이트가 태그 크기
        info["EXPL"] = f"ID3v2.2.{revision_version}"
    else:
        return {"error": f"Unsupported ID3v2 version: {major_version}"}

    pos = 10 # ID3v2 헤더 다음부터 시작

    # 확장 헤더 처리 (v2.4에 있을 수 있음)
    flags = data[5]
    if flags & 0x40: # Extended header flag is set
        if major_version == 0x04:
            ext_header_size = synchsafe_int(data[10:14])
        else: # For v2.3, this would be a regular int
            ext_header_size = int.from_bytes(data[10:14], byteorder="big")
        pos += ext_header_size # Skip extended header

    # 프레임 파싱 루프
    while pos < 10 + tag_size:
        # 4바이트 프레임 ID, 4바이트 크기, 2바이트 플래그 (v2.3, v2.4)
        # 3바이트 프레임 ID, 3바이트 크기 (v2.2)
        
        # 최소 10바이트 (ID + Size + Flags) 이상 남아있어야 함
        if pos + 10 > len(data):
            break

        frame_id_raw = data[pos:pos+4]
        if not frame_id_raw.strip(b'\x00'): # 널 바이트로 채워진 프레임 (패딩)
            break
        
        try:
            frame_id = frame_id_raw.decode("latin1", errors="ignore")
        except UnicodeDecodeError:
            # 유효하지 않은 프레임 ID (손상된 태그 등)
            break

        if major_version == 0x04: # ID3v2.4
            frame_size = synchsafe_int(data[pos+4:pos+8])
        else: # ID3v2.3 또는 2.2
            frame_size = int.from_bytes(data[pos+4:pos+8], byteorder="big")
        
        # 유효하지 않은 프레임 크기 (무한 루프 방지)
        if frame_size <= 0 or pos + 10 + frame_size > len(data):
            break

        frame_flags = data[pos+8:pos+10] # 프레임 플래그 (ID3v2.3/2.4)
        
        # 실제 프레임 내용 (텍스트 인코딩 바이트 포함)
        content_start = pos + 10
        content_end = content_start + frame_size
        content = data[content_start:content_end]

        if not content: # 내용이 없으면 건너뜀
            pos += 10 + frame_size
            continue

        # 텍스트 프레임 (TXXX, TCON, TIT2, TPE1 등)
        if frame_id.startswith("T") and frame_id not in ["TXXX", "TSOA", "TSOP", "TSOC", "TPRO", "TIPL", "TMCL", "TMOO"] : # 특수 T-프레임 제외
            if len(content) > 0: # 인코딩 바이트 존재 가정
                encoding_byte = content[0]
                value = try_decode(content[1:], encoding_byte)
                # v2.4에서 다중 값은 널 바이트로 구분됨
                if major_version == 0x04 and '\x00' in value:
                     value = value.replace('\x00', ', ') # 널 바이트를 쉼표로 대체하여 출력
                info[frame_id] = value
        
        # 사용자 정의 텍스트 프레임 (TXXX)
        elif frame_id == "TXXX" and len(content) > 1:
            encoding_byte = content[0]
            # 텍스트 설명 (description)과 값 (value)이 널 바이트로 구분됨
            null_byte_index = content.find(b'\x00', 1)
            if null_byte_index != -1:
                desc = try_decode(content[1:null_byte_index], encoding_byte)
                value = try_decode(content[null_byte_index+1:], encoding_byte)
                # 여러 TXXX 프레임이 있을 수 있으므로 리스트로 관리하거나 합쳐서 출력
                if "TXXX" not in info:
                    info["TXXX"] = {}
                info["TXXX"][desc] = value
            else: # 널 바이트가 없으면 전체가 값으로 처리
                info[frame_id] = try_decode(content[1:], encoding_byte)

        # 설명 (COMM), 가사 (USLT) 프레임
        elif frame_id in ("COMM", "USLT") and len(content) > 4: # 인코딩 바이트(1) + 언어(3) = 4바이트
            encoding_byte = content[0]
            lang = content[1:4].decode("latin1") # 언어 코드는 항상 latin1
            
            # 설명 또는 내용 끝의 널 바이트를 찾음
            # v2.3과 v2.4는 텍스트 인코딩에 따라 널 바이트가 다름 (0x00 또는 0x00 0x00)
            null_byte_seq = b'\x00'
            if encoding_byte in (0x01, 0x02): # UTF-16
                null_byte_seq = b'\x00\x00'
            
            desc_end_idx = content.find(null_byte_seq, 4) # 4바이트 (encoding + lang) 이후부터 찾기
            if desc_end_idx != -1:
                description = try_decode(content[4:desc_end_idx], encoding_byte)
                text_start_idx = desc_end_idx + len(null_byte_seq)
                text = try_decode(content[text_start_idx:], encoding_byte)
                
                info[frame_id] = text # 보통 description은 중요하지 않으므로 본문만 저장
                if description: # 설명이 있다면 추가
                    info[f"{frame_id}_DESC"] = description
            else: # 널 바이트가 없으면 전체를 텍스트로 처리
                 info[frame_id] = try_decode(content[4:], encoding_byte)


        # 썸네일 (APIC 프레임)
        elif frame_id == "APIC":
            if len(content) > 1: # 인코딩 바이트 존재
                encoding_byte = content[0] # APIC 프레임의 텍스트 인코딩 바이트 (MIME, description)
                
                # MIME 타입, 이미지 형식, 설명(description), 이미지 데이터
                # MIME 타입 끝의 널 바이트 찾기
                mime_end_idx = content.find(b'\x00', 1) # 인코딩 바이트 다음부터
                if mime_end_idx != -1:
                    mime_type = content[1:mime_end_idx].decode("latin1", errors="ignore")
                    
                    # 그림 유형 바이트 (1바이트)
                    picture_type = content[mime_end_idx + 1]
                    
                    # 설명(description) 끝의 널 바이트 찾기
                    desc_start_idx = mime_end_idx + 2 # MIME 타입 + 널 + 그림 유형 바이트 다음
                    desc_end_idx = content.find(b'\x00', desc_start_idx)
                    
                    if desc_end_idx != -1:
                        description = try_decode(content[desc_start_idx:desc_end_idx], encoding_byte)
                        image_data_start = desc_end_idx + 1 # 널 바이트 다음
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

        # 웹 주소 (WXXX) 프레임
        elif frame_id == "WXXX" and len(content) > 1:
            encoding_byte = content[0]
            null_byte_index = content.find(b'\x00', 1)
            if null_byte_index != -1:
                desc = try_decode(content[1:null_byte_index], encoding_byte)
                url = try_decode(content[null_byte_index+1:], encoding_byte)
                info[frame_id] = url
                if desc:
                    info[f"{frame_id}_DESC"] = desc
            else: # 설명이 없으면 전체가 URL
                info[frame_id] = try_decode(content[1:], encoding_byte)

        # PRIV (개인 프레임) - 데이터는 바이너리 그대로 저장
        elif frame_id == "PRIV":
            # PRIV 프레임은 owner ID와 데이터로 구성됩니다.
            # owner ID는 널 바이트로 끝나는 문자열입니다.
            null_byte_idx = content.find(b'\x00')
            if null_byte_idx != -1:
                owner_id = try_decode(content[:null_byte_idx], 0) # owner ID는 보통 ISO-8859-1
                private_data = content[null_byte_idx+1:]
                # private_data는 바이너리이므로 화면에 출력하기 위해 Base64 인코딩
                info[frame_id] = f"Owner: {owner_id}, Data (Base64): {base64.b64encode(private_data).decode('ascii')}"
            else: # 널 바이트가 없으면 전체가 데이터
                info[frame_id] = f"Data (Base64): {base64.b64encode(content).decode('ascii')}"


        # 다음 프레임 위치로 이동
        pos += 10 + frame_size # 프레임 헤더(10) + 내용 크기

    return info

def get_duration(file_data: bytes) -> float:
    try:
        audio = AudioSegment.from_file(io.BytesIO(file_data), format="mp3")
        return round(audio.duration_seconds, 2)
    except Exception as e:
        print(f"Error getting duration: {e}")
        return -1

@app.post("/upload")
async def upload_file(request: Request, file: UploadFile = File(...)):
    contents = await file.read()
    file_path = upload_dir / file.filename
    file_path.write_bytes(contents) # 파일을 서버에 저장 (실제 서비스에서는 보안 고려 필요)

    info = parse_id3v2(contents, file.filename)
    duration = get_duration(contents)
    m = int(duration // 60)
    s = duration % 60
    info["DURATION"] = f"{m:02}:{s:05.2f}" if duration > 0 else "N/A"

    # HTML 응답을 Jinja2 템플릿으로 렌더링
    # upload.html 템플릿 내의 results_content 블록만 업데이트되도록 설계
    return templates.TemplateResponse(
        "results_content.html",
        {"request": Request, "info": info} # request 객체 필요
    )

@app.get("/", response_class=HTMLResponse)
async def upload_form(request: Request):
    # base.html 템플릿을 렌더링하여 드래그 앤 드롭 UI를 포함한 전체 페이지를 제공
    return templates.TemplateResponse("base.html", {"request": request})

# 'templates' 디렉토리에 Jinja2 템플릿을 저장합니다.
# `public` 폴더도 생성하여 JS, CSS 파일을 저장할 수 있습니다.
# app.mount("/static", StaticFiles(directory="public"), name="static")

