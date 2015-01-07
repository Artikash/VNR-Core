# coding: utf8
# dictman.py
# 10/10/2012 jichi

from sakurakit.skclass import memoized
from sakurakit.skdebug import dwarn
from sakurakit.sktr import tr_
from mytr import my
#from kagami import GrimoireBean
import config, convutil, dicts, ebdict, growl, mecabman, rc, settings

@memoized
def manager(): return DictionaryManager()

EMPTY_HTML = '<span style="font:youyuan">%s</span>' % (tr_("Not found") + u"!＞＜")
MIN_HTML_LEN = 290 # empty html length

#@Q_Q
class _DictionaryManager:

  @staticmethod
  def lookupEdict(text, limit=5):
    """
    @param  text  unicode
    @param  limit  int
    @yield  cjklib.dictionary.EntryTuple
    """
    return dicts.edict().lookup(text, limit=limit) if settings.global_().isEdictEnabled() else []

  @staticmethod
  def _iterEB():
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

  @classmethod
  def lookupEB(cls, text, limit=3, complete=True): # Use less count to save memory
    """
    @param  text  unicode
    @param* limit  int
    @param* complete  bool  whether complete word
    @yield  unicode
    """
    for eb in cls._iterEB():
      count = 0
      for v in eb.render(text):
        yield v
        count += 1
        if count >= limit:
          break
      if complete and not count:
        t = cls._completeEB(text)
        if t and t != text:
          for i,v in enumerate(eb.render(t)):
            if i < limit:
              yield v
            else:
              break

  _COMPLETE_TRIM_CHARS = u'ぁ', u'ぇ', u'ぃ', u'ぉ', u'ぅ', u'っ', u'ッ'
  @classmethod
  def _completeEB(cls, t):
    """Trim half katagana/hiragana.
    @param  t  unicode
    @return  unicode
    """
    if t:
      while len(t) > 1:
        if t[-1] in cls._COMPLETE_TRIM_CHARS:
          t = t[:-1]
        else:
          break
      while len(t) > 1:
        if t[0] in cls._COMPLETE_TRIM_CHARS:
          t = t[1:]
        else:
          break
    return t

  @staticmethod
  def _iterLD():
    """
    @yield  LingoesDic
    """
    ss = settings.global_()
    if ss.isLingoesJaZhGbkEnabled():
      yield dicts.lingoes('ja-zh-gbk'), 'zh'
    if ss.isLingoesJaZhEnabled():
      yield dicts.lingoes('ja-zh'), 'zh'
    if ss.isLingoesJaKoEnabled():
      yield dicts.lingoes('ja-ko'), 'ko'
    if ss.isLingoesJaViEnabled():
      yield dicts.lingoes('ja-vi'), 'vi'
    if ss.isLingoesJaEnEnabled():
      yield dicts.lingoes('ja-en'), 'en'

  @classmethod
  def lookupLD(cls, text, limit=3): # LD seems contains lots of wrong word, use smaller size
    """
    @param  text  unicode
    @param  limit  int
    @yield  unicode source, [unicode xml]
    """
    for db, lang in cls._iterLD():
      for word, xml in db.lookup(text, limit=limit):
        if lang == 'vi':
          yield word, cls._simplifyLingoesXml(xml)
        else:
          yield word, xml

  # Example Vietnamese dictionary:
  # ちょっと一杯
  # <C><F><H /><K><![CDATA[<ul><li><font color='#cc0000'><b> {ちょっといっぱい}</b></font></li></ul><ul><li><font color='#cc0000'><b> {let's have quick drink}</b></font></li></ul>]] > </K></F></C>
  @staticmethod
  def _simplifyLingoesXml(text): # unicode -> unicode
    return (text
        .replace('<![CDATA[', '').replace(']] >', '').replace(']]>', '')
        .replace("<font color='#cc0000'>", '').replace('</font>', '')
        .replace('<ul>', '').replace('</ul>', '')
        .replace('<li>', '<br/>').replace('</li>', '<br/>')
        .replace('{', '').replace('}', ''))

class DictionaryManager:

  def __init__(self):
    self.enabled = True # bool

    #ss = settings.global_()
    #self.enabled = ss.isDictionaryEnabled()
    #ss.dictionaryEnabledChanged.connect(self.setEnabled)

  def isEnabled(self): return self.enabled
  def setEnabled(self, v): self.enabled = v

  def renderKorean(self, text):
    """
    @param  text  Korean phrase
    @return  unicode not None  html
    """
    romaja = convutil.toroman(text, 'ko')
    hanja = convutil.hangul2hanja(text)
    feature = ', '.join(filter(bool, (romaja, hanja)))
    return rc.jinja_template('html/shiori').render({
      'language': 'ko',
      'text': text,
      'feature': feature,
    })

  def renderJapanese(self, text, feature='', fmt=None): # unicode => unicode
    """
    @param  text  Japanese phrase
    @return  unicode not None  html
    """
    if not self.enabled:
      return EMPTY_HTML
    #google = proxy.manager().google_search
    #feature = GrimoireBean.instance.lookupFeature(text)
    if feature:
      if fmt:
        surf = fmt.getsurface(feature)
        if surf:
          text = surf
        feature = mecabman.renderfeature(feature, fmt)
    try:
      ret = rc.jinja_template('html/shiori').render({
        'language': 'ja',
        'text': text,
        'feature': feature,
        'edict_tuples': _DictionaryManager.lookupEdict(text),
        'ld_tuples': _DictionaryManager.lookupLD(text),
        'eb_strings': _DictionaryManager.lookupEB(text),
        #'google': google,
        #'locale': _DictionaryManager.locale,
      })
    except UnicodeDecodeError, e:
      dwarn(e)
      ret = rc.jinja_template('html/shiori').render({
        'text': text,
        'feature': feature,
        #'google': google,
        #'locale': _DictionaryManager.locale,
      })
    except Exception, e: # eb.EBError
      dwarn(e)
      return  EMPTY_HTML
    return ret if len(ret) > MIN_HTML_LEN else EMPTY_HTML

# EOF
