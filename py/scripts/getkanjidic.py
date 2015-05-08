# coding: utf8
# getkanjidic.py
# Get the latest EDICT dictionary.
# 2/9/2014 jichi

if __name__ == '__main__':
  import initrc
  initrc.chcwd()
  initrc.initenv()

  import os
  title = os.path.basename(__file__)
  initrc.settitle(title)

import os
from sakurakit.skdebug import dprint, dwarn
from sakurakit.skprof import SkProfiler

import initdefs
TARGET_DIR = initdefs.CACHE_KANJIDIC_RELPATH
TMP_DIR = initdefs.TMP_RELPATH

DIC_URL = 'http://ftp.monash.edu.au/pub/nihongo/kanjidic.gz'
HP_URL = 'http://www.csse.monash.edu.au/~jwb/kanjidic.html'
MIN_DIC_SIZE = 1 * 1024 * 1024 # 1MB, actually 1.1 MB
DIC_FILENAME = 'kanjidic'

import initdefs
TMP_DIR = initdefs.TMP_RELPATH

# Tasks

def init(): # raise
  for it in TARGET_DIR, TMP_DIR:
    if not os.path.exists(it):
      os.makedirs(it)

def get(): # -> bool
  url = DIC_URL
  minsize = MIN_DIC_SIZE
  path = TMP_DIR + '/' + DIC_FILENAME
  targetpath = TARGET_DIR + '/' + DIC_FILENAME

  dprint("enter: url = %s, minsize = %s" % (url, minsize))

  #from sakurakit import skfileio
  #if os.path.exists(path) and skfileio.filesize(path) == size:
  #  dprint("leave: already downloaded")
  #  return True

  ok = False
  from sakurakit import skfileio, sknetio
  with SkProfiler("fetch"):
    # gzip=True to automatically extract gzip
    # flush=false to use more memory to reduce disk access
    if sknetio.getfile(url, path, flush=False, gzip=True):
      ok = skfileio.filesize(path) > minsize
  if ok:
    os.renames(path, targetpath)
  elif os.path.exists(path):
    skfileio.removefile(path)
  dprint("leave: ok = %s" % ok)
  return ok

# Main process

def msg():
  import messages
  messages.info(
    name="KANJIDIC",
    location="Caches/Dictionaries/KanjiDic/kanjidic",
    size=MIN_DIC_SIZE,
    urls=[HP_URL, DIC_URL],
  )

def main():
  """
  @return  int
  """
  dprint("enter")
  ok = False
  try:
    init()
    msg()
    ok = get()
    if ok:
      from sakurakit import skos
      skos.open_location(os.path.abspath(TARGET_DIR))
  except Exception, e:
    dwarn(e)
  ret = 0 if ok else 1
  dprint("leave: ret = %s" % ret)
  return ret

if __name__ == '__main__':
  import sys
  if not initrc.lock('kanjidic.lock'):
    dwarn("multiple instances")
    sys.exit(1)
  ret = main()
  sys.exit(ret)


# EOF
