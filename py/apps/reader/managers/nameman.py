# coding: utf8
# nameman.py
# 11/10/2012 jichi

import os, re
from sakurakit import skdatetime, skfileio
from sakurakit.skdebug import dprint, dwarn
from sakurakit.skclass import memoized
from jptraits import jpchars
from mecabjlp import mecabcsv, mecabdic
import config, dataman, os, rc

@memoized
def manager(): return NameManager()

class NameManager(object):

  _rx_name_delim = re.compile(ur'[ 　・＝]') # space, \u3000, dot
  _rx_badname = re.compile(ur'[ 　,]') # space, \u3000, comma

  def __init__(self):
    self.dicname = '' # name
    self.dicdir = '' # path

  def setMeCabDictionary(self, v):
    self.dicname = v
    self.dicdir = os.path.abspath(rc.mecab_dic_path(v)) if v else '' # force absolute path

  # Create name dictionary for MeCab

  @classmethod
  def _goodMeCabName(cls, name):
    """Return true if name has more than 1 characters and does not have katagana
    @param  name  unicode not None
    @return  bool
    """
    # like http://www.getchu.com/soft.phtml?id=779853
    return not cls._rx_badname.search(name) and (
        len(name) > 2 or
        not jpchars.containskata(name) and not jpchars.containspunc(name))

  @classmethod
  def _iterMeCabNameYomi(cls, names):
    """
    @param  names  (unicode name, unicode yomi)
    @yield  (unicode name, unicode yomi)
    """
    for name,yomi in names:
      if cls._goodMeCabName(name):
        yield name, yomi
      if yomi:
        nl = filter(bool, cls._rx_name_delim.split(name))
        if len(nl) > 1:
          yl = filter(bool, cls._rx_name_delim.split(yomi))
          if len(nl) == len(yl):
            first = nl[-1]
            if cls._goodMeCabName(first):
              yield first, yl[-1]
            last = nl[0]
            if cls._goodMeCabName(last):
              yield last, yl[0]
      else: # no yomi
        nl = filter(bool, cls._rx_name_delim.split(name))
        if len(nl) > 1:
          first = nl[-1]
          if cls._goodMeCabName(first):
            yield first, first
          last = nl[0]
          if cls._goodMeCabName(last):
            yield last, last

  def cachedMeCabDic(self, itemId):
    """
    @param  itemId  long
    @return  unicode or None  MeCab dicpath
    """
    if itemId and self.dicname:
      dicpath = rc.mecab_userdic_path(itemId, self.dicname)
      if os.path.exists(dicpath):
        ts = skfileio.fileupdatetime(dicpath)
        if ts + config.NAME_EXPIRE_TIME > skdatetime.CURRENT_UNIXTIME:
          dprint("cache HIT")
          return dicpath
      dprint("cache MISS")

  def compileMeCabDic(self, names, itemId):
    """
    @param  names  [(unicode surface, unicode yomi)]
    @param  itemId  long
    @return  unicode or None  MeCab dicpath
    """
    if itemId and self.dicdir:
      csvpath = rc.mecab_usercsv_path(itemId, self.dicname)
      dir = os.path.dirname(csvpath)
      if not os.path.exists(dir):
        os.makedirs(dir)
      names = self._iterMeCabNameYomi(names) # FIXME: check empty
      if mecabcsv.writecsv(names, csvpath):
        dicpath = rc.mecab_userdic_path(itemId, self.dicname)
        if mecabdic.csv2dic(dicpath, csvpath, dicpath=self.dicdir):
          return dicpath

  # Pares name items for Shared Dictionary

  @staticmethod
  def _goodName(name):
    """Return true if name has more than 1 characters and does not have katagana
    @param  name  unicode not None
    @return  bool
    """
    return not jpchars.containskata(name) and not jpchars.containspunc(name) # like http://www.getchu.com/soft.phtml?id=779853

  @classmethod
  def _iterNameYomi(cls, names):
    """
    @param  (unicode name, unicodde yomi)
    @yield  (unicode name, unicode yomi)
    """
    for name,yomi in names:
      if len(name) > 1:
        allgood = cls._goodName(name)
        if allgood:
          yield name, yomi
        if yomi:
          nl = filter(bool, cls._rx_name_delim.split(name))
          if len(nl) > 1:
            yl = filter(bool, cls._rx_name_delim.split(yomi))
            if len(nl) == len(yl):
              first = nl[-1]
              if len(first) > 1 and (allgood or cls._goodName(first)):
                yield first, yl[-1]
              last = nl[0]
              if len(last) > 1 and (allgood or cls._goodName(last)):
                yield last, yl[0]
              if allgood:
                yield last + first, ' '.join((yl[0], yl[-1]))
        else: # no yomi
          nl = filter(bool, cls._rx_name_delim.split(name))
          if len(nl) > 1:
            first = nl[-1]
            if len(first) > 1 and (allgood or cls._goodName(first)):
              yield first, ''
            last = nl[0]
            if len(last) > 1 and (allgood or cls._goodName(last)):
              yield last, ''
            if allgood:
              yield last + first, ' '.join((last, first))

  #def isNameItemsEnabled(self):
  #  lang = self.user.language
  #  return config.is_kanji_language(lang) or config.is_latin_language(lang)

  def parseNameItems(self, names):
    """
    @param  names  [(unicode surface, unicode hiragana)]
    @return  [dataman.NameItem] not None
    """
    dprint("enter")
    ret = []
    for name,yomi in self._iterNameYomi(names):
      ret.append(dataman.NameItem(text=name, yomi=yomi))
    if ret:
      count = len(ret)
      if count == 1:
        ret[0].id = 1
      else:
        ret.sort(reverse=True, key=lambda it:len(it.text))
        for i in xrange(count):
          ret[i].id = count - i # [count ... 1]
    dprint("leave: len = %i" % len(ret))
    return ret

# EOF
