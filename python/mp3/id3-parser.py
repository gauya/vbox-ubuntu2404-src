import os
import sys

def parse_id3v2_header(data):
    if data[:3] != b'ID3':
        return None

    version = data[3]
    revision = data[4]
    flags = data[5]
    size_bytes = data[6:10]

    def syncsafe_to_int(syncsafe_bytes):
        return (
            (syncsafe_bytes[0] & 0x7F) << 21 |
            (syncsafe_bytes[1] & 0x7F) << 14 |
            (syncsafe_bytes[2] & 0x7F) << 7 |
            (syncsafe_bytes[3] & 0x7F)
        )

    tag_size = syncsafe_to_int(size_bytes)
    return {
        "version": f"ID3v2.{version}.{revision}",
        "flags": flags,
        "size": tag_size,
        "header_size": 10
    }

def extract_id3v2_frames(data, tag_size, output_dir, filename_prefix="frame"):
    i = 10
    frames = {}
    while i < tag_size:
        frame_id = data[i:i+4].decode(errors='ignore')
        frame_size = int.from_bytes(data[i+4:i+8], byteorder='big')
        frame_flags = data[i+8:i+10]
        frame_content = data[i+10:i+10+frame_size]

        if not frame_id.strip("\x00"):
            break

        if frame_id.startswith("T"):  # 텍스트 프레임
            try:
                encoding = frame_content[0]
                if encoding == 0:
                    content = frame_content[1:].decode("latin1")
                elif encoding == 1:
                    content = frame_content[1:].decode("utf-16")
                elif encoding == 3:
                    content = frame_content[1:].decode("utf-8")
                else:
                    content = f"<Unknown encoding: {encoding}>"
            except Exception as e:
                content = f"<Decode error: {str(e)}>"
        else:
            # 이진 데이터 프레임 저장
            safe_id = frame_id.strip().replace("/", "_")
            bin_filename = os.path.join(output_dir, f"{filename_prefix}_{safe_id}.bin")
            with open(bin_filename, "wb") as f:
                f.write(frame_content)
            content = f"<Binary data: {len(frame_content)} bytes> → saved as {bin_filename}"

        frames[frame_id] = content
        i += 10 + frame_size

    return frames

def main(mp3_path):
    if not os.path.exists(mp3_path):
        print(f"File not found: {mp3_path}")
        return

    with open(mp3_path, "rb") as f:
        data = f.read()

    id3_header = parse_id3v2_header(data)
    if not id3_header:
        print("No ID3v2 tag found.")
        return

    output_dir = os.path.join(os.path.dirname(mp3_path), "id3_extracted")
    os.makedirs(output_dir, exist_ok=True)

    print(f"ID3 Version: {id3_header['version']}")
    print(f"Tag Size: {id3_header['size']} bytes")

    frames = extract_id3v2_frames(data, id3_header["size"], output_dir, os.path.basename(mp3_path))
    print("\n--- Extracted Frames ---")
    for frame_id, content in frames.items():
        print(f"{frame_id}: {content}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python mp3_id3_parser.py [filename.mp3]")
    else:
        main(sys.argv[1])

