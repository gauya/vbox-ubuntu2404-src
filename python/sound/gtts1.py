from gtts import gTTS
from playsound import playsound

tts = gTTS("안녕하세요. 반갑습니다.", lang='ko')
tts.save("hello.mp3")

playsound("hello.mp3")
