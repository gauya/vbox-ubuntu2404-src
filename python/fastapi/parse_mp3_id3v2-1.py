import sys
import os
import struct

def try_decode(b: bytes):
    """한글 인코딩 깨짐 방지용 디코더."""
    for enc in ("utf-8", "euc-kr", "utf-16", "cp949", "latin1"):
        try:
            return b.decode(enc)
        except UnicodeDecodeError:
            #print(f"dec {enc} not")
            continue
    return f"<binary:{len(b)}B>"

def parse_id3v2(data: bytes, filename: str):
    info = {}
    if not data.startswith(b"ID3"):
        #return {"error": "ID3v2 header not found"}
        print(f"ID3V2 not")
        # check ID3V1
        if len(data) > 128 and head[0:3] == "TAG":
            head = data[-128:]
            info["TIT2"] = head[3:33]
            info["TPE1"] = head[33:63]
            info["TALB"] = head[63:93]
            info["TYER"] = head[93:97]
            info["COMM"] = head[97:127]
            return info
        print("may be not mp3")
        return {"error": "ID3v1 not"}

    tag_size = int.from_bytes(data[6:10], byteorder="big") & 0x7F7F7F7F
    pos = 10

    print(f"--{filename}-- tagsize={tag_size}")
    while pos < 10 + tag_size:
        frame_id = data[pos:pos+4].decode("latin1", errors="ignore")
        frame_size = int.from_bytes(data[pos+4:pos+8], byteorder="big")
        print(f"frame_id={frame_id} {frame_size}")
        if frame_size == 0 or not frame_id.strip():
            break

        content = data[pos+10:pos+10+frame_size]

        # 텍스트 프레임 (예: TIT2, TPE1, TPE2, TCON, TCOP, TENC, TALB, TYER, TRCK, TPOS 등)
        if frame_id.startswith("T") and len(content) > 1:
            value = try_decode(content[1:])
            info[frame_id] = value

        # 설명 (COMM 프레임), 가사 (USLT 프레임)
        elif frame_id in ("COMM","USLT") and len(content) > 5:
            lang = content[1:4].decode("latin1")
            print(f"COMM.....lang={lang}")
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
                img_path = filename + ext
                with open(img_path, "wb") as f:
                    f.write(image_data)
                info["APIC"] = f"/thumbs/{img_path}"

        pos += 10 + frame_size

    return info

def decode_text(data: bytes, encoding_byte: int):
    encodings = {
        0: "latin1",
        1: "utf-16",
        2: "utf-16be",
        3: "utf-8"
    }
    encoding = encodings.get(encoding_byte, "latin1")
    try:
        return data.decode(encoding).strip()
    except:
        return f"<decode_error:{len(data)}B>"

def parse_text_frame(frame_id, content, info):
    encoding = content[0]
    lang = content[1:4].decode("latin1")
    qlast = 9 if frame_id in ('COMM','USLT') else 4
    desc_end = content.find(b'\x00', qlast)
    print(f"{frame_id} : lang={lang} desc_end={desc_end}")
    if desc_end == -1:
        desc_end = 4  # fallback
    #desc = decode_text(content[4:desc_end], encoding)
    #body = decode_text(content[desc_end+1:], encoding)
    desc = try_decode(content[4:desc_end])
    body = try_decode(content[desc_end+1:])

    key = f"{frame_id}::{lang}"
    info[key] = body

    # USLT/COMM default 저장
    if frame_id == "USLT" and "USLT" not in info and lang in ("kor", "eng"):
        info["USLT"] = body
    if frame_id == "COMM" and "COMM" not in info and lang in ("kor", "eng"):
        info["COMM"] = body

def parse_id3v2_(file_path):
    info = {}
    with open(file_path, "rb") as f:
        header = f.read(10)
        if header[0:3] != b"ID3":
            print("ID3 태그 없음.")
            return {}

        version = header[3]
        tag_size = ((header[6] & 0x7f) << 21) | ((header[7] & 0x7f) << 14) | ((header[8] & 0x7f) << 7) | (header[9] & 0x7f)

        offset = 10
        print(f"{file_path} : {version} tagsize={tag_size}")
        while offset < tag_size:
            f.seek(offset)
            frame_header = f.read(10)
            if len(frame_header) < 10 or frame_header[0] == 0:
                print(f"end {len(frame_header)} {frame_header[0]}")
                break

            frame_id = frame_header[0:4].decode("latin1")
            frame_size = struct.unpack(">I", frame_header[4:8])[0]
            content = f.read(frame_size)

            print(f"{frame_id} size={frame_size}")

            if frame_id in ("TIT2","TPE1","TYER","TENC","TALB","TDRC","TPE2","TCON","TRCK","TCOM"):
                text = decode_text(content[1:], content[0])
                info[frame_id] = text
            elif frame_id in ("USLT", "COMM"):
                parse_text_frame(frame_id, content, info)
            elif frame_id == "APIC":
                mime_end = content.find(b'\x00', 1)
                encoding = content[0]
                mime = content[1:mime_end].decode("latin1")
                pic_type = content[mime_end + 1]
                desc_end = content.find(b'\x00', mime_end + 2)
                img_data = content[desc_end + 1:]

                ext = "jpg" if mime.endswith("jpeg") else "png"
                filename = f"{os.path.splitext(file_path)[0]}_thumbnail.{ext}"
                with open(filename, "wb") as img_file:
                    img_file.write(img_data)
                info["APIC"] = f"{filename} ({len(img_data)} bytes)"
            else:
                # 바이너리 프레임 저장
                binname = f"{os.path.splitext(file_path)[0]}_{frame_id}.bin"
                with open(binname, "wb") as binf:
                    binf.write(content)
                info[frame_id] = f"<binary:{len(content)}B, saved as {os.path.basename(binname)}>"
            offset += 10 + frame_size
    return info

def print_metadata(info):
    print("\n📋 MP3 메타데이터")
    for k, v in info.items():
        print(f"{k}: {v if isinstance(v, str) else '<binary>'}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("사용법: python parse_mp3_id3v2.py yourfile.mp3")
        sys.exit(1)

    path = sys.argv[1]
    with open(path,"rb") as f:
      dat = f.read()
    result = parse_id3v2(dat,path)
    print_metadata(result)
