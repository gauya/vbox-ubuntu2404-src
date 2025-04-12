import sys
import termios
import tty
import os
import time
import select

# 키 입력 읽기
def keyev(timeout=0):
	rlist, _, _ = select.select([sys.stdin], [], [], timeout)
	if rlist:
		return True
	else:
		return False

def read_key(timeout=0.1):
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        if keyev(timeout):
            ch1 = sys.stdin.read(1)
            if ch1 == '\x1b':
               if keyev(0.1):
                  ch2 = sys.stdin.read(1)
                  if keyev(0.1):
                     ch3 = sys.stdin.read(1)
                     if keyev(0.1):
                        ch4 = sys.stdin.read(1)
                        return ch1 + ch2 + ch3 + ch4
                     else:
                        return ch1 + ch2 + ch3
                  else:
                     return ch1 + ch2
               else:
                  return ch1
            else:
               return ch1
#                rlist_esc, _, _ = select.select([fd], [], [], 0)
#                if rlist_esc:
#                   ch2 = sys.stdin.read(1)
#                   ch3 = sys.stdin.read(1)
#                   return ch1 + ch2 + ch3
#                else:
#                   return ch1
            return ch1
        else:
            return None
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)

while True:
    key = read_key(0.2)
    if not key:
        continue

    if key == 'q' or key == 'Q':
        print("게임 종료!")
        break
    elif key == '\x1b[A':  # ↑
        print("UP")
    elif key == '\x1b[B':  # ↓
        print("DOWN")
    elif key == '\x1b[C':  # →
        print("RIGHT")
    elif key == '\x1b[D':  # ←
        print("LEFT")
    elif key >= ' ' or key == '\t':
        print(key,end='',flush=True)
    elif key == '\n' or key == '\r':
        print(key,flush=True)
    else:
        print(f"{repr(key)}")
	 


