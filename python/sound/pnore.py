#
import os,sys,random
import playsound

def get_mp3file_list(path, rand=None):
	lst = os.listdir(path);
	#print(path,lst)
	olst=[]
	for fn in lst:
		if not os.path.isfile(path+"/"+fn):
			continue
		olst.append(fn)

	if rand:
		random.shuffle(olst)
	
	return olst

def play(fn):
	playsound.playsound(fn)


if __name__ == "__main__":
	lst=get_mp3file_list(sys.argv[1],1)
	#print(lst)

	for fn in lst:
		play( sys.argv[1] +"/"+fn )


