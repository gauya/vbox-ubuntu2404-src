from pydub import AudioSegment
from pydub.playback import play

# 오디오 파일 로드
audio = AudioSegment.from_file("input.mp3")

# 재생 속도 조절 (기본 샘플링 레이트 변경)
def change_speed(audio, speed=1.0):
    sample_rate = int(audio.frame_rate * speed)
    return audio._spawn(audio.raw_data, overrides={'frame_rate': sample_rate})

def test1():
    # 빠르게 재생 (1.5배)
    fast_audio = change_speed(audio, speed=1.3)
    # 느리게 재생 (0.75배)
    slow_audio = change_speed(audio, speed=0.75)

    play(fast_audio)  # 빠른 재생
    play(slow_audio)  # 느린 재생

from pydub.effects import speedup

def test2():
    # 피치 유지하며 1.5배 빠르게
    fast_audio = speedup(audio, playback_speed=1.5)
    # 피치 유지하며 0.7배 느리게
    slow_audio = slow_down(audio, playback_speed=0.7)
    
    play(fast_audio)
    play(slow_audio)

def time_stretch(audio, speed):
    return audio.effect(
        "atempo",
        args=[str(speed)],
        use_ffmpeg=True
    )

def test3():
    stretched_audio = time_stretch(audio, 1.5)
    play(stretched_audio)

def change_playback_speed(sound: AudioSegment, speed: float) -> AudioSegment:
    new_frame_rate = int(sound.frame_rate * speed)
    return sound._spawn(sound.raw_data, overrides={"frame_rate": new_frame_rate}).set_frame_rate(sound.frame_rate)

def test4():
    # 오디오 로딩
    
    # 2배 빠르게
    fast_sound = speedup(audio, playback_speed=1.5)
    
    # 0.5배 느리게
    #slow_sound = change_playback_speed(audio, 0.5)
    
    # 재생
    play(fast_sound)  # 빠르게
    # play(slow_sound)  # 느리게
    
    
def test5():
    # 재생 속도 변경 (2배 빠르게)
    faster_sound = audio.speedup(playback_speed=2.0)
    
    # 재생 속도 변경 (0.5배 느리게)
    #slower_sound = audio.speedup(playback_speed=0.5)
    
    # 변경된 파일 저장
    faster_sound.export("faster.mp3", format="mp3")
    #slower_sound.export("slower.mp3", format="mp3")


test5()
