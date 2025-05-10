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
#        'release_date': audio.get('TDRC', [None])[0],
        'lyrics': audio.get('USLT::eng', [None]),
        'lyrics':audio.get('USLT::eng', None).text if audio.get('USLT::eng') else audio.get('USLT::kor'),
        'genre1': audio.get('TCON', [None, None])[0],
#        'genre2': audio.get('TCON', [None, None])[1],
        'file_path': file
        }

def get_meta2( file ):
    audio = mutagen.id3.ID3(file)
    return { 
        'title': audio.getall('TIT2'),
        'artist': audio.getall('TPE1'),
        'composer': audio.getall('TCOM'),
        'sample_rate': audio.info.sample_rate,
        'file_size': os.path.getsize(file),
        'duration': audio.info.length,
        'album': audio.getall('TALB'),
        'release_date': audio.getall('TDRC'),
        'lyrics': audio.getall('USLT'),
        'genre1': audio.getall('TCON'),
#        'genre2': audio.get('TCON', [None, None])[1],
        'file_path': file
    }

import chardet

def dprint( fn ):
    rt = get_meta( fn )
    for k, d in rt.items():
        u = d
        try:
            if k not in ('title','artist','album'):
                continue
            cs = chardet.detect(d)['encoding'].lower()
            print('>>>>>>>>>>>>>>>>>>>>>>>')
            if cs == 'utf-8':
                print('^^^^^^^^^^^^^^^^^^^^^^^^^^^^')
            elif cs == 'latin1':
                u = d.encode('raw_unicode_escape').decode('latin1')
                print('!!!!!!!!!!!!!!')
            elif cs == 'euc-kr':
                u = d.encode('raw_unicode_escape').decode('euc-kr')
                print('!!!!!!!!!!!!!!')
            elif cs == 'cp949':
                u = d.encode('raw_unicode_escape').decode('cp949')
                print('~~~~~~~~~~~~~~~~~~~~~~~~')
            elif cs == 'iso-8859-1':
                u = d.encode('raw_unicode_escape').decode('iso-8859-1')
                print('//////////////////////////////')
            else:
                u = d.encode('utf-8')
                print(f"++++++++++++++++++++++++++++++++++++++{cs}")
        except Exception as e:
            #print(f"에러 발생! 유형: {type(e).__name__}, 메시지: {str(e)}")
            #print(f'--------------------------{d}')
            pass
        finally:
            print( f"{k}  : {u}" )

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
