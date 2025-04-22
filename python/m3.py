from mutagen.mp3 import MP3
from mutagen.id3 import ID3, ID3NoHeaderError
import os

# 흔히 쓰이는 문자셋 리스트
COMMON_ENCODINGS = ["utf-8", "euc-kr", "cp949", "latin1", "iso-8859-1"]

def try_decode(text):
    """텍스트가 bytes 형태로 되어 있으면 가능한 인코딩으로 디코딩 시도"""
    if isinstance(text, bytes):
        for encoding in COMMON_ENCODINGS:
            try:
                return text.decode(encoding)
            except:
                continue
        return str(text)  # 최종 실패 시 raw bytes 출력
    return text

def read_metadata(filepath):
    try:
        audio = MP3(filepath)
        tags = ID3(filepath)

        print(f"\n📄 파일: {os.path.basename(filepath)}")

        for frame_id, frame in tags.items():
            if hasattr(frame, 'text'):
                texts = []
                for text in frame.text:
                    decoded = try_decode(text)
                    texts.append(decoded)
                if isinstance(texts, list):
                    print(f"{frame_id}: {''.join(texts)}")
                else:
                    print(f"{frame_id}: {str(texts)}")

            if frame_id in ['TDRC', 'TDOR', 'TDRL']:
                print(f"{frame_id}: {audio[frame_id].text}")

        print("-" * 50)

    except ID3NoHeaderError:
        print(f"{filepath}: ID3 태그가 없습니다.")
    except Exception as e:
        print(f"{filepath}: 오류 발생 - {e}")

# 예시 파일 경로
# mp3_path = "sample.mp3"
# read_metadata(mp3_path)

# 또는 디렉터리 내 모든 mp3파일 처리
def scan_folder(folder_path):
    for filename in os.listdir(folder_path):
        if filename.lower().endswith(".mp3"):
            read_metadata(os.path.join(folder_path, filename))

# 사용 예
# scan_folder("/path/to/mp3/folder")

import sys
if __name__ == '__main__':
    scan_folder(sys.argv[1])

