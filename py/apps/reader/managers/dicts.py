# coding: utf8
# dicts.py
# 11/25/2012 jichi

import os
#from PySide.QtCore import QMutex
from sakurakit.skclass import memoized, memoizedproperty
from sakurakit.skdebug import dwarn
from sakurakit.skthreads import SkMutexLocker
from sakurakit.sktr import tr_
from mytr import my
import growl, rc, res

class Dict(res.Resource):
  def __init__(self, *args, **kwargs): super(Dict, self).__init__(*args, **kwargs)

  def lookup(self, *args, **kwargs): pass

class Edict(Dict):
  def __init__(self):
    super(Edict, self).__init__(
      path=os.path.join(rc.EDICT_PATH),
      lockpath=os.path.join(rc.DIR_TMP, "edict.lock"),
    )

  def get(self): # override
    from scripts import edict
    return edict.get()

  @memoizedproperty
  def d(self):
    from dictdb import edictdb
    return edictdb.Edict(self.path)

  def lookup(self, *args, **kwargs): # override
    if self.valid():
      for it in self.d.lookup(*args, **kwargs):
        yield it
    else:
      growl.warn(my.tr("{0} does not exist. Please try redownload it in Preferences").format('EDICT'))

  def translate(self, *args, **kwargs):
    d = self.d
    if d.valid():
      return d.translate(*args, **kwargs)
    else:
      growl.warn(my.tr("{0} does not exist. Please try redownload it in Preferences").format('EDICT'))

class MeCabEdict(res.Resource):

  def __init__(self):
    from sakurakit import skpaths
    super(MeCabEdict, self).__init__(
      path=os.path.join(rc.MECAB_EDICT_PATH),
      lockpath=os.path.join(rc.DIR_TMP, "mecab-edict.lock"),
    )

  def get(self): # override
    from scripts import edict
    return edict.align()

class LingoesDict(Dict):
  def __init__(self, lang): # string, ex. 'ja-en'
    self.lang = lang # str
    super(LingoesDict, self).__init__(
      path=os.path.join(rc.DIR_CACHE_DICT, "Lingoes/%s.db" % lang),
      lockpath=os.path.join(rc.DIR_TMP, "lingoes.%s.lock" % lang),
    )

  @memoizedproperty
  def d(self):
    from lingoes import lingoesdb
    return lingoesdb.LingoesDb(self.path)

  def lookup(self, *args, **kwargs): # override
    if self.valid():
      return self.d.lookup(*args, **kwargs)
    else:
      growl.warn(my.tr("{0} does not exist. Please try redownload it in Preferences").format('Lingoes ' + self.lang))

  def get(self): # override
    from scripts import lingoes
    return lingoes.get(self.lang)

  def translate(self, t):
    """
    @param  unicode
    @return  unicode or None
    """
    if not self.valid():
      #growl.warn(my.tr("{0} does not exist. Please try redownload it in Preferences").format('Lingoes ' + self.lang))
      return

    parse = self._getTranslationParser()
    if parse:
      import sqlite3
      from dictdb import dictdb
      try:
        with sqlite3.connect(self.path) as conn:
          t = dictdb.queryentry(conn.cursor(), t, select=dictdb.SELECT_CONTENT)
          if t:
            return parse(t[0])
      except Exception, e: dwarn(e)

  def _getTranslationParser(self):
    """
    @return  unicode -> unicode  or None
    """
    if self.lang  == 'ja-zh':
      from dictp import jazhdictp
      return jazhdictp.parsedef
    if self.lang  == 'ja-zh-gbk':
      from dictp import gbkdictp
      return gbkdictp.parsedef
    if self.lang  == 'ja-ko':
      from dictp import naverdictp
      return naverdictp.parsedef

class StarDict(Dict):
  def __init__(self, lang): # string, ex. 'ja-en'
    import config
    self.lang = lang # str
    super(StarDict, self).__init__(
      path=os.path.join(config.STARDICT_LOCATIONS[lang]),
      lockpath=os.path.join(rc.DIR_TMP, "stardict.%s.lock" % lang),
    )

  def exists(self): # override
    return os.path.exists(self.path + '.dict')

  @memoizedproperty
  def d(self):
    from stardict import stardict
    ret = stardict.StarDict(self.path)
    ret.init()
    return ret

  def lookup(self, text): # override
    d = self.d
    if d.valid():
      return d.query(text)
    else:
      growl.warn(my.tr("{0} does not exist. Please try redownload it in Preferences").format('StarDict ' + self.lang))

  def get(self): # override
    from scripts import stardict
    return stardict.get(self.lang)

  def remove(self): # override
    from sakurakit import skfileio
    return skfileio.removetree(os.path.dirname(self.path))

  def translate(self, t, **kwargs):
    """
    @param  t  unicode
    @param*  kwargs  parameters passed to parse
    @return  unicode or None
    """
    d = self.d
    if not d.valid():
      growl.warn(my.tr("{0} does not exist. Please try redownload it in Preferences").format('StarDict ' + self.lang))
      return
    parse = self._getTranslationParser()
    if parse:
      from dictp import ovdpdictp
      q = d.query(t)
      if q:
        for it in q:
          ret = parse(it, **kwargs)
          if ret:
            return ret

  def _getTranslationParser(self):
    """
    @return  unicode -> unicode  or None
    """
    if self.lang  == 'ja-vi':
      from dictp import ovdpdictp
      return ovdpdictp.parsedef

