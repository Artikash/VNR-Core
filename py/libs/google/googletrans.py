# coding: utf8
# 8/2/2013 jichi
#
# See:
# http://stackoverflow.com/questions/5871710/translate-url-with-google-translate-from-python-script
# https://github.com/mouuff/Google-Translate-API/blob/master/gtranslate.py
if __name__ == '__main__':
  import sys
  sys.path.append('..')

import requests
from sakurakit.skdebug import dwarn, derror
from sakurakit.skstr import unescapehtml

session = requests # global session

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

#def defaulturl(): return 'https://translate.google.com/m'
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
#__rx = re.compile(r'class="t0"\>(.*?)\<') #, re.DOTALL|re.IGNORECASE)
_TEXT_BEGIN = 'class="t0">' # faster than re
_TEXT_END = "<"
def translate(t, to='auto', fr='auto'):
  """
  @param  t  unicode
  @param* to  str
  @param* fr  str
  @return  unicode or None
  """
  try:
    r = session.get(GOOGLE_TRANS_API, headers=GOOGLE_HEADERS, params={
      'hl': _locale(to),
      'sl': _locale(fr),
      'q': t,
    })
    h = r.content
    if h:
      start = h.find(_TEXT_BEGIN)
      if start > 0:
        start += len(_TEXT_BEGIN)
        stop = h.find(_TEXT_END, start)
        if stop > 0:
          h = h[start:stop]
          return unescapehtml(h)

  #except socket.error, e:
  #  dwarn("socket error", e.args)
  except requests.ConnectionError, e:
    dwarn("connection error", e.args)
  except requests.HTTPError, e:
    dwarn("http error", e.args)
  #except UnicodeDecodeError, e:
  #  dwarn("unicode decode error", e)
  except (ValueError, KeyError, IndexError, TypeError), e:
    dwarn("format error", e)
  except Exception, e:
    derror(e)
  dwarn("failed")
  try: dwarn(r.url)
  except: pass

if __name__ == '__main__':
  #print translate(u"お花の匂い！\n<そんなワケないお。", to='en', fr='ja')
  #print translate(u"Hiragana", to='ja', fr='en')
  #print translate(u"お花の匂い！\nそんなワケないお。", 'en', 'zht')
  def test():
    global session

    #s = u"""オープニングやエンディングのアニメーションは単純に主人公を入れ替えた程度の物ではなく、タイトルロゴはもちろん金時や定春の行動や表情、登場する道具（万事屋の面々が乗る車のデザインなど）やクレジット文字など、細部に渡って変更がなされた。更に、坂田金時が『銀魂'』を最終回に追い込み新しいアニメ『まんたま』を始めようとした時にはエンディングや提供表示の煽りコメントが最終回を思わせる演出となり、『まんたま』でも専用のタイトルロゴとオープニングアニメーション（スタッフクレジット付き）が新造され、偽物の提供クレジットまで表示されるなど随所に至るまで徹底的な演出が行われた。また、テレビ欄では金魂篇終了回は『金魂'』最終回として、その翌週は新番組「銀魂'」として案内された。"""
    s = u"お花の匂い！"

    fr = 'ja'
    to = 'zhs'

    #s = u"What are you doing?"
    #fr = "en"

    from sakurakit.skprofiler import SkProfiler

    # Does not work because
    # 1. need GZIP
    # 2. need redirect from http:// to https://
    from qtrequests import qtrequests
    from PySide.QtNetwork import QNetworkAccessManager
    session = qtrequests.Session(QNetworkAccessManager())
    with SkProfiler():
      for i in range(1):
        t = translate(s, to=to, fr=fr)
    #print t

    session = requests.Session()
    with SkProfiler():
      for i in range(1):
        t = translate(s, to=to, fr=fr)
    print t

    #session = requests
    #with SkProfiler():
    #  for i in range(10):
    #    t = translate(s, to=to, fr=fr)
    #print t

    app.quit()

  from PySide.QtCore import QCoreApplication, QTimer
  app = QCoreApplication(sys.argv)
  QTimer.singleShot(0, test)
  app.exec_()

# EOF
