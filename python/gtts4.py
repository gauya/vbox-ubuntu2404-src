from gtts import gTTS
from pydub import AudioSegment
from pydub.playback import play

tts = gTTS("이건 안정적으로 작동해야 합니다", lang='ko')
tts.save("temp.mp3")

sound = AudioSegment.from_mp3("temp.mp3")
play(sound)  # 이건 소리 남, 세그폴트는 ffmpeg 버그 가능성

