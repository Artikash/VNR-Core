# coding: utf8
# getedict.py
# Get the latest EDICT dictionary.
# 2/9/2014 jichi

if __name__ == '__main__':
  import initrc
  initrc.chcwd()
  initrc.initenv()

import os
from sakurakit.skdebug import dprint, dwarn
from sakurakit.skprof import SkProfiler

import initdefs
TARGET_DIR = initdefs.CACHE_EDICT_RELPATH
TMP_DIR = initdefs.TMP_RELPATH
FILENAME = 'edict.db'

EDICT_URL = 'http://ftp.monash.edu.au/pub/nihongo/edict2u.gz'
MIN_EDICT_SIZE = 15 * 1024 * 1024 # 15MB, actually 15 ~ 16 MB
EDICT_FILENAME = 'edict2u'
DB_FILENAME = 'edict.db'

import initdefs
TMP_DIR = initdefs.TMP_RELPATH

# Tasks

def init(): # raise
  for it in TARGET_DIR, TMP_DIR:
    if not os.path.exists(it):
      os.makedirs(it)

def get(): # -> bool
  url = EDICT_URL
  minsize = MIN_EDICT_SIZE
  path = TMP_DIR + '/' + EDICT_FILENAME

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
  if not ok and os.path.exists(path):
    skfileio.removefile(path)
  dprint("leave: ok = %s" % ok)
  return ok

def makedb(): # -> bool
  dprint("enter")
  tmpdic = TMP_DIR + '/' + EDICT_FILENAME
  tmpdb = TMP_DIR + '/' + DB_FILENAME

  targetdic = TARGET_DIR + '/' + EDICT_FILENAME
  targetdb = TARGET_DIR + '/' + DB_FILENAME

  from dictdb import edictdb
  with SkProfiler("create db"):
    ok = edictdb.makedb(tmpdb, tmpdic)
  if ok:
    with SkProfiler("create index"):
      ok = edictdb.makesurface(tmpdb)

  from sakurakit import skfileio
  if ok:
    skfileio.removefile(targetdb)
    skfileio.removefile(targetdic)
    os.rename(tmpdb, targetdb)
    os.rename(tmpdic, targetdic)
  else:
    for it in tmpdb, tmpdic:
      if os.path.exists(it):
        skfileio.removefile(it)
  dprint("leave: ok = %s" % ok)
  return ok

# Main process

def main():
  """
  @return  int
  """
  dprint("enter")
  ok = False
  try:
    init()
    ok = get() and makedb()
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
  if not initrc.lock('edict.lock'):
    dwarn("multiple instances")
    sys.exit(1)
  ret = main()
  sys.exit(ret)


# EOF
