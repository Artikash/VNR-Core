# coding: utf8
# getunidic.py
# 2/24/2014 jichi

if __name__ == '__main__':
  import initrc
  initrc.chcwd()
  initrc.initenv()

import os
from sakurakit.skdebug import dprint, dwarn
from sakurakit.skprofiler import SkProfiler

import initdefs
TARGET_DIR = initdefs.CACHE_UNIDIC_RELPATH
TMP_DIR = initdefs.TMP_RELPATH

# http://sourceforge.jp/projects/unidic/
# http://jaist.dl.sourceforge.jp/unidic/58338/unidic-mecab-2.1.2_bin.zip
# ftp://ftp.jaist.ac.jp/pub/sourceforge.jp/unidic/58338/unidic-mecab-2.1.2_bin.zip
#UNIDIC_URL = 'http://jaist.dl.sourceforge.jp/unidic/58338/unidic-mecab-2.1.2_bin.zip'
#UNIDIC_URL = 'http://ftp.jaist.ac.jp/pub/sourceforge.jp/unidic/58338/unidic-mecab-2.1.2_bin.zip'
UNIDIC_URL = 'http://osdn.dl.sourceforge.jp/unidic/58338/unidic-mecab-2.1.2_bin.zip'
UNIDIC_FILESIZE = 46307109
UNIDIC_FILENAME = 'unidic'
UNIDIC_SUFFIX = '.zip'

def init():
  for it in TMP_DIR:
    if not os.path.exists(it):
      os.makedirs(it)

def get(): # return bool
  url = UNIDIC_URL
  path = TMP_DIR + '/' + UNIDIC_FILENAME + UNIDIC_SUFFIX
  size = UNIDIC_FILESIZE

  dprint("enter: size = %s, url = %s" % (size, url))

  from sakurakit import skfileio
  if os.path.exists(path) and skfileio.filesize(path) == size:
    dprint("leave: already downloaded")
    return True

  from sakurakit import sknetio
  ok = False
  with SkProfiler():
    if sknetio.getfile(url, path, flush=False): # flush=false to use more memory to reduce disk access
      ok = skfileio.filesize(path) == size
  if not ok and os.path.exists(path):
    os.remove(path)
  dprint("leave: ok = %s" % ok)
  return ok

def extract():
  dprint("enter")

  srcpath = TMP_DIR + '/' + UNIDIC_FILENAME + UNIDIC_SUFFIX
  tmppath = TMP_DIR + '/' + UNIDIC_FILENAME
  targetpath = TARGET_DIR

  import shutil
  from sakurakit import skfileio
  ok = skfileio.extractzip(srcpath, tmppath)
  if ok:
    if os.path.exists(targetpath):
      shutil.rmtree(targetpath)
    child = skfileio.getfirstchilddir(tmppath)
    os.renames(child, targetpath)
  if os.path.exists(tmppath):
    shutil.rmtree(tmppath)
  os.remove(srcpath)

  dprint("leave: ok = %s" % ok)
  return ok

def main():
  """
  @param  argv  [unicode]
  @return  int
  """
  dprint("enter")
  ok = False
  try:
    init()
    ok = get() and extract()
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
  if not initrc.lock('unidic.lock'):
    dwarn("multiple instances")
    sys.exit(1)
  ret = main()
  sys.exit(ret)

# EOF
