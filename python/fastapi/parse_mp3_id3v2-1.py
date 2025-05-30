import sys
import os
import struct

def try_decode(b: bytes):
    """한글 인코딩 깨짐 방지용 디코더."""
    for enc in ("utf-8", "euc-kr", "utf-16", "cp949", "latin1"):
        try:
            return b.decode(enc)
        except UnicodeDecodeError:
            continue
    return f"<binary:{len(b)}B>"

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
    desc_end = content.find(b'\x00', 4)
    if desc_end == -1:
        desc_end = 4  # fallback
    desc = decode_text(content[4:desc_end], encoding)
    body = decode_text(content[desc_end+1:], encoding)

    key = f"{frame_id}::{lang}"
    info[key] = body

    # USLT/COMM default 저장
    if frame_id == "USLT" and "USLT" not in info and lang in ("kor", "eng"):
        info["USLT"] = body
    if frame_id == "COMM" and "COMM" not in info and lang in ("kor", "eng"):
        info["COMM"] = body

def parse_id3v2(file_path):
    info = {}
    with open(file_path, "rb") as f:
        header = f.read(10)
        if header[0:3] != b"ID3":
            print("ID3 태그 없음.")
            return {}

        version = header[3]
        tag_size = ((header[6] & 0x7f) << 21) | ((header[7] & 0x7f) << 14) | ((header[8] & 0x7f) << 7) | (header[9] & 0x7f)

        offset = 10
        while offset < tag_size:
            f.seek(offset)
            frame_header = f.read(10)
            if len(frame_header) < 10 or frame_header[0] == 0:
                break

            frame_id = frame_header[0:4].decode("latin1")
            frame_size = struct.unpack(">I", frame_header[4:8])[0]
            content = f.read(frame_size)

            if frame_id in ("TIT2", "TPE1", "TALB", "TDRC", "TPE2", "TCON", "TRCK", "TCOM"):
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
    result = parse_id3v2(path)
    print_metadata(result)
