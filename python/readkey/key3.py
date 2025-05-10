import sys
import select,tty,termios

# 데이터가 있는지 확인 (타임아웃 0으로 즉시 반환)

def rkey( timeout=0):
	fd = sys.stdin.fileno()
	old_settings = termios.tcgetattr(sys.stdin)
	try:
		tty.setraw(fd)
		rlist, _, _ = select.select([fd], [], [], timeout)
		if rlist:
			ch1 = sys.stdin.read(1)
			if ch1 == '\x1b':
				rlist, _, _ = select.select([fd], [], [], timeout)
				if rlist:
					ch2 = sys.stdin.read(1)
					ch3 = sys.stdin.read(1)
					return ch1+ch2+ch3
				else:
					return ch1
			return ch1
		else:
			return None
	except:
		return '!'
	finally:
		termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)

"""	
	if rlist:
	    print("읽을 데이터가 있습니다!")
	    data = sys.stdin.read()
	    print( '--->['+data+' :'+len(data)+rlist )
	else:
		 #pass
	    print("아직 입력된 데이터가 없습니다.")
"""	

import time

while True:
	v = rkey(0)

	if v:
		print(len(v))
		for i in range(len(v)):
			print(v[i])
	else:
		pass
		#print('*')

	time.sleep(.1)
