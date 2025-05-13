from flask import Flask, request, jsonify
from flask_cors import CORS
from mutagen.mp3 import MP3
from mutagen.id3 import ID3NoHeaderError,ID3, TIT2, TPE1, TALB, TRCK, TYER, TCON

app = Flask(__name__)
CORS(app)

def decode_id3_text(frame):
    try:
        # 1. 이미 유니코드면 그대로 반환
        if isinstance(frame.text[0], str):
            return frame.text[0]
    except:
        pass

    # 2. raw bytes로 수동 디코딩 시도
    raw = bytes(frame.text[0])
    for enc in ['utf-8', 'cp949', 'euc-kr', 'latin1']:
        try:
            return raw.decode(enc)
        except:
            continue
    return raw.decode('utf-8', errors='replace')  # fallback
    
@app.route('/metadata/<path:filepath>', methods=['GET'])
def get_metadata(filepath):
    try:
        audio = MP3(filepath)
        metadata = {}
        tags = audio.tags

        if 'TIT2' in tags:
            metadata['title'] = decode_id3_text(tags['TIT2'])
        if 'TPE1' in tags:
            metadata['artist'] = decode_id3_text(tags['TPE1'])
        if 'TALB' in tags:
            metadata['album'] = decode_id3_text(tags['TALB'])
        if 'TRCK' in tags:
            metadata['track'] = decode_id3_text(tags['TRCK'])
        if 'TYER' in tags:
            metadata['year'] = decode_id3_text(tags['TYER'])
        if 'TCON' in tags:
            metadata['genre'] = decode_id3_text(tags['TCON'])

        return jsonify(metadata)
    except ID3NoHeaderError:
        return jsonify({"error": "ID3 태그가 없습니다."}), 404
    except Exception as e:
        return jsonify({"error": str(e)}), 500

"""
@app.route('/metadata/<path:filepath>', methods=['GET'])
def get_metadata(filepath):
    try:
        audio = MP3(filepath)
        metadata = {}
        if 'TIT2' in audio.tags:
            metadata['title'] = audio.tags['TIT2'].text[0]
        if 'TPE1' in audio.tags:
            metadata['artist'] = audio.tags['TPE1'].text[0]
        if 'TALB' in audio.tags:
            metadata['album'] = audio.tags['TALB'].text[0]
        if 'TRCK' in audio.tags:
            metadata['track'] = audio.tags['TRCK'].text[0]
        if 'TYER' in audio.tags:
            metadata['year'] = audio.tags['TYER'].text[0]
        if 'TCON' in audio.tags:
            metadata['genre'] = audio.tags['TCON'].text[0]
        return jsonify(metadata)
    except ID3NoHeaderError:
        return jsonify({"error": "ID3 태그가 없습니다."}), 404
    except Exception as e:
        return jsonify({"error": e}), 500
"""

@app.route('/metadata/<path:filepath>', methods=['POST'])
def update_metadata(filepath):
    data = request.get_json()
    try:
        audio = MP3(filepath)
        if audio.tags is None:
            audio.add_tags()
        
        if 'title' in data:
            audio.tags.add(TIT2(encoding=3, text=data['title']))
        if 'artist' in data:
            audio.tags.add(TPE1(encoding=3, text=data['artist']))
        if 'album' in data:
            audio.tags.add(TALB(encoding=3, text=data['album']))
        if 'track' in data:
            audio.tags.add(TRCK(encoding=3, text=data['track']))
        if 'year' in data:
            audio.tags.add(TYER(encoding=3, text=data['year']))
        if 'genre' in data:
            audio.tags.add(TCON(encoding=3, text=data['genre']))

        audio.save()
        return jsonify({"message": "메타데이터가 성공적으로 업데이트되었습니다."})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True, port=5000, host='localhost')
