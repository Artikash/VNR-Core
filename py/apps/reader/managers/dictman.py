# coding: utf8
# dictman.py
# 10/10/2012 jichi

import re
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

def _render_edict(text):
  """
  @param  text  unicode
  @return  unicode  html

  Example: /(n,adj-no) (1) center/centre/middle/heart/core/focus/pivot/emphasis/balance/(suf) (2) -centered/-centred/-focussed/-oriented/centered on/focussed on/(P)/
  """
  if text.startswith('/'):
    text = text[1:]
  if text.endswith('/'):
    text = text[:-1]
  role = ''
  if text.startswith('('): # extract leading word role
    i = text.find(')')
    if i != -1:
      role = text[1:i]
      text = text[i+1:]
  popular = text.endswith('(P)') # deal with trailing (popular) mark
  if popular:
    text = text[:-3]
  if text.endswith('/'): # replace (1) (2) ... by html list
    text = text[:-1]
  # Render list
  t = re.sub(r'\s?\(\d+\)\s?', '<li>', text)
  if t == text:
    text = "<ul><li>%s</ul>" % text
  else:
    if not t.startswith('<li>'):
      i = t.find('<li>')
      if i != -1:
        if role:
          role += u' →' # space + みぎ
        role += t[:i]
        t = t[i:]
    t = t.replace('/<', '<')
    text = "<ol>%s</ol>" % t
  # Render heading
  head = ''
  if role:
    head = '<span class="role">[%s]</span>' % role
  if popular:
    if head:
      head += ' '
    head += '<span class="flag">(common phrase)</div>'
  if head:
    head = '<div class="head">%s</div>' % head
    text = head + text
  return text

# Example Vietnamese dictionary:
# ちょっと一杯
# <C><F><H /><K><![CDATA[<ul><li><font color='#cc0000'><b> {ちょっといっぱい}</b></font></li></ul><ul><li><font color='#cc0000'><b> {let's have quick drink}</b></font></li></ul>]] > </K></F></C>
def _simplify_ovdp_xml(text): # unicode -> unicode
  """
  @param  text  unicode
  @return  unicode
  """
  return (text
      .replace('<![CDATA[', '').replace(']] >', '').replace(']]>', '')
      .replace("<font color='#cc0000'>", '').replace('</font>', '')
      .replace('<ul>', '').replace('</ul>', '')
      .replace('<li>', '<br/>').replace('</li>', '<br/>')
      .replace('{', '').replace('}', ''))

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
    if settings.global_().isEdictEnabled():
      for it in dicts.edict().lookup(text, limit=limit):
        yield it.Headword, it.Reading, _render_edict(it.Translation)

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

  def lookupLD(self, text, limit=3): # LD seems contains lots of wrong word, use smaller size
    """
    @param  text  unicode
    @param  limit  int
    @yield  unicode source, [unicode xml]
    """
    for db, lang in self._iterLD():
      for word, xml in db.lookup(text, limit=limit):
        if lang == 'vi':
          yield word, _simplify_ovdp_xml(xml)
        else:
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

  def renderJapanese(self, text, feature='', fmt=None): # unicode => unicode
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
        'edict_tuples': d.lookupEdict(text),
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
