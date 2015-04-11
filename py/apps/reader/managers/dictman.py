# coding: utf8
# dictman.py
# 10/10/2012 jichi

#from sakurakit.skprof import SkProfiler

from sakurakit.skclass import memoized
from sakurakit.skdebug import dwarn
from sakurakit.sktr import tr_
from mecabparser import mecabformat
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

  def lookupEdict(self, text, feature=None, limit=5, fmt=mecabformat.UNIDIC_FORMATTER):
    """
    @param  text  unicode
    @param  feature  unicode
    @param  limit  int
    @yield  (unicode word, unicode reading, unicode translation)
    """
    kwargs = {'surfaces':[text]}
    v = convutil.kata2hira(text)
    if v != text:
      kwargs['surfaces'].append(v)
    reading = None
    if feature:
      v = fmt.getkanji(feature)
      if v and '-' not in v and v != text:
        kwargs['surfaces'].append(v)
      if fmt.gettype(feature) == 'edict':
        id = fmt.getid(feature)
        if id:
          kwargs['id'] = id

      #kata = fmt.getkata(feature)
      #if kata:
      #  reading = conv.kata2hira(kata)
      #  romaji = convutil.kata2romaji(kata)
      #  if romaji and romaji != reading:
      #    reading = "%s, %s" % (reading, roman)

    for id, word, content in dicts.edict().lookup(limit=limit, **kwargs):
      html = _dictman.render_edict(content)
      yield word, html

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

  def lookupLD(self, text, exact=False, limit=3): # LD seems contains lots of wrong word, use smaller size
    """
    @param  text  unicode
    @param* exact  bool
    @param* limit  int
    @yield  unicode source, unicode html
    """
    for db, lang, cat in self._iterLD():
      for word, xml in db.lookup(text, exact=exact, limit=limit):
        xml = _dictman.render_lingoes(xml, cat)
        yield word, xml

  def lookupDB(self, text, exact=False, feature=None): # LD seems contains lots of wrong word, use smaller size
    """
    @param  text  unicode
    @param* exact  bool
    @param* feature  unicode
    @yield  unicode source, unicode html
    """
    if settings.global_().isEdictEnabled():
      for it in self.lookupEdict(text, feature=feature):
        yield it
    for it in self.lookupLD(text, exact=exact):
      yield it

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

  def renderJapanese(self, text, exact=False, feature=''): #
    """
    @param  text  unicode  Japanese phrase
    @param  exact  bool  wheher do exact match  exact match faster but return less phrases
    @param* feature  unicode  MeCab feature
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
        'tuples': d.lookupDB(text, exact=exact, feature=feature),
        'eb_strings': d.lookupEB(text), # exact not used, since it is already very fast
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
