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

def _remove_word_parenthesis(text):
  """Remove parenthesis
  @param  text  unicode  surface
  @return  unicode
  """
  i = text.find('(')
  if i == -1:
    return text
  else:
    return text[:i]

# Example: 噯;噯気;噫気;噯木(iK) [おくび(噯,噯気);あいき(噯気,噫気,噯木)]
def parseword(word):
  """
  @param  path  str
  @yield  unicode surface, unicode reading
  """
  # Remove duplicate keys
  s = set()
  for k,v in _parseword(word):
    if ord(k[0]) > 255 and k not in s: # skip ascii keys
      s.add(k)
      yield k,v

def _parseword(word):
  """
  @param  path  str
  @yield  unicode surface, unicode reading
  """
  if '[' not in word: # no yomi
    if ';' not in word:
      yield _remove_word_parenthesis(word), ''
    else:
      for it in word.split(';'):
        yield _remove_word_parenthesis(it), ''
  else:
    left, sep, right = word.partition(' [')
    if right[-1] == ']':
      right = right[:-1]
      if ')' in right:
        for group in right.split(';'):
          ll, sep, rr = group.partition('(')
          if rr and rr[-1] == ')':
            rr = rr[:-1]
            if ',' not in rr:
              yield rr, ll
            else:
              for it in rr.split(','):
                yield it, ll
      yomi = None
      if ';' not in right:
        yomi = right
      else:
        yomi = right.partition(';')[0]
        yomi = _remove_word_parenthesis(yomi)
      if ';' not in left:
        yield _remove_word_parenthesis(left), yomi
      else:
        for it in left.split(';'):
          yield _remove_word_parenthesis(it), yomi

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
