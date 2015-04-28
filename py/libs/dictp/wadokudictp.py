# coding: utf8
# wadokudictp.py
# 4/27/2015 jichi
from unitraits.uniconv import wide2thin

def parsedef(t):
  """Get short definition out of translation
  @param  t  unicode
  @return  unicode
  """
  DELIM = '<br/>'
  i = t.find(DELIM)
  if i != -1:
    t = t[i+len(DELIM):]
  i = t.find(DELIM)
  if i != -1:
    t = t[:i]
  if t.startswith('<span'):
    STOP = '</span>'
    i = t.find(STOP)
    if i != -1:
      t = t[i+len(STOP):]
  for c in u'，．；（':
    i = t.find(c)
    if i != -1:
      t = t[:i]
  i = t.find(u'｜｜')
  if i != -1:
    t = t[i+3:] # '|| '
  t = t.strip()
  for s in u'［］', u'（）':
    if t and t[0] == s[0]:
      i = t.find(s[1])
      if i != -1:
        t = t[i+1:].lstrip()
    if t and t[-1] == s[0]:
      i = t.find(s[1])
      if i != -1:
        t = t[:i].rstrip()
  if t and t[-1] == u'…':
    t = t[:-1].rstrip()
  if t:
    for c in u'<>［］｜−·':
      if c in t:
        return ''
    t = wide2thin(t)
  return t

# EOF
