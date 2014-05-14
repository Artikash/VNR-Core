# coding: utf8
# sktr.py
# 11/1/2012 jichi
import re
from sakurakit.skdebug import dwarn

# http://stackoverflow.com/questions/196345/how-to-check-if-a-string-in-python-is-in-ascii
def isascii(s):
  try: s.decode('ascii'); return True
  except UnicodeDecodeError: return False
  except Exception, e:
    dwarn(e)
    return False

def signed_ord(c):
  """Return signed char value of the character
  @param  c  char
  @return  int[-128,127]
  """
  ret = ord(c)
  return ret if ret < 128 else ret - 256

# http://stackoverflow.com/questions/6609895/efficiently-replace-bad-characters
def multireplacer(table, flags=0, escape=False, prefix=None, suffix=None):
  """
  @param  table  {unicode fr:unicode to}
  @param* flags  re compile flags  However, re.IGNORECASE will break the dictionary
  @param* prefix  str or None
  @param* suffix  str or None
  @return  fn(text)
  """
  it = table.iterkeys() if escape else (re.escape(k) for k in table.iterkeys())
  l = sorted(it, key=len, reverse=True) # sort based on length of the string
  #l.reverse() # descendant
  pat = '(%s)' % '|'.join(l)
  if suffix:
    if escape:
      suffix = re.escape(suffix)
    pat += suffix
  if prefix:
    if escape:
      prefix = re.escape(prefix)
    pat = prefix + pat
  rx = re.compile(pat, flags)
  def ret(text):
    def replace(match):
      char = match.group(1)
      return table[char]
    return rx.sub(replace, text)
  return ret

def removebr(t):
  """Remove leading and trailing br
  @param  t  unicode
  @return  unicode
  """
  return t.replace('<br>', '').replace('<BR>', '')

__removeruby_rx = re.compile(r"</?ruby>|</?rb>|</?rt>|</?rp>", re.IGNORECASE)
def removeruby(t):
  """Remove leading and trailing br
  @param  t  unicode
  @return  unicode
  """
  return __removeruby_rx.sub('', t)

__stripbr_rx = re.compile(r"^(?:\s*<br>)+|(?:<br>\s*)+$", re.IGNORECASE)
def stripbr(t):
  """Remove leading and trailing br
  @param  t  unicode
  @return  unicode
  """
  return __stripbr_rx.sub('', t).strip()

def uniqbr(t, repeat=2):
  """Remove duplicate br
  @param  t  unicode
  @param* repeat  int  maximum number of repetition
  @return  unicode
  """
  pat = r"(?:\s*<br>\s*){%i,}" % repeat
  repl = "<br>" * (repeat - 1)
  return re.sub(pat, repl, t, re.IGNORECASE)

from HTMLParser import HTMLParser
_HP = HTMLParser()
# @param  t  unicode
# @return  unicode
unescapehtml = _HP.unescape

# @param  t  unicode
# @return  unicode
from cgi import escape as escapehtml

# See: http://stackoverflow.com/questions/1695183/how-to-percent-encode-url-parameters-in-python
import urllib
def urlencode(text, safe=''):
  """
  @param  text  str not unicode
  @param* safe  str  characters do not encode
  @param* encoding  str
  @param* errors  str
  @return  str not unicode
  """
  return urllib.quote(text, safe=safe) # to percentage encoding

def urldecode(text):
  """
  @param  text  str
  @param* encoding  str
  @param* errors  str
  @return  str not unicode
  """
  text = urllib.unquote(text) # from percentage encoding

if __name__ == '__main__':
  print urlencode('hello world')
  print multireplacer({
      '1': 'a',
      '123': 'b',
      })('123')

# EOF

# See: http://stackoverflow.com/questions/919056/python-case-insensitive-replace
#def remove(source, pattern, flags=0):
#  """
#  @param  pattern  str or None
#  @param  source  str or None
#  @param  flags  re flags
#  @return  str or ""
#  """
#  return ("" if not source or not pattern else
#      re.compile(re.escape(pattern), flags).sub(source, "") if flags else
#      source.replace(pattern, ""))

