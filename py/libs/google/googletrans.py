# coding: utf8
# 8/2/2013 jichi
#
# See:
# http://stackoverflow.com/questions/5871710/translate-url-with-google-translate-from-python-script
# https://github.com/mouuff/Google-Translate-API/blob/master/gtranslate.py
if __name__ == '__main__':
  import sys
  sys.path.append('..')

import re, requests
from sakurakit import skstr
from sakurakit.skdebug import dwarn, derror

_LOCALES = {
  'zht': 'zh-TW',
  'zhs': 'zh-CN',
}
def _locale(lang):
  """
  @param  lang  unicode
  @return  unicode
  """
  return _LOCALES.get(lang) or lang

def defaulturl(): return 'http://translate.google.com/m'
def seturl(url):
  """
  @param  url  str
  """
  global GOOGLE_TRANS_API
  GOOGLE_TRANS_API = url

GOOGLE_TRANS_API = defaulturl()

GOOGLE_HEADERS = {
  'User-Agent': "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30)",
  'Accept-Encoding': 'gzip',
}
__rx = re.compile(r'class="t0"\>(.*?)\<') #, re.DOTALL|re.IGNORECASE)
def translate(t, to='auto', fr='auto'):
  """
  @param  t  unicode
  @param* to  str
  @param* fr  str
  @return  unicode or None
  """
  try:
    r = requests.get(GOOGLE_TRANS_API, headers=GOOGLE_HEADERS, params={
      'hl': _locale(to),
      'sl': _locale(fr),
      'q': t,
    })
    h = r.content
    if h:
      m = __rx.search(h)
      if m:
        ret = m.group(1)
        if ret:
          ret = skstr.unescapehtml(ret)
          return ret

  #except socket.error, e:
  #  dwarn("socket error", e.args)
  except requests.ConnectionError, e:
    dwarn("connection error", e.args)
  except requests.HTTPError, e:
    dwarn("http error", e.args)
  #except UnicodeDecodeError, e:
  #  dwarn("unicode decode error", e)
  except (ValueError, KeyError, IndexError, TypeError), e:
    dwarn("json format error", e)
  except Exception, e:
    derror(e)
  dwarn("failed")
  try: dwarn(r.url)
  except: pass

if __name__ == '__main__':
  print translate(u"お花の匂い！\n<そんなワケないお。", to='en', fr='ja')
  print translate(u"Hiragana", to='ja', fr='en')
  #print translate(u"お花の匂い！\nそんなワケないお。", 'en', 'zht')

# EOF
