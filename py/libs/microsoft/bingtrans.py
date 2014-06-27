# coding: utf8
# BingTranslator.py
# 8/21/2012 jichi
#
# Old API, see: http://www.forum-invaders.com.br/vb/showthread.php/42510-API-do-Bing-para-Traduzir-Textos
if __name__ == '__main__':
  import sys
  sys.path.append('..')

import json, re, requests
from sakurakit.skdebug import dwarn, derror
from sakurakit.sknetio import GZIP_HEADERS
from sakurakit.skstr import unescapehtml

# See: http://msdn.microsoft.com/en-us/library/hh456380.aspx
MS_LCODE = {
  'zht': 'zh-CHT',
  'zhs': 'zh-CHS',
}
def _lcode(lang):
  """
  @param  lang  unicode
  @return  unicode
  """
  return MS_LCODE.get(lang) or lang

class _BingTranslator:

  # The RTTAppId is found in LandingPage.js
  # This is the same file containing "TranslateArray2" string
  #
  # Sample minimized js:
  #   ,rttAppId:"TTW3CqZ9Xwce1fOTykYCtIDpQqQgUv-CHSMjp8EzMTTfn6B63_mUiA0QymMqKUpRs",
  #
  # CoffeeScript:
  #   n.Configurations =
  #     serviceName: "LP"
  #     serviceURL: "http://api.microsofttranslator.com/v2/ajax.svc"
  #     baseURL: "http://www.microsofttranslator.com:80/"
  #     locale: j
  #     referrer: b
  #     appId: "TgKz3PgbCYkZ-EfOs_xE8m0y_mcnC9SCYAAYq0YHWAZ8*"
  #     rttAppId: "TTW3CqZ9Xwce1fOTykYCtIDpQqQgUv-CHSMjp8EzMTTfn6B63_mUiA0QymMqKUpRs"
  #     maxNumberOfChars: 5e3
  #     translationLoggerUrl: "TranslationLogger.ashx"
  #     rttEnabled: c
  #     phraseAlignmentEnabled: c
  TOKEN_RE = re.compile(r'rttAppId:"(.*?)"')
  TOKEN_URL = "http://www.bing.com/translator/dynamic/js/LandingPage.js"

  TRANSLATE_URL = "http://api.microsofttranslator.com/v2/ajax.svc/TranslateArray2"

  def __init__(self):
    self._token = None # str

  @property
  def token(self):
    if not self._token:
      self.resetToken()
    return self._token

  def resetToken(self):
    try:
      r = requests.get(_BingTranslator.TOKEN_URL, headers=GZIP_HEADERS)
      t = r.content
      if r.ok and t:
        m = _BingTranslator.TOKEN_RE.search(t)
        if m:
          self._token = m.group(1)
    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except Exception, e:
      derror(e)

class BingTranslator(object):

  def __init__(self):
    self.__d = _BingTranslator()

  def reset(self): self.__d.resetToken()

  def translate(self, text, to='en', fr='ja'):
    """Return translated text, which is NOT in unicode format
    @param  text  unicode not None
    @param  fr  unicode not None, must be valid language code
    @param  to  unicode not None, must be valid language code
    @return  unicode or None
    """
    tok = self.__d.token
    if tok:
      try:
        # See: http://www.forum-invaders.com.br/vb/showthread.php/42510-API-do-Bing-para-Traduzir-Textos
        # Example:
        # http://api.microsofttranslator.com/v2/ajax.svc/TranslateArray2?appId=%22TLxLo1mCVB0gJQETyvO96kBhkckrhqTQ0I6ciRT8M3f0r_QJ3gmiH4tWHK0YQybpK%22&texts=[%22hello%22]&from=%22ja%22&to=%22zh-chs%22

        # 6/5/2014: Example logged by Firefox
        # - Request URL: http://api.microsofttranslator.com/v2/ajax.svc/TranslateArray2?appId=%22TE3gDqQ53Hv5Rr1NzYVbTVjF0Ltz506ddiwMm7gHTA94EhKCSrchQ5ckGy2Po0NRa%22&texts=%5B%22%E6%86%8E%E3%81%97%E3%81%BF%E3%81%AF%E6%86%8E%E3%81%97%E3%81%BF%E3%81%97%E3%81%8B%E7%94%9F%E3%81%BE%E3%81%AA%E3%81%84%22%5D&from=%22%22&to=%22en%22&options=%7B%7D&oncomplete=onComplete_0&onerror=onError_0&_=1402033855022
        # - Request Method: GET
        # - Status Code: HTTP/1.1 200 OK
        # - Response Body:
        #   onComplete_0([{"Alignment":"0:2-19:24 4:6-5:10 7:8-0:3 9:12-12:17","From":"ja","OriginalTextSentenceLengths":[13],"TranslatedText":"Only hatred begets hatred","TranslatedTextSentenceLengths":[25]}]);
        r = requests.get(_BingTranslator.TRANSLATE_URL,
          #headers=GZIP_HEADERS,
          params={
            'appId': '"%s"' % tok,
            'from': '"%s"' % _lcode(fr),
            'to': '"%s"' % _lcode(to),
            'texts': '["%s"]' % '","'.join(text.replace('\\', '\\\\').replace('"', '\\"').split('\n')),
          }
        )

        ret = r.content
        if r.ok and len(ret) > 20:
          i = ret.index('[')
          if i >= 0:
            ret = ret[i:] # skip leading JSONP function name
            l = json.loads(ret)
            if len(l) == 1:
               ret = l[0]['TranslatedText']
            else:
              ret = '\n'.join(it['TranslatedText'] for it in l)
            ret = unescapehtml(ret)
            return ret

      #except socket.error, e:
      #  dwarn("socket error", e.args)
      except requests.ConnectionError, e:
        dwarn("connection error", e.args)
      except requests.HTTPError, e:
        dwarn("http error", e.args)
      #except UnicodeDecodeError, e:
      #  dwarn("unicode decode error", e)
      #except (ValueError, KeyError, IndexError, TypeError), e:
      #  dwarn("json format error", e)
      except Exception, e:
        derror(e)
      dwarn("failed")
      try: dwarn(r.url)
      except: pass

def create_engine(): return BingTranslator()

if __name__ == "__main__":
  import sys
  e = create_engine()
  t = e.translate(u'"こんにちは！"\nこん"fawe\\"にちは！', to='en', fr='ja')
  #t = e.translate(u'こんにちは！\nこんにちは！', to='vi', fr='ja')

  print len(t)
  print t

  #import sys
  #from PySide.QtGui import *
  #a = QApplication(sys.argv)
  #w = QLabel(t)
  #w.show()
  #a.exec_()

# EOF
