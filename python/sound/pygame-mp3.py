import os
os.environ["SDL_AUDIODRIVER"] = "alsa"  # 또는 "dsp"

import pygame
from gtts import gTTS
from io import BytesIO

# 음성 생성
tts = gTTS("파이게임에서 재생 테스트입니다", lang='ko')
fp = BytesIO()
tts.write_to_fp(fp)
fp.seek(0)

# mp3를 wav로 변환 (pygame은 wav 권장)
from pydub import AudioSegment
sound = AudioSegment.from_file(fp, format="mp3")
wav_fp = BytesIO()
sound.export(wav_fp, format="wav")
wav_fp.seek(0)

# pygame으로 재생
#pygame.mixer.init()
pygame.mixer.init(frequency=44100, size=-16, channels=2, buffer=4096)
pygame.mixer.music.load(wav_fp)
pygame.mixer.music.play()

# 재생 완료까지 대기
while pygame.mixer.music.get_busy():
    pygame.time.wait(100)
