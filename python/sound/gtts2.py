from gtts import gTTS
from pydub import AudioSegment
from pydub.playback import play
from io import BytesIO

# 1. gTTS로 음성 생성
tts = gTTS("안녕하세요. 파일 없이 바로 재생됩니다.", lang='ko')

with BytesIO() as fp:
    tts.write_to_fp(fp)
    fp.seek(0)
    sound = AudioSegment.from_file(fp, format="mp3")
    play(sound)
# 여기서 자동으로 close()됨

