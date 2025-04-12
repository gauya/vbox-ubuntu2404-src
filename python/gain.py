from pydub import AudioSegment

sound = AudioSegment.from_file("input.mp3")
louder = sound + 6       # +6dB
# louder = sound.apply_gain(6) 
quieter = sound - 10     # -10dB
normalized = sound.apply_gain(-sound.max_dBFS)  # 최대볼륨 정규화
normalized.export("normalized.mp3", format="mp3")

# 결과 저장
louder.export("louder.mp3", format="mp3")
quieter.export("quieter.mp3", format="mp3")

fade_in = sound.fade_in(2000)    # 2초간 점점 커짐
fade_out = sound.fade_out(3000)  # 3초간 점점 작아짐


