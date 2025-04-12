from gtts import gTTS
from io import BytesIO
from pydub import AudioSegment
#from pydub.playback import _play_with_simpleaudio as play
from pydub.playback import play

tts = gTTS("세그폴트 방지 테스트입니다", lang='ko')
#tts = gTTS("good morning")
fp = BytesIO()
tts.write_to_fp(fp)
fp.seek(0)

sound = AudioSegment.from_file(fp, format="mp3")

# 너무 짧으면 최소 길이 확보
if len(sound) < 500:
    sound += AudioSegment.silent(duration=500 - len(sound))

#print(len(sound))
play(sound)
