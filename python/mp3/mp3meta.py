#! /usr/bin/python

import mutagen
import os,sys

def get_meta( file ):
    audio = mutagen.File(file)
    return { 
        'title': audio.get('TIT2', ['Unknown'])[0],
        'artist': audio.get('TPE1', ['Unknown'])[0],
        'composer': audio.get('TCOM', [None])[0],
        'sample_rate': audio.info.sample_rate,
        'file_size': os.path.getsize(file),
        'duration': audio.info.length,
        'album': audio.get('TALB', [None])[0],
        'release_date': audio.get('TDRC', [None])[0],
        'genre1': audio.get('TCON', [None, None])[0],
#        'genre2': audio.get('TCON', [None, None])[1],
        'file_path': file
    }

def dprint( fn ):
    rt = get_meta( fn )
    for k, d in rt.items():
        print( f"{k}  : {d}" )

if __name__ == "__main__":
#    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

    if len(sys.argv) <= 1:
        path = '.'
    else:
        path = sys.argv[1]
        if os.path.isfile(path):
            dprint(path)
            sys.exit(1)
    lst = os.listdir( path )
    for fn in lst:
        if fn[-4:].lower() == '.mp3':
            fn = path + '/' + fn
            dprint(fn)
