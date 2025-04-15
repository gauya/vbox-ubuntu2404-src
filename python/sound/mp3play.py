#! /usr/bin/python

import sys
import select
import tty, termios

def get_char(timeout=0):
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)

    try:
        #tty.setcbreak(fd) # echo off
        tty.setraw(fd)  # 입력을 즉시 처리
        #termios.tcsetattr(fd, termios.TCSANOW, termios.tcgetattr(fd))

        rlist, _, _ = select.select([fd], [], [], timeout)
        if rlist:
            ch1 = sys.stdin.read(1)
            if ch1 == '\x1b':
               ch2 = sys.stdin.read(1)
               ch3 = sys.stdin.read(1)
               return ch1 + ch2 + ch3
            return ch1
        else:
            return None  # 시간 초과
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)

import vlc
import time

class VLCPlayer:
    def __init__(self):
        self.instance = vlc.Instance()
        self.player = self.instance.media_player_new()
        self.is_playing = False
    
    def play(self, file_path):
        media = self.instance.media_new(file_path)
        self.player.set_media(media)
        self.player.play()
        self.is_playing = True
    
    def pause(self):
        if self.is_playing:
            self.player.pause()
    
    def resume(self):
        if self.is_playing:
            self.player.play()
    
    def stop(self):
        self.player.stop()
        self.is_playing = False
    
    def force_stop(self):
        self.stop()  # VLC에서는 일반 stop으로 충분


from pydub import AudioSegment
from pydub.playback import play
import threading
import time

class PydubMP3Player:
   def __init__(self):
      self.audio = None
      self.play_thread = None
      self.stop_flag = False
      self.pause_flag = False
      self.playing_pos = 0

   def __del__(self):
      self.stop()

   def play(self, file_path):
      if self.play_thread and self.play_thread.is_alive():
         print("already alive thread")
         self.stop()
      
      self.audio = AudioSegment.from_mp3(file_path)
      self.stop_flag = False
      self.pause_flag = False
      self.playing_pos = 0
      print("start thread")
      
   def _play(self):
      while self.playing_pos < len(self.audio) and not self.stop_flag:
         if not self.pause_flag:
             chunk = self.audio[self.playing_pos:self.playing_pos+100]
             play(chunk)
             self.playing_pos += 100
         else:
             time.sleep(0.1)

         self.play_thread = threading.Thread(target=_play, daemon=True)
         self.play_thread.start()

   def pause(self):
      self.pause_flag = True

   def resume(self):
      self.pause_flag = False

   def stop(self):
      self.stop_flag = True
      if self.play_thread and self.play_thread.is_alive():
         self.play_thread.join(timeout=0.5)

   def force_stop(self):
      self.stop()  # pydub에서는 일반 stop으로 충분

import os
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = '1'
import pygame
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = '0'

import time
import threading

class MP3Player:
    def __init__(self):
        pygame.mixer.init()
        self.is_playing = False
        self.is_paused = False
        self.thread = None

    def play(self, file_path):
        if self.is_playing and not self.is_paused:
            return

        def _play():
            pygame.mixer.music.load(file_path)
            pygame.mixer.music.play()
            self.is_playing = True
            self.is_paused = False

            while pygame.mixer.music.get_busy() and self.is_playing:
                if self.is_paused:
                    pygame.mixer.music.pause()
                    while self.is_paused and self.is_playing:
                        time.sleep(0.1)
                    pygame.mixer.music.unpause()
                time.sleep(0.1)

        self.thread = threading.Thread(target=_play, daemon=True)
        self.thread.start()
        time.sleep(.1)

    def pause(self):
        if self.is_playing and not self.is_paused:
            self.is_paused = True

    def resume(self):
        if self.is_playing and self.is_paused:
            self.is_paused = False

    def stop(self):
        self.is_playing = False
        self.is_paused = False
        pygame.mixer.music.stop()
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=0.5)
        self.thread = None

    def force_stop(self):
        self.stop()  # pygame에서는 일반 stop으로 충분

    def get_volume(self):
        return pygame.mixer.music.get_volume()

    def set_volume(self, vol):
        pygame.mixer.music.set_volume(vol)

    def rewind(self):
        t = pygame.mixer.music.get_pos()
        pygame.mixer.music.rewind()
        #pygame.mixer.music.set_pos(0)
        return t

    def is_alive(self):
        return (pygame.mixer.music.get_busy())

