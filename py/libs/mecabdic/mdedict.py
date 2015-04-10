# coding: utf8
# mdedict.py
# 11/9/2013 jichi

from sakurakit.skdebug import dwarn
import mdcompile

csv2dic = mdcompile.compile

def db2csv(csvpath, dbpath):
  """
  @param  csvpath  unicode output
  @param  dbpath  unicode dbpath
  @return  bool
  """
  import sqlite3
  from dictdb import dictdb
  from dictp import edictp
  try:
    with sqlite3.connect(dbpath) as conn:
      cur = conn.cursor()
      q = dictdb.iterwords(cur)
      with open(csvpath, 'w') as f:
        for i,word in enumerate(q):
          id = i + 1
          entries = edictp.parseword(word)
          lines = mdcompile.assemble(entries, id=id, type='edict')
          f.writelines(lines)
        return True
  except Exception, e:
    dwarn(e)
    return False

# EOF
