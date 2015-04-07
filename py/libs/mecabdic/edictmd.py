# coding: utf8
# edictmd.py
# 4/6/2015 jichi

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

from sakurakit.skdebug import dwarn

def parse(id, word, define):
  """
  @param  id  long
  @param  word  unicode
  @param  define  unicode
  @return  unicode
  """
  return

if __name__ == '__main__':
  dbpath = '../dictp/edict.db'
  import sqlite3
  from dictdb import dictdb
  with sqlite3.connect(dbpath) as conn:
    cur = conn.cursor()
    q = dictdb.iterentries(cur)
    for i,it in enumerate(q):
      print i+1, it[0], it[1]
      break

# EOF
