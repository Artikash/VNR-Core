# coding: utf8
# edictp.py
# 4/6/2015 jichi
#
# Download:
# http://www.edrdg.org/jmdict/edict.html
# http://ftp.monash.edu.au/pub/nihongo/00INDEX.html
#
# UTF-8 encoding:
# http://ftp.monash.edu.au/pub/nihongo/edict2u.gz

if __name__ == '__main__':
  import sys
  sys.path.append("..")

# For better performance
#try: from pysqlite2 import dbapi2 as sqlite3
#except ImportError: import sqlite3
import sqlite3
from sakurakit.skdebug import dwarn

#TABLE_NAME = 'entry'

def parsefile(path, encoding='utf8'):
  """
  @param  path  str
  @yield  (unicode word, unicode definition)
  @raise
  """
  try:
    with open(path, 'r') as f:
      for line in f:
        if encoding:
          line = line.decode(encoding)
        ch = ord(line[0])
        # skip the first line and letters
        if ch == 0x3000 or ch >= ord(u'Ａ') and ch <= ord(u'Ｚ') or ch >= ord(u'ａ') and ch <= ord(u'ｚ'):
          continue
        #left.strip(), sep, right = line.partition('/')
        i = line.index('/')
        yield line[:i-1], line[i:-1] # trim trailing spaces
  except Exception, e:
    dwarn(e)
    raise

if __name__ == '__main__':
  path = 'edict2u'
  try:
    for i,(k,v) in enumerate(iterparse(path)):
      print '|%s|' % i
      print '|%s|' % k
      print '|%s|' % v
      break
  except:
    pass

  dbpath = 'edict.db'
  import os
  if os.path.exists(dbpath):
    os.remove(dbpath)

  from sakurakit.skprof import SkProfiler

  from dictdb import dictdb

  with SkProfiler("create db"):
    print dictdb.createdb(dbpath)

  with SkProfiler("insert db"):
    try:
      with sqlite3.connect(dbpath) as conn:
        q = parsefile(path)
        dictdb.insertentries(conn.cursor(), q, ignore_errors=True)
        conn.commit()
    except Exception, e:
      dwarn(e)

# EOF
