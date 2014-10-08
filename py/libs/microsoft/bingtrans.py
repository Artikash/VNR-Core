# coding: utf8
# bingtrans.py
# 8/21/2012 jichi
#
# Old API, see: http://www.forum-invaders.com.br/vb/showthread.php/42510-API-do-Bing-para-Traduzir-Textos
if __name__ == '__main__':
  import sys
  sys.path.append('..')

import json, re, requests
from sakurakit.skdebug import dprint, dwarn, derror
from sakurakit.sknetio import GZIP_HEADERS
from sakurakit.skstr import unescapehtml

session = requests # global session

API = "http://api.microsofttranslator.com/v2/ajax.svc/TranslateArray2"

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

def translate(appId, text, to='en', fr='ja'):
  """Return translated text, which is NOT in unicode format
  @param  appId  str
  @param  text  unicode not None
  @param  fr  unicode not None, must be valid language code
  @param  to  unicode not None, must be valid language code
  @return  unicode or None
  """
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
    r = session.get(API,
      #headers=GZIP_HEADERS,
      params={
        'appId': '"%s"' % appId,
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

if __name__ == "__main__":
  import bingauth
  appid = bingauth.getappid()

  #e = create_engine()
  #t = e.translate(u'"こんにちは！"\nこん"fawe\\"にちは！', to='en', fr='ja')
  #t = e.translate(u'こんにちは！\nこんにちは！', to='vi', fr='ja')

  #print len(t)
  #print t

  def test():
    global session

    s = u"""
オープニングやエンディングのアニメーションは単純に主人公を入れ替えた程度の物ではなく、タイトルロゴはもちろん金時や定春の行動や表情、登場する道具（万事屋の面々が乗る車のデザインなど）やクレジット文字など、細部に渡って変更がなされた。更に、坂田金時が『銀魂'』を最終回に追い込み新しいアニメ『まんたま』を始めようとした時にはエンディングや提供表示の煽りコメントが最終回を思わせる演出となり、『まんたま』でも専用のタイトルロゴとオープニングアニメーション（スタッフクレジット付き）が新造され、偽物の提供クレジットまで表示されるなど随所に至るまで徹底的な演出が行われた。また、テレビ欄では金魂篇終了回は『金魂'』最終回として、その翌週は新番組「銀魂'」として案内された。
"""

    fr = 'ja'
    to = 'zhs'

    #s = u"What are you doing?"
    #fr = "en"

    from sakurakit.skprofiler import SkProfiler

    from qtrequests import qtrequests
    from PySide.QtNetwork import QNetworkAccessManager
    session = qtrequests.Session(QNetworkAccessManager())
    with SkProfiler():
      for i in range(1):
        t = translate(appid, s, to=to, fr=fr)
    print t

    session = requests.Session()
    with SkProfiler():
      for i in range(1):
        t = translate(appid, s, to=to, fr=fr)
    print t

    #session = requests
    #e = create_engine()
    #with SkProfiler():
    #  for i in range(10):
    #    t = e.translate(s, to=to, fr=fr)
    #print t

    app.quit()

  from PySide.QtCore import QCoreApplication, QTimer
  app = QCoreApplication(sys.argv)
  QTimer.singleShot(0, test)
  app.exec_()

# EOF
