# coding: utf8
# edictmd.py
# 4/6/2015 jichi

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

from sakurakit.skdebug import dwarn

def assemble(id, word, define):
  """
  @param  id  long  sql role id
  @param  word  unicode
  @param  define  unicode
  @return  unicode
  """
  ret = "%s,%s,%s" % (id, word, define)
  return ret

if __name__ == '__main__':
  dbpath = '../dictp/edict.db'
  import sqlite3
  from dictdb import dictdb
  with sqlite3.connect(dbpath) as conn:
    cur = conn.cursor()
    q = dictdb.iterentries(cur)
    for i,it in enumerate(q):
      define = assemble(i+1, *it)
      print define
      break

# EOF
