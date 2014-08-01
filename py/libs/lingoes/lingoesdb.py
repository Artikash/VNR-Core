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

  def _lookupprefix(self, t, limit=0):
    """
    @param  t  unicode
    @Param  limit  int
    @return  iter(unicode word, unicode xml) or None
    """
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

  _COMPLETE_TRIM_CHARS = u'ぁ', u'ぇ', u'ぃ', u'ぉ', u'ぅ', u'っ', u'ッ'
  def _complete(self, t):
    """Trim half katagana/hiragana.
    @param  t  unicode
    @return  unicode
    """
    if t:
      while len(t) > 1:
        if t[-1] in self._COMPLETE_TRIM_CHARS:
          t = t[:-1]
        else:
          break
      while len(t) > 1:
        if t[0] in self._COMPLETE_TRIM_CHARS:
          t = t[1:]
        else:
          break
    return t

  def lookupprefix(self, t, limit=0, complete=True):
    """Lookup dictionary while eliminate duplicate definitions
    @param  t  unicode
    @param* complete  bool  whether complete the word
    @param* limit  int
    @yield  (unicode word, unicode xml)
    """
    lastxml = None
    count = 0
    q = self._lookupprefix(t, limit=limit)
    if q:
      for key, xml in q:
        if lastxml != xml:
          yield key, xml
          lastxml = xml
          count += 1
      # Only complete when failed to match any thing
      # For example, 「って」 should not search 「て」
    if complete and not count:
      s = self._complete(t)
      if s and s != t:
        for it in self.lookupprefix(s, limit=limit):
          yield it

  lookup = lookupprefix

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

  #dic = location + 'New Japanese-Chinese Dictionary.ld2'

  #HOME = os.path.expanduser('~')
  #if os.name == 'nt':
  #  from sakurakit import skpaths
  #  APPDATA = skpaths.APPDATA
  #else: # mac
  #  import getpass
  #  HOME = os.path.expanduser('~')
  #  USER = getpass.getuser() # http://stackoverflow.com/questions/842059/is-there-a-portable-way-to-get-the-current-username-in-python
  #  APPDATA = os.path.join(os., '.wine/drive_c/users/' + USER + '/Application Data')
  PWD = os.getcwd()
  dbpath = os.path.join(PWD, '../../../../../../Caches/Dictionaries/Lingoes')
  dbpath = os.path.join(dbpath, 'ja-zh.db')
  #dbpath += 'ja-ko.db'
  #dbpath += 'ja-vi.db'

  ld = LingoesDb(dbpath)

  #if os.path.exists(dbpath):
  #  os.remove(dbpath)

  if not os.path.exists(dbpath):
    with SkProfiler():
      print ld.create(dic, 'utf16')

  print '-' * 4

  with SkProfiler():
    print "lookup start"
    #t = u"かわいい"
    t = u"かわいいっ"
    #t = u"ちょっと"
    #t = u"だしゃれ"
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
