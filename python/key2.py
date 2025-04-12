import sys
import termios
import tty
import os
import time
import select

# 키 입력 읽기
def read_key(timeout=0.1):
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        rlist, _, _ = select.select([fd], [], [], timeout)
        if rlist:
            ch1 = sys.stdin.read(1)
            if ch1 == '\x1b':
                ch2 = sys.stdin.read(1)
                ch3 = sys.stdin.read(1)
                return ch1 + ch2 + ch3
            return ch1
        else:
            return None
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)

# 맵 설정
game_map = [
    "###############",
    "#             #",
    "#   #####     #",
    "#   #   #     #",
    "#   ### #     #",
    "#             #",
    "###############",
]

# 초기 위치
player_x = 1
player_y = 1

def draw():
    os.system('clear')  # 터미널 클리어
    for y, line in enumerate(game_map):
        row = ""
        for x, ch in enumerate(line):
            if x == player_x and y == player_y:
                row += "O"
            else:
                row += ch
        print(row)

def can_move(x, y):
    if 0 <= y < len(game_map) and 0 <= x < len(game_map[0]):
        return game_map[y][x] == " "
    return False

# 게임 루프
print("게임 시작! 방향키로 움직이고 Q 키로 종료하세요.")
while True:
    draw()
    key = read_key(0.2)
    if not key:
        continue

    if key == 'q' or key == 'Q':
        print("게임 종료!")
        break
    elif key == '\x1b[A':  # ↑
        if can_move(player_x, player_y - 1):
            player_y -= 1
    elif key == '\x1b[B':  # ↓
        if can_move(player_x, player_y + 1):
            player_y += 1
    elif key == '\x1b[C':  # →
        if can_move(player_x + 1, player_y):
            player_x += 1
    elif key == '\x1b[D':  # ←
        if can_move(player_x - 1, player_y):
            player_x -= 1

