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

  mecabfmt = mecabformat.UNIDIC_FORMATTER

  def __init__(self):
    self.japaneseLookupEnabled = True # bool
    self.japaneseTranslateLanguages = [] # [str]

  def lookupEdict(self, text, feature=None, limit=5):
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
      v = self.mecabfmt.getsource(feature)
      if v and '-' not in v and v != text:
        kwargs['surfaces'].append(v)
      # id is ignored and not needed
      #if fmt.getdictype(feature) == 'edict':
      #  id = fmt.getdicid(feature)
      #  if id:
      #    kwargs['id'] = id

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

  def lookupEB(self, text, limit=3, feature=None, complete=True): # Use less count to save memory
    """
    @param  text  unicode
    @param* limit  int
    @param* feature  unicode or None  mecab feature
    @param* complete  bool  whether complete word
    @yield  unicode
    """
    source = None
    if feature:
      v = self.mecabfmt.getsource(feature)
      if v and '-' not in v and v != text:
        source = v
    for eb in self._iterEB():
      count = 0
      for v in eb.render(text):
        yield v
        count += 1
        if count >= limit:
          break
      if source:
        for v in eb.render(source):
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
    #if ss.isLingoesJaViEnabled():
    #  yield dicts.lingoes('ja-vi'), 'ja-vi', 'ovdp'
    if ss.isLingoesJaEnEnabled():
      yield dicts.lingoes('ja-en'), 'ja-en', 'vicon'

  def lookupLD(self, text, exact=True, feature=None, limit=3): # LD seems contains lots of wrong word, use smaller size
    """
    @param  text  unicode
    @param* exact  bool
    @param* feature  unicode or None
    @param* limit  int
    @yield  unicode source, unicode html
    """
    source = None
    if feature:
      v = self.mecabfmt.getsource(feature)
      if v and '-' not in v and v != text:
        source = v
    for db, lang, cat in self._iterLD():
      count = 0
      for word, xml in db.lookup(text, exact=exact, limit=limit, complete=not source):
        count += 1
        xml = _dictman.render_lingoes(xml, cat)
        yield word, xml
      if count < limit and source:
        for word, xml in db.lookup(source, exact=exact, limit=limit - count, complete=False):
          xml = _dictman.render_lingoes(xml, cat)
          yield word, xml

  def _iterStarDict(self):
    """
    @yield  StarDict, str category
    """
    ss = settings.global_()
    if ss.isStardictJaViEnabled():
      yield dicts.stardict('ja-vi'), 'ovdp'

  def lookupStarDict(self, text, exact=True, feature=None, limit=3): # LD seems contains lots of wrong word, use smaller size
    """
    @param  text  unicode
    @param* exact  bool
    @param* feature  unicode or None
    @param* limit  int
    @yield  unicode source, unicode html
    """
    source = None
    if feature:
      v = self.mecabfmt.getsource(feature)
      if v and '-' not in v and v != text:
        source = v
    for dic, cat in self._iterStarDict():
      count = 0
      q = dic.lookup(text)
      if q:
        for html in q:
          html = _dictman.render_stardict(html, cat)
          yield text, html
          count += 1
          if count >= limit:
            break
      if count < limit and source:
        q = dic.lookup(source)
        if q:
          for html in q:
            html = _dictman.render_stardict(html, cat)
            yield source, html
            if count >= limit:
              break

  def lookupDB(self, text, exact=True, feature=None): # LD seems contains lots of wrong word, use smaller size
    """
    @param  text  unicode
    @param* exact  bool
    @param* feature  unicode
    @yield  unicode source, unicode html
    """
    if settings.global_().isEdictEnabled():
      for it in self.lookupEdict(text, feature=feature):
        yield it
    for it in self.lookupStarDict(text, feature=feature):
      yield it
    for it in self.lookupLD(text, feature=feature, exact=exact):
      yield it

  def translateJapanese(self, t):
    """
    @param  unicode
    @return  unicode or None
    """
    MAX_LENGTH = len(t) * 20
    ret = None
    if not ret and 'zh' in self.japaneseTranslateLanguages:
      for dic in 'ja-zh', 'ja-zh-gbk':
        ret = dicts.lingoes(dic).translate(t)
        if ret and len(ret) < MAX_LENGTH:
          if self.userLanguage == 'zht':
            ret = convutil.zhs2zht(ret)
          return ret

    if 'ko' in self.japaneseTranslateLanguages:
      ret = dicts.lingoes('ja-ko').translate(t)
      if ret and len(ret) < MAX_LENGTH:
        return ret

    if 'vi' in self.japaneseTranslateLanguages:
      ret = dicts.stardict('ja-vi').translate(t)
      if ret and len(ret) < MAX_LENGTH:
        return ret

    if 'de' in self.japaneseTranslateLanguages:
      ret = ebdict.wadoku().translate(t)
      if ret and len(ret) < MAX_LENGTH:
        return ret

    for lang in config.JMDICT_LANGS:
      if lang in self.japaneseTranslateLanguages:
        ret = ebdict.jmdict(lang).translate(t)
        if ret and len(ret) < MAX_LENGTH:
          return ret

    if not ret and 'en' in self.japaneseTranslateLanguages:
      ret = dicts.stardict('ja-vi').translate(t, english=True)
      if ret and len(ret) < MAX_LENGTH:
        return ret

class DictionaryManager:

  def __init__(self):
    self.__d = _DictionaryManager()

    #ss = settings.global_()
    #self.enabled = ss.isDictionaryEnabled()
    #ss.dictionaryEnabledChanged.connect(self.setEnabled)

  def setUserLanguage(self, v): self.__d.userLanguage = v
  def userLanguage(self): return self.__d.userLanguage

  def translateJapanese(self, t):
    """
    @param  text
    @return  unicode or None
    """
    return self.__d.translateJapanese(t)

  def japaneseTranslateLanguages(self): return self.__d.japaneseTranslateLanguages
  def setJapaneseTranslateLanguages(self, v): self.__d.japaneseTranslateLanguages = v

  def isJapaneseLookupEnabled(self): return self.__d.japaneseLookupEnabled
  def setJapaneseLookupEnabled(self, t): self.__d.japaneseLookupEnabled = t

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

  def renderJapanese(self, text, exact=True, feature=''): #
    """
    @param  text  unicode  Japanese phrase
    @param  exact  bool  wheher do exact match  exact match faster but return less phrases
    @param* feature  unicode  MeCab feature
    @return  unicode not None  html
    """
    #if not d.japaneseLookupEnabled:
    #  return EMPTY_HTML
    #google = proxy.manager().google_search
    #feature = GrimoireBean.instance.lookupFeature(text)
    try:
      d = self.__d
      #with SkProfiler("en-vi"): # 1/8/2014: take 7 seconds for OVDP
      ret = rc.jinja_template('html/shiori').render({
        'language': 'ja',
        'text': text,
        'feature': mecabman.renderfeature(feature),
        'tuples': d.lookupDB(text, exact=exact, feature=feature),
        'eb_strings': d.lookupEB(text, feature=feature), # exact not used, since it is already very fast
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
