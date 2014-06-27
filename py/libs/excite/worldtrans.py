# coding: utf8
# worldtrans.py
# 7/5/2012 jichi
#
# URLs
if __name__ == '__main__':
  import sys
  sys.path.append('..')

import re, requests
from sakurakit.skdebug import dwarn, derror
from sakurakit.sknetio import GZIP_HEADERS
from sakurakit.skstr import unescapehtml

EXCITE_API =  "http://www.excite.co.jp/world/"
def api(to='en', fr='ja'):
  """
  @return  str
  @raise  KeyError
  """
  FR, TO = fr.upper(), to.upper()
  langs = fr, to
  if 'en' in langs:
    return ''.join((EXCITE_API, "english/?wb_lp=", FR, TO)) # http://www.excite.co.jp/world/english/?wb_lp=jaen
  elif 'zhs' in langs:
    return ''.join((EXCITE_API, "chinese/?wb_lp=", FR[:2], TO[:2]))
  elif 'zht' in langs:
    return ''.join((EXCITE_API, "chinese/?big5=yes&wb_lp=", FR[:2], TO[:2]))
  elif 'ko' in langs:
    return ''.join((EXCITE_API, "korean/?wb_lp=", FR, TO))
  elif 'fr' in langs:
    return ''.join((EXCITE_API, "french/?wb_lp=", FR, TO))
  elif 'de' in langs:
    return ''.join((EXCITE_API, "german/?wb_lp=", FR, TO))
  elif 'it' in langs:
    return ''.join((EXCITE_API, "italian/?wb_lp=", FR, TO))
  elif 'es' in langs:
    return ''.join((EXCITE_API, "spanish/?wb_lp=", FR, TO))
  elif 'pt' in langs:
    return ''.join((EXCITE_API, "portuguese/?wb_lp=", FR, TO))
  elif 'ru' in langs:
    return ''.join((EXCITE_API, "russian/?wb_lp=", FR, TO))
  else:
    #raise KeyError(langs) # English by default
    return ''.join((EXCITE_API, "english/?wb_lp=", FR, TO))

__re_search = re.compile(r"%s%s%s" % (
  re.escape(' name="after">'),
  r'(.*?)',
  re.escape('</textarea>')
), re.DOTALL|re.IGNORECASE)
def translate(text, to='en', fr='ja'):
  """Return translated text, which is NOT in unicode format
  @param  text  unicode not None
  @param  fr  unicode not None, must be valid language code
  @param  to  unicode not None, must be valid language code
  @return  unicode or None
  """
  try:
    r = requests.get(api(to, fr),
      headers=GZIP_HEADERS,
      params={
        'before': text
      }
    )

    #print r.headers['Content-Type']
    ret = r.content

    if r.ok and len(ret) > 1000:
      # Extract text within '<textarea .*name="after">' and '</textarea>'
      m = __re_search.search(ret)
      if m:
        ret = m.group(1)
        ret = ret.decode('utf8', errors='ignore')
        ret = unescapehtml(ret)
      else:
        dwarn("content not matched: %s" % ret)
    return ret

  #except socket.error, e:
  #  dwarn("socket error", e.args)
  except requests.ConnectionError, e:
    dwarn("connection error", e.args)
  except requests.HTTPError, e:
    dwarn("http error", e.args)
  #except UnicodeDecodeError, e:
  #  dwarn("unicode decode error", e)
  except KeyError, e:
    dwarn("language error", e)
  #except KeyError, e:
  #  dwarn("invalid response header", e.args)
  except Exception, e:
    derror(e)

  dwarn("failed")

  try: dwarn(r.url)
  except: pass

  return ""

if __name__ == '__main__':
  t = translate(u"あのね\nすもももももももものうち！", 'zht')
  import sys
  from PySide.QtGui import *
  a = QApplication(sys.argv)
  w = QLabel(t)
  w.show()
  a.exec_()

# EOF
