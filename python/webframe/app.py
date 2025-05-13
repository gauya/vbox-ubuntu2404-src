from flask import Flask, request, jsonify
from flask_cors import CORS
from mutagen.mp3 import MP3
from mutagen.id3 import ID3NoHeaderError

app = Flask(__name__)
CORS(app)

@app.route('/metadata/<path:filepath>', methods=['GET'])
def get_metadata(filepath):
    try:
        audio = MP3(filepath)
        metadata = {}
        if 'TIT2' in audio.tags:
            metadata['title'] = str(audio.tags['TIT2'])
        if 'TPE1' in audio.tags:
            metadata['artist'] = str(audio.tags['TPE1'])
        if 'TALB' in audio.tags:
            metadata['album'] = str(audio.tags['TALB'])
        if 'TRCK' in audio.tags:
            metadata['track'] = str(audio.tags['TRCK'])
        if 'TYER' in audio.tags:
            metadata['year'] = str(audio.tags['TYER'])
        if 'TCON' in audio.tags:
            metadata['genre'] = str(audio.tags['TCON'])
        return jsonify(metadata)
    except ID3NoHeaderError:
        return jsonify({"error": "ID3 태그가 없습니다."}), 404
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/metadata/<path:filepath>', methods=['POST'])
def update_metadata(filepath):
    data = request.get_json()
    try:
        audio = MP3(filepath)
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
        return jsonify({"message": "메타데이터가 성공적으로 업데이트되었습니다."})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True, port=5000, host='localhost')