def vlctest():
   v = VLCPlayer()

   v.play('/home/gau/media/Music/sounds/소영-01-숨-숨-192.mp3')

   print("End vlctest")

def duptest():
   player = PydubMP3Player()
   player.play("/home/gau/media/Music/sounds/소영-01-숨-숨-192.mp3")
   
   # 5초 후 일시정지
   time.sleep(5)
   player.pause()
   
   # 3초 후 재개
   time.sleep(3)
   player.resume()
   
   # 5초 후 정지
   time.sleep(5)
   player.stop()

def pygametest(path,flst):
   player = MP3Player()

   #for fn in flst:
   idx=0
   mfn=len(flst)
   if mfn == 0:
      return
   ing = True
   while ing:
      if idx >= len(flst): idx = 0
      fn = flst[idx]
      idx = idx+1

      if path == str:
         file_path = path+"/"+fn
      else:
         file_path = fn
      print(file_path)

      player.play(file_path)

      while player.is_alive():
         k=get_char(0.2)
         if not k: 
            #time.sleep(0.1)
            continue
         if k == "n": # next
            break
         elif k == "p": # prev
            idx = idx - 2
            if idx < 0: idx = len(flst) - 1
            break
         elif k == "z": # pause / resume
            player.pause()
            if get_char(1000): # any key
               player.resume()
               time.sleep(0.1)
               continue
         elif k == 0x3 or k == 'q': # end
            ing = False
            break
         elif k[0] == '\x1b':
            if k == '\x1b[A': # UP
               # volum up
               vol = player.get_volume() + 0.1
               if vol > 1.0:
                  vol = 1.0
               player.set_volume(vol)
            elif k == '\x1b[B': # DOWN
               #
               vol = player.get_volume() - 0.1
               if vol < 0.1:
                  vol = 0.1
               player.set_volume(vol)
            elif k == '\x1b[C': # RIGHT
               break # next
            elif k == '\x1b[D': # LEFT
               i = player.rewind()
               #print(f"rewind={i}")
               if i < 3000:
                  idx = idx - 2
                  if idx < 0: idx = len(flst) - 1
                  break
            continue
         elif k < ' ':
            ing = False
            break

      player.stop()

import getopt,sys,random

def get_mp3file_list(path='.', rand=None):
   if type(path) == list:
      lst = path
   else:
      lst = os.listdir(path);
   #print(path,lst)
   olst=[]
   for fn in lst:
      if len(fn) < 4 or fn[-4:] != ".mp3":
         continue
      if path and type(path) == str:
         fn = path+"/"+fn
      if not os.path.isfile(fn):
         continue
      olst.append(fn)

   if rand:
      random.shuffle(olst)
   
   return olst

def help():
   help_msg="""<mp3play.py>

   -h, --help : help
   -r, -R : random play
   -d, --path : path

   ex) mp3play.py -rd ~/media/Music/sounds

Active hotkeys

   n : next mp3
   p : prev mp3
   q : exit
   z : pause  --> anykey to unpause
   UP : volume UP
   DOWN : volume DOWN
   LEFT : rewind"""

   print(help_msg)

def prep():
   try:
      opts, args = getopt.getopt(sys.argv[1:], "hrRd:", ["help", "path="])
   except getopt.GetoptError as err:
      #usage()
      sys.exit(2)
   
   path=None
   random = None
   for o,a in opts:
      if o in ( "-d","--path" ):
         path=a
      elif o in ("-r","-R"):
         random = True
      elif o in ("-h","--help"):
         help()
         sys.exit(0)
      else:
         assert False, "unknown option"
   
   if not path and args:
      flst = get_mp3file_list(args,random)
   else:
      flst = get_mp3file_list(path,random)

   #vlctest()
   #duptest()
   pygametest(path,flst)


if __name__ == '__main__':
   prep()

