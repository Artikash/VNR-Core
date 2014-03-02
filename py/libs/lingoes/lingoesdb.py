# coding: utf8
# lingoesdb.py
# 1/16/2013 jichi

__all__ = ['LingoesDb']

if __name__ == '__main__':
  import sys
  sys.path.append("..")
import sqlite3
from sakurakit.skdebug import dprint, dwarn
#from sakurakit.skprofiler import SkProfiler
from dictdb import dictdb

class LingoesDb(object):
  XML_DELIM = '<br class="xml"/>'

  def __init__(self, dbpath=''): # unicode
    super(LingoesDb, self).__init__()
    self.dbpath = dbpath

  def create(self, ldpath, inenc='', outenc=''): # unicode, str, str -> bool
    import lingoesparse
    try:
      with sqlite3.connect(self.dbpath) as conn:
        cur = conn.cursor()
        dictdb.createtables(cur)
        conn.commit()
        l = lingoesparse.parsefile(ldpath, inenc=inenc, outenc=outenc or inenc,
            callback=lambda word, xmls: dictdb.insertentry(cur, (
              word,
              self.XML_DELIM.join(xmls),
            )))
        ok = l != None
        conn.commit()
        return ok
    except Exception, e:
      dwarn(e)
    return False

  def lookup(self, t, limit=0): # unicode, int* -> yield (unicode word, unicode xml) or return None
    #if self.trie:
    #  return self.trie.iteritems(t)
    #if os.path.exists(self.dbpath):
    try:
      with sqlite3.connect(self.dbpath) as conn:
        cur = conn.cursor()
        return dictdb.queryentries(cur, limit=limit, wordlike=t + '%')
      return
    except Exception, e:
      dwarn(e)

if __name__ == '__main__':
  import os
  from sakurakit.skprofiler import SkProfiler
  location = '/Users/jichi/Dropbox/Developer/Dictionaries/LD2/'
  #dic = 'Naver Japanese-Korean Dictionary.ld2'
  #dic = 'OVDP Japanese-Vietnamese Dictionary.ld2'
  #dic = 'Vicon Japanese-English Dictionary.ld2'
  if os.name == 'nt':
    location = 'C:' + location

  dbpath = 'test.db'
  #if os.path.exists(dbpath):
  #  os.remove(dbpath)

  print '-' * 4

  dic = location + 'New Japanese-Chinese Dictionary.ld2'

  HOME = os.path.expanduser('~')
  if os.name == 'nt':
    from sakurakit import skpaths
    APPDATA = skpaths.APPDATA
  else: # mac
    import getpass
    HOME = os.path.expanduser('~')
    USER = getpass.getuser() # http://stackoverflow.com/questions/842059/is-there-a-portable-way-to-get-the-current-username-in-python
    APPDATA = os.path.join(HOME, '.wine/drive_c/users/' + USER + '/Application Data')
  dbpath = APPDATA + '/org.sakuradite.reader/db/lingoes/'

  #dbpath += 'ja-zh.db'
  #dbpath += 'ja-ko.db'
  dbpath += 'ja-vi.db'

  ld = LingoesDb(dbpath)

  #if os.path.exists(dbpath):
  #  os.remove(dbpath)

  if not os.path.exists(dbpath):
    with SkProfiler():
      print ld.create(dic, 'utf16')

  print '-' * 4

  with SkProfiler():
    print "lookup start"
    t = u"かわいい"
    t = u"ちょっと"
    it = ld.lookup(t, limit=6)
    if it:
      for key, xml in it:
        print key
        print xml
        #print xmls[0]
    print "lookup finish"

  # 175 MB
  # 57 MB
  #import time
  #time.sleep(10)

# EOF
