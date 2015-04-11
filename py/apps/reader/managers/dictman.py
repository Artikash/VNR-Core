# coding: utf8
# dictman.py
# 10/10/2012 jichi

#from sakurakit.skprof import SkProfiler

from sakurakit.skclass import memoized
from sakurakit.skdebug import dwarn
from sakurakit.sktr import tr_
from mytr import my
#from kagami import GrimoireBean
import config, convutil, dicts, ebdict, growl, mecabman, rc, settings
import _dictman

@memoized
def manager(): return DictionaryManager()

EMPTY_HTML = '<span style="font:youyuan">%s</span>' % (tr_("Not found") + u"!＞＜")
MIN_HTML_LEN = 290 # empty html length

#@Q_Q
class _DictionaryManager:

  def __init__(self):
    self.enabled = True # bool

  def lookupEdict(self, text, limit=5):
    """
    @param  text  unicode
    @param  limit  int
    @yield  (unicode word, unicode reading, unicode translation)
    """
    #if settings.global_().isEdictEnabled():
    for it in dicts.edict().lookup(text, limit=limit):
      trans = _dictman.render_edict(it.Translation)
      surface = it.Headword
      reading = it.Reading
      if reading == surface:
        reading = None
      if reading:
        roman = convutil.kana2romaji(reading)
        if roman and roman != reading:
          reading = "%s, %s" % (reading, roman)
      yield surface, reading, trans

  def _iterEB(self):
    """
    @yield  EB
    """
    ss = settings.global_()
    if ss.isZhongriEnabled():
      eb = ebdict.zhongri()
      if eb.exists():
        yield eb
      else:
        growl.warn("<br/>".join((
          my.tr("Cannot load {0}").format(u"Zhongri (日中統合辞典)"),
          my.tr("Please double check its location in Preferences."))))

    if ss.isWadokuEnabled():
      eb = ebdict.wadoku()
      if eb.exists():
        yield eb
      else:
        growl.warn("<br/>".join((
          my.tr("Cannot load {0}").format(u"Wadoku (和独辞書)"),
          my.tr("Please double check its location in Preferences."))))

    for lang in config.JMDICT_LANGS:
      if ss.isJMDictEnabled(lang):
        eb = ebdict.jmdict(lang)
        if eb.exists():
          yield eb
        else:
          growl.warn("<br/>".join((
            my.tr("Cannot load {0}").format("JMDict (%s)" % lang),
            my.tr("Please double check its location in Preferences."))))

    if ss.isDaijirinEnabled():
      eb = ebdict.daijirin()
      if eb.exists():
        yield eb
      else:
        growl.warn("<br/>".join((
          my.tr("Cannot load {0}").format(u"DAIJIRIN (大辞林)"),
          my.tr("Please double check its location in Preferences."))))

    if ss.isKojienEnabled():
      eb = ebdict.kojien()
      if eb.exists():
        yield eb
      else:
        growl.warn("<br/>".join((
          my.tr("Cannot load {0}").format(u"KOJIEN6 (広辞苑)"),
          my.tr("Please double check its location in Preferences."))))

  def lookupEB(self, text, limit=3, complete=True): # Use less count to save memory
    """
    @param  text  unicode
    @param* limit  int
    @param* complete  bool  whether complete word
    @yield  unicode
    """
    for eb in self._iterEB():
      count = 0
      for v in eb.render(text):
        yield v
        count += 1
        if count >= limit:
          break
      if complete and not count:
        t = self._completeEB(text)
        if t and t != text:
          for i,v in enumerate(eb.render(t)):
            if i < limit:
              yield v
            else:
              break

  _COMPLETE_TRIM_CHARS = u'ぁ', u'ぇ', u'ぃ', u'ぉ', u'ぅ', u'っ', u'ッ'
  def _completeEB(self, t):
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

  def _iterLD(self):
    """
    @yield  LingoesDic, str language, str category
    """
    ss = settings.global_()
    if ss.isLingoesJaZhGbkEnabled():
      yield dicts.lingoes('ja-zh-gbk'), 'ja-zh', None
    if ss.isLingoesJaZhEnabled():
      yield dicts.lingoes('ja-zh'), 'ja-zh', None
    if ss.isLingoesJaKoEnabled():
      yield dicts.lingoes('ja-ko'), 'ja-ko', 'naver'
    if ss.isLingoesJaViEnabled():
      yield dicts.lingoes('ja-vi'), 'ja-vi', 'ovdp'
    if ss.isLingoesJaEnEnabled():
      yield dicts.lingoes('ja-en'), 'ja-en', 'vicon'

  def lookupLD(self, text, limit=3): # LD seems contains lots of wrong word, use smaller size
    """
    @param  text  unicode
    @param  limit  int
    @yield  unicode source, [unicode xml]
    """
    for db, lang, cat in self._iterLD():
      for word, xml in db.lookup(text, limit=limit):
        xml = _dictman.render_lingoes(xml, cat)
        yield word, xml

class DictionaryManager:

  def __init__(self):
    self.__d = _DictionaryManager()

    #ss = settings.global_()
    #self.enabled = ss.isDictionaryEnabled()
    #ss.dictionaryEnabledChanged.connect(self.setEnabled)

  def isEnabled(self): return self.__d.enabled
  def setEnabled(self, v): self.__d.enabled = v

  def renderKorean(self, text):
    """
    @param  text  Korean phrase
    @return  unicode not None  html
    """
    l = [text]
    romaja = convutil.toroman(text, 'ko')
    if romaja:
      l.append(romaja)
    hanja = convutil.hangul2hanja(text)
    if hanja and hanja != text:
      l.append(hanja)
    feature = ', '.join(l)
    return rc.jinja_template('html/shiori').render({
      'language': 'ko',
      'text': text,
      'feature': feature,
    })

  def renderJapanese(self, text, feature=''): # unicode => unicode
    """
    @param  text  Japanese phrase
    @return  unicode not None  html
    """
    d = self.__d
    if not d.enabled:
      return EMPTY_HTML
    #google = proxy.manager().google_search
    #feature = GrimoireBean.instance.lookupFeature(text)
    if feature:
      feature = mecabman.renderfeature(feature)
    try:
      #with SkProfiler("en-vi"): # 1/8/2014: take 7 seconds for OVDP
      ret = rc.jinja_template('html/shiori').render({
        'language': 'ja',
        'text': text,
        'feature': feature,
        'edict_tuples': d.lookupEdict(text) if settings.global_().isEdictEnabled() else None,
        'ld_tuples': d.lookupLD(text),
        'eb_strings': d.lookupEB(text),
        #'google': google,
        #'locale': d.locale,
      })
    except UnicodeDecodeError, e:
      dwarn(e)
      ret = rc.jinja_template('html/shiori').render({
        'text': text,
        'feature': feature,
        #'google': google,
        #'locale': d.locale,
      })
    except Exception, e: # eb.EBError
      dwarn(e)
      return  EMPTY_HTML
    return ret if len(ret) > MIN_HTML_LEN else EMPTY_HTML

# EOF
