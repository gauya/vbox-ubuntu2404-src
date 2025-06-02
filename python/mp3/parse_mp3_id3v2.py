#! /usr/bin/python
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
        if len(data) > 128 and data[0:3] == "TAG":
            head = data[-128:]
            info["TIT2"] = head[3:33]
            info["TPE1"] = head[33:63]
            info["TALB"] = head[63:93]
            info["TYER"] = head[93:97]
            info["COMM"] = head[97:127]

            info["EXPL"] = f"ID3v1"
        
            print(f"--{filename}-- ID3v1 tagsize=128")
            
            return info
        
        print("may be not mp3")
        return {"error": "ID3v1 not"}

    tag_size = int.from_bytes(data[6:10], byteorder="big") & 0x7F7F7F7F
    majar_ver = data[3]
    info["EXPL"] = f"ID3v2.{majar_ver}"
    pos = 10

    print(f"--{filename}-- ID3v2.{majar_ver} tagsize={tag_size}")
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

        elif frame_id in ("PRIV","WXXX"):
            parts = content[4:frame_size]
            info[frame_id] = parts.decode('utf-8')

        pos += 10 + frame_size

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
