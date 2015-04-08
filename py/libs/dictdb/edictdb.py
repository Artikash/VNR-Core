# coding: utf8
# edictdb.py
# 1/16/2014
# See: http://zetcode.com/db/sqlitepythontutorial/

if __name__ == '__main__':
  import sys
  sys.path.append("..")

import os, sqlite3
from sakurakit.skdebug import dwarn
from unitraits.uniconv import hira2kata, kata2hira
from dictp import edictp
import dictdb, dbutil

# Queries

SELECT_ALL = 'entry.*'

def queryentries(cur, surface, limit=0, select=SELECT_ALL):
  """
  @param  cursor
  @param* surface  unicode
  @param* limit  int
  @return  (unicode word, unicode content)
  @raise
  """
  params = [surface]
  sql = "SELECT %s FROM entry, surface f where entry.id = f.entry_id and f.text = ?" % select
  if limit:
    sql += ' limit ?'
    params.append(limit)
  cur.execute(sql, params)
  return cur.fetchall()

# Construction

def makedb(dbpath, dictpath): # unicode path -> bool
  """
  @param  dbpath  unicode  target db path
  @param  dictpath  unicode  source edict2u path
  @return  bool
  """
  try:
    if os.path.exists(dbpath):
      os.remove(dbpath)
    with sqlite3.connect(dbpath) as conn:
      dictdb.createtables(conn.cursor())
      conn.commit()

      q = edictp.parsefile(dictpath)
      dictdb.insertentries(conn.cursor(), q, ignore_errors=True)
      conn.commit()
      return True
  except Exception, e:
    dwarn(e)
  return False

def makesurface(dbpath): # unicode path -> bool
  """
  @param  dbpath  unicode  target db path
  @param  dictpath  unicode  source edict2u path
  @return  bool
  """
  try:
    with sqlite3.connect(dbpath) as conn:
      q = create_surface_tables(conn.cursor())
      conn.commit()

      q = iter_entry_surfaces(conn.cursor())
      insertsurfaces(conn.cursor(), q)
      conn.commit()
      return True
  except Exception, e:
    dwarn(e)
  return False

def create_surface_tables(cur): # cursor -> bool
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
text TEXT NOT NULL,
entry_id INTEGER)
''').execute('''\
CREATE INDEX idx_surface ON surface(text ASC)
''')
  return True

def iter_entry_surfaces(cur):
  """
  @param  cursor
  @yield  (unicode surface, int entry_id)
  @raise
  """
  for i,word in enumerate(dictdb.iterwords(cur)):
    id = i + 1
    surfaces = set()
    for surf,yomi in edictp.parseword(word):
      if surf not in surfaces:
        surfaces.add(surf)
        hira = kata2hira(yomi) if yomi else ''
        kata = hira2kata(yomi) if yomi else ''
        yield surf, id
        for kana in (yomi, hira, kata):
          if kana and kana not in surfaces:
            surfaces.add(kana)
            yield kana, id

def insertsurface(cur, entry): # cursor, entry; raise
  """
  @param  cursor
  @param  entry  (unicode text, int entry_id)
  @param* ignore_errors  whether ignore exceptions
  @raise
  """
  sql = "INSERT INTO surface(text,entry_id) VALUES(?,?)"
  cur.execute(sql, entry)

def insertsurfaces(cur, entries): # cursor, [entry]; raise
  """
  @param  cursor
  @param  entries  [unicode word, unicode content]
  @param* ignore_errors  whether ignore exceptions
  @raise
  """
  for it in entries:
    insertsurface(cur, it)

if __name__ == '__main__':
  from sakurakit.skprof import SkProfiler
  dictpath = '../dictp/edict2u'
  dbpath = 'edict.db'

  def test_create():
    with SkProfiler("make db"):
      print makedb(dbpath, dictpath)
    with SkProfiler("make surface"):
      print makesurface(dbpath)

  def test_query():
    #t = u'あしきり'
    #t = u'ごめんなさい'
    t = u'ご'
    with sqlite3.connect(dbpath) as conn:
      cur = conn.cursor()
      for it in queryentries(cur, surface=t):
        print it[0], it[1], it[2]

  #test_create()
  test_query()

# EOF
