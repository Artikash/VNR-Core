# coding: utf8
# jpmacros.py
# 1/15/2015 jichi
if __name__ == '__main__':
  import sys
  sys.path.append('..')

import re
from sakurakit.skdebug import dwarn

# Dump from Shared Dictionary at 1/15/2014
MACROS = {
  ':bos:': u'。？！…~～〜♪❤【】＜《（『「“',
  ':eos:': u'。？！…~～〜♪❤【】＞》）』」”',

  ':boc:': u'{{:bos:}}、，―─',
  ':eoc:': u'{{:eos:}}、，―─',

  'boc': u"(?:^|(?<=[{{:boc:}}]))",
  'eoc': u"(?:^|(?<=[{{:eoc:}}]))",

  'bos': u"(?:^|(?<=[{{:bos:}}]))",
  'eos': u"(?:^|(?<=[{{:eos:}}]))",

  ':kanji:': u'一-龥',
  ':hira:': u'あ-ん',
  ':kata:': u'ア-ヴ',
  ':kana:': u'{{:hira:}}{{:kata:}}',

  'kanji': u'[{{:kanji:}}]',
  'hira': u'[{{:hira:}}]',
  'kata': u'[{{:kata:}}]',
  'kana': u'[{{:kana:}}]',
}

_RE_MACRO = re.compile('{{(.+?)}}')

def evalmacros(macros, limit=1000):
  """
  @param  macros  {unicode name:unicode value}
  @param* limit  int  maximum iteration count
  @return  unicode
  """
  for count in xrange(1, limit):
    dirty = False
    for pattern,text in macros.iteritems(): # not iteritems as I will modify ret
      if text and '{{' in text:
        dirty = True
        ok = False
        for m in _RE_MACRO.finditer(text):
          macro = m.group(1)
          repl = macros.get(macro)
          if repl:
            text = text.replace("{{%s}}" % macro, repl)
            ok = True
          else:
            dwarn("missing macro", macro, text)
            ok = False
            break
        if ok:
          macros[pattern] = text
        else:
          macros[pattern] = None # delete this pattern
    if not dirty:
      break
  if count == limit - 1:
    dwarn("recursive macro definition")
evalmacros(MACROS)

def getmacro(name, macros=MACROS):
  """
  @param  name  unicode
  @param* macros  {unicode name:unicode value}
  @return  unicode
  """
  return macros.get(name)

def applymacros(text, macros=MACROS):
  """
  @param  text  unicode
  @param* macros  {unicode name:unicode value}
  @return  unicode
  """
  if text and '{{' in text:
    for m in _RE_MACRO.finditer(text):
      macro = m.group(1)
      repl = macros.get(macro)
      if repl is None:
        dwarn("missing macro:", macro, text)
      else:
        text = text.replace("{{%s}}" % macro, repl)
  return text

if __name__ == '__main__':
  import re

  t = u'す、す、すみません'
  boc = getmacro('boc')
  _re_jitter = re.compile(boc + ur'([あ-んア-ヴ])(?=[、…]\1)')
  t = _re_jitter.sub('xxx', t)
  print t

# EOF
