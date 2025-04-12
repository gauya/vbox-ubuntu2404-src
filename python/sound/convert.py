from os import path
from pydub import AudioSegment
import sys,os

afms = [ "wav","mp3" ]

if len(sys.argv) == 3:
	src = sys.argv[1]
	dst = sys.argv[2]

	sfm = src[-3:]
	efm = dst[-3:]

	if sfm in afms and efm in afms:
		if os.path.exists(src): 
			if sfm == 'mp3':
				sound = AudioSegment.from_mp3(src)
			else:
				sound = AudioSegment.from_wav(src)

			sound.export(dst, format=efm)