class JMDict(Dict):
  def __init__(self, lang): # string, ex. 'fr', 'ru', 'nl'
    self.lang = lang # str
    super(JMDict, self).__init__(
      path=os.path.join(rc.DIR_CACHE_DICT, "JMDict/%s.fpw" % lang),
      lockpath=os.path.join(rc.DIR_TMP, "jmdict.%s.lock" % lang),
    )

  def get(self): # override
    from scripts import jmdict
    return jmdict.get(self.lang)

  def remove(self): # override
    return self.removetree()

class Wadoku(Dict):
  def __init__(self):
    super(Wadoku, self).__init__(
      path=os.path.join(rc.DIR_CACHE_DICT, "Wadoku"),
      lockpath=os.path.join(rc.DIR_TMP, "wadoku.lock"),
    )

  def get(self): # override
    from scripts import wadoku
    return wadoku.get()

  def remove(self): # override
    return self.removetree()

# MeCab

#class IPADIC(Dict):
#  def __init__(self):
#    super(IPADIC, self).__init__(
#      path=os.path.join(rc.DIR_CACHE_DICT, "IPAdic"),
#      lockpath=os.path.join(rc.DIR_TMP, "ipadic.lock"),
#    )
#
#  def get(self): # override
#    from scripts import ipadic
#    return ipadic.get()
#
#  def remove(self): # override
#    return self.removetree()

class UniDic(Dict):
  def __init__(self):
    super(UniDic, self).__init__(
      path=os.path.join(rc.DIR_UNIDIC),
      lockpath=os.path.join(rc.DIR_TMP, "unidic.lock"),
    )

  def get(self): # override
    from scripts import unidic
    return unidic.get()

  def remove(self): # override
    return self.removetree()

#class UNIDICMLJ(Dict):
#  def __init__(self):
#    super(UNIDICMLJ, self).__init__(
#      path=os.path.join(rc.DIR_CACHE_DICT, "UniDicMLJ"),
#      lockpath=os.path.join(rc.DIR_TMP, "unidicmlj.lock"),
#    )
#
#  def get(self): # override
#    from scripts import unidicmlj
#    return unidicmlj.get()
#
#  def remove(self): # override
#    return self.removetree()

# Global objects

@memoized
def edict(): return Edict()

@memoized
def mecabedict(): return MeCabEdict()

@memoized
def unidic(): return UniDic()

@memoized
def wadoku(): return Wadoku()

LINGOES = {} # {str name:LingoesDict}
def lingoes(name):
  """
  @param  name  str  such as ja-en
  @return  LingoesDict
  """
  ret = LINGOES.get(name)
  if not ret:
    LINGOES[name] = ret = LingoesDict(name)
  return ret

STARDICT = {} # {str name:LingoesDict}
def stardict(name):
  """
  @param  name  str  such as ja-en
  @return  StarDict
  """
  ret = STARDICT.get(name)
  if not ret:
    STARDICT[name] = ret = StarDict(name)
  return ret

JMDICT = {} # {str lang:JMDictic}
def jmdict(lang):
  """
  @param  lang  str  such as fr, ru, nl
  @return  JMDict
  """
  ret = JMDICT.get(lang)
  if not ret:
    JMDICT[lang] = ret = JMDict(lang)
  return ret

# EOF

#MECAB_CLASS = {
#  'ipadic': IPADIC,
#  'unidic': UNIDIC,
#  'unidic-mlj': UNIDICMLJ,
#}
#MECAB = {} # {str name:Dict}
#def mecab(name):
#  """
#  @param  name  str  such as 'ipadic'
#  @return  Dict
#  """
#  ret = MECAB.get(name)
#  if not ret:
#    MECAB[name] = ret = MECAB_CLASS[name]()
#  return ret
#
#CABOCHA_CLASS = {
#  'ipadic': IPADICCaboChaModel,
#  'unidic': UNIDICCaboChaModel,
#  #'juman': JUMANCaboChaModel,
#}
#CABOCHA = {} # {str name:Dict}
#def cabocha(name):
#  """
#  @param  name  str  such as 'ipadic'
#  @return  CaboChaModel
#  """
#  ret = CABOCHA.get(name)
#  if not ret:
#    CABOCHA[name] = ret = CABOCHA_CLASS[name]()
#  return ret

#@memoized
#def edict():
#  from edict import edict as E
#  MIN_EDICT_SIZE = 20 * 1024 * 1024 # 20MB
#  for path in rc.iter_dict_paths('edict'):
#    if os.path.exists(path) and skfileio.filesize(path) > MIN_EDICT_SIZE:
#      ret = E.Edict(path)
#      if ret.valid():
#        return ret
#  return E.NullEdict()

#@memoized
#def enamdict():
#  from edict import edict
#  return edict.Edict(config.ENAMDICT_LOCATION)

# CaboCha

#class CaboChaModel(Dict):
#  def __init__(self, dic):
#    self.dic = dic # str, one of ipadic, unidic, and juman
#    super(CaboChaModel, self).__init__(
#      path=os.path.join(rc.DIR_CACHE_DICT, "CaboCha/%s" % dic),
#      lockpath=os.path.join(rc.DIR_TMP, "cabocha.%s.lock" % dic),
#    )
#
#  def get(self): # override
#    from scripts import cabocha
#    return cabocha.get(self.dic)
#
#  def remove(self): # override
#    return self.removetree()
#
#class IPADICCaboChaModel(CaboChaModel):
#  def __init__(self,):
#    super(IPADICCaboChaModel, self).__init__('ipadic')
#
#class UNIDICCaboChaModel(CaboChaModel):
#  def __init__(self,):
#    super(UNIDICCaboChaModel, self).__init__('unidic')

#class JUMANCaboChaModel(CaboChaModel):
#  def __init__(self,):
#    super(JUMANCaboChaModel, self).__init__('juman')

