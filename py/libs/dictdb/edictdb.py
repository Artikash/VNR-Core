# coding: utf8
# edictdb.py
# 1/16/2014
# See: http://zetcode.com/db/sqlitepythontutorial/

if __name__ == '__main__':
  import sys
  sys.path.append("..")

import sqlite3
from sakurakit.skdebug import dwarn
import dictdb, dbutil

def createtables(cur): # cursor -> bool
  """
  @param  cursor
  @return  bool
  @raise
  """
# Forign key is not enabled, which will make sqlite slightly slower
# http://stackoverflow.com/questions/13934994/sqlite-foreign-key-examples
#FOREIGN KEY(entry_id) REFERENCES entry(id),
  cur.execute('''\
CREATE TABLE surface(
id INTEGER PRIMARY KEY AUTOINCREMENT,
text TEXT NOT NULL UNIQUE,
entry_id INTEGER)
''').execute('''\
CREATE UNIQUE INDEX idx_surface ON surface(text ASC)
''')
  return True

def createdb(dbpath): # unicode path -> bool
  """
  @param  dbpath  unicode
  @return  bool
  """
  try:
    with sqlite3.connect(dbpath) as conn:
      cur = conn.cursor()
      dictdb.createtables(cur)
      createtables(cur)
      conn.commit()
      return True
  except Exception, e:
    dwarn(e)
  return False

if __name__ == '__main__':
  import os
  path = 'test.db'
  if os.path.exists(path):
    os.remove(path)
  print createdb(path)

# EOF
