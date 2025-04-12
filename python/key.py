import sys
import tty
import termios
import select

def read_key(timeout=0.1):
    """Escape 시퀀스 포함한 키 입력을 받아 문자열로 반환"""
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    
    try:
        tty.setraw(fd)
        rlist, _, _ = select.select([fd], [], [], timeout)
        if rlist:
            first_char = sys.stdin.read(1)
            if first_char == '\x1b':
                seq = first_char + sys.stdin.read(2)
                # 일부 키는 추가로 더 읽어야 함
                if seq[2] == '~':
                    seq += sys.stdin.read(1)  # 예: F5 ~ F8 처리용
                return seq
            else:
                return first_char
        else:
            return None
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)

def key_to_name(seq):
    """입력된 키 시퀀스를 사람이 이해할 수 있는 이름으로 변환"""
    mapping = {
        '\x1b[A': 'KEY_UP',
        '\x1b[B': 'KEY_DOWN',
        '\x1b[C': 'KEY_RIGHT',
        '\x1b[D': 'KEY_LEFT',
        '\x1bOP': 'KEY_F1',
        '\x1bOQ': 'KEY_F2',
        '\x1bOR': 'KEY_F3',
        '\x1bOS': 'KEY_F4',
        '\x1b[15~': 'KEY_F5',
        '\x1b[17~': 'KEY_F6',
        '\x1b[18~': 'KEY_F7',
        '\x1b[19~': 'KEY_F8',
        '\x1b[20~': 'KEY_F9',
        '\x1b[21~': 'KEY_F10',
        '\x1b[23~': 'KEY_F11',
        '\x1b[24~': 'KEY_F12',
        '\x03': 'CTRL_C',
        '\x04': 'CTRL_D',
        '\x1b': 'ESC',
    }
    return mapping.get(seq, seq)  # 매핑 안 된 건 그대로 반환

# 🔁 루프 예제
print("키를 눌러보세요 (Ctrl+C로 종료):")
while True:
    key = read_key()
    if key:
        name = key_to_name(key)
        print(f"입력: {repr(key)} → {name}")
        if name == 'CTRL_C':
            print("종료합니다.")
            break

