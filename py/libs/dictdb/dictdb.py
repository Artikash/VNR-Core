# coding: utf8
# dictdb.py
# 1/16/2014
# See: http://zetcode.com/db/sqlitepythontutorial/

if __name__ == '__main__':
  import sys
  sys.path.append("..")
import sqlite3
from sakurakit.skdebug import dwarn

#TABLE_NAME = 'entry'

def queryentry(cur, word='', wordlike='', limit=0): # cursor, string -> entry; raise
  params = []
  sql = "SELECT word,content FROM entry"
  if wordlike:
    sql += ' where word like ?'
    params.append(wordlike)
  if limit:
    sql += ' limit ?'
    params.append(limit)
  cur.execute(sql, params)
  return cur.fetchone()

def queryentries(cur, word='', wordlike='', limit=0): # cursor, string -> [entry]; raise
  params = []
  sql = "SELECT word,content FROM entry"
  if word:
    sql += ' where word = ?'
    params.append(word)
  elif wordlike:
    sql += ' where word like ?'
    params.append(wordlike)
  if limit:
    sql += ' limit ?'
    params.append(limit)
  cur.execute(sql, params)
  return cur.fetchall()

def insertentry(cur, entry): # cursor, entry; raise
  cur.execute("INSERT INTO entry(word,content) VALUES(?,?)", entry)

def insertentries(cur, entries): # cursor, [entry]; raise
  for it in entries:
    insertentry(it)

def createtables(cur): # cursor -> bool
  cur.execute('''\
CREATE TABLE entry(
id INTEGER PRIMARY KEY AUTOINCREMENT,
word TEXT NOT NULL UNIQUE,
content TEXT NOT NULL)
''').execute('''\
CREATE UNIQUE INDEX idx_word ON entry(word ASC)
''')
  return True

def createdb(dbpath): # unicode path -> bool
  try:
    with sqlite3.connect(dbpath) as conn:
      cur = conn.cursor()
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

  with sqlite3.connect(path) as conn:
    cur = conn.cursor()
    insertentry(cur, ('a', 'b'))
    insertentry(cur, ('ab', 'b'))
    conn.commit()

    print queryentry(cur, wordlike='a')

    for it in queryentries(cur, wordlike='%a%'):
      print it

# EOF
