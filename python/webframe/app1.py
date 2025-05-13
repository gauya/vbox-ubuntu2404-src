from flask import Flask, request, jsonify
from flask_cors import CORS
from mutagen.mp3 import MP3
from mutagen.id3 import ID3NoHeaderError, ID3TagError  # ID3TagError 추가

app = Flask(__name__)
CORS(app)

@app.route('/metadata/<path:filepath>', methods=['GET'])
def get_metadata(filepath):
    try:
        audio = MP3(filepath)
        metadata = {}
        # 태그가 존재하는지 확인하고, 존재할 경우에만 값을 가져오도록 수정
        if 'TIT2' in audio:
            metadata['title'] = str(audio['TIT2'][0])  # 리스트에서 첫 번째 값 추출
        if 'TPE1' in audio:
            metadata['artist'] = str(audio['TPE1'][0])
        if 'TALB' in audio:
            metadata['album'] = str(audio['TALB'][0])
        if 'TRCK' in audio:
            metadata['track'] = str(audio['TRCK'][0])
        if 'TYER' in audio:
            metadata['year'] = str(audio['TYER'][0])
        if 'TCON' in audio:
            metadata['genre'] = str(audio['TCON'][0])
        return jsonify(metadata, ensure_ascii=False)
    except ID3NoHeaderError:
        return jsonify({"error": "ID3 태그가 없습니다."}, 404)
    except ID3TagError as e:  # ID3TagError 처리 추가
        return jsonify({"error": f"ID3 태그 처리 오류: {str(e)}"}, 500)
    except FileNotFoundError: # 파일이 없을 경우 처리
        return jsonify({"error": "파일을 찾을 수 없습니다."}, 404)
    except Exception as e:
        return jsonify({"error": str(e)}, 500)

@app.route('/metadata/<path:filepath>', methods=['POST'])
def update_metadata(filepath):
    data = request.get_json()
    try:
        audio = MP3(filepath)
        # 태그를 설정할 때, 문자열 값이 되도록 수정
        if 'title' in data:
            audio.tags['TIT2'] = data['title']
        if 'artist' in data:
            audio.tags['TPE1'] = data['artist']
        if 'album' in data:
            audio.tags['TALB'] = data['album']
        if 'track' in data:
            audio.tags['TRCK'] = data['track']
        if 'year' in data:
            audio.tags['TYER'] = data['year']
        if 'genre' in data:
            audio.tags['TCON'] = data['genre']
        audio.save()
        return jsonify({"message": "메타데이터가 성공적으로 업데이트되었습니다."}, ensure_ascii=False) #성공메시지도 utf-8
    except ID3TagError as e: # ID3TagError 처리 추가
        return jsonify({"error": f"ID3 태그 처리 오류: {str(e)}"}, 500)
    except FileNotFoundError: # 파일이 없을 경우 처리
        return jsonify({"error": "파일을 찾을 수 없습니다."}, 404)
    except Exception as e:
        return jsonify({"error": str(e)}, 500)

if __name__ == '__main__':
    app.run(debug=True, port=5000, host='localhost')

