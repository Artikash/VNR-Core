# coding: utf8
# niftyhonyaku.py
# 4/11/2015 jichi
# See: https://github.com/alexbft/chiitrans/blob/master/ChiitransLite/www/js/translators.coffee

if __name__ == '__main__':
  import sys
  sys.path.append('..')

import json
import requests
from sakurakit import skstr
from sakurakit.skdebug import dwarn, derror
from sakurakit.sknetdef import GZIP_HEADERS
import niftydef

session = requests # global session

HONYAKU_API = "http://honyaku-result.nifty.com/LUCNIFTY/text/text.php"

# Example:
# Sent form data:
# - XMODE: 0
# - txtDirection: jazh
# - TLANG: zh
# - SSRC: こんにちは
# - SLANG: ja
# Response is a large XHTML document that includes the TSRC text area:
# <textarea name="TSRC" id="txtTransArea2">你好</textarea>

HONYAKU_TEXT_START = 'id="txtTransArea2">'
HONYAKU_TEXT_STOP = '</textarea>'
def translate(text, to='en', fr='ja'):
  """Return translated text, which is NOT in unicode format
  @param  text  unicode not None
  @param* fr  unicode not None, must be valid language code
  @param* to  unicode not None, must be valid language code
  @return  unicode or None

  Returned text is not decoded, as its encoding can be guessed.
  """
  try:
    JSONP_CALLBACK = 'ret'
    r = session.post(HONYAKU_API,
      headers=GZIP_HEADERS,
      data={
        'SSRC': text,
        'SLANG': niftydef.nifty_lang(fr),
        'TLANG': niftydef.nifty_lang(to),
        #'txtDirection': fr + to, # not needed
        #'XMODE': 0, # not needed
      },
    )

    ret = r.content
    # Example: ret('', {"translatedText":"If you can be calm, true-kun or 210 Yen."}, 200, null, null);
    if r.ok:
      ret = skstr.findbetween(ret, HONYAKU_TEXT_START, HONYAKU_TEXT_STOP)
      ret = ret.decode('utf8', errors='ignore')
      ret = skstr.unescapehtml(ret)
      return ret

  #except socket.error, e:
  #  dwarn("socket error", e.args)
  except requests.ConnectionError, e:
    dwarn("connection error", e.args)
  except requests.HTTPError, e:
    dwarn("http error", e.args)
  #except KeyError, e:
  #  dwarn("invalid response header", e.args)
  except Exception, e:
    derror(e)

  dwarn("failed")

  try: dwarn(r.url)
  except: pass

if __name__ == '__main__':
  def test():
    global session

    #s = u"オープニングやエンディングのアニメーションは単純に主人公を入れ替えた程度の物ではなく、タイトルロゴはもちろん金時や定春の行動や表情、登場する道具（万事屋の面々が乗る車のデザインなど）やクレジット文字など、細部に渡って変更がなされた。更に、坂田金時が『銀魂'』を最終回に追い込み新しいアニメ『まんたま』を始めようとした時にはエンディングや提供表示の煽りコメントが最終回を思わせる演出となり、『まんたま』でも専用のタイトルロゴとオープニングアニメーション（スタッフクレジット付き）が新造され、偽物の提供クレジットまで表示されるなど随所に至るまで徹底的な演出が行われた。また、テレビ欄では金魂篇終了回は『金魂'』最終回として、その翌週は新番組「銀魂'」として案内された。"
    #s = u"オープニングやエンディングのアニメーションは単純に主人公を入れ替えた程度の物ではなく"
    #s = "test"
    #s = u"悠真くんを攻略すれば２１０円か。"
    s = u"悠真く『んを攻略すれば』２１０円か。"
    fr = "ja"
    to = "zht"

    #s = u"What are you doing?"
    #fr = "en"

    from sakurakit.skprof import SkProfiler

    #from qtrequests import qtrequests
    #from PySide.QtNetwork import QNetworkAccessManager
    #session = qtrequests.Session(QNetworkAccessManager())
    #with SkProfiler():
    #  for i in range(10):
    #    t = translate(s, to=to, fr=fr)
    #print t

    session = requests.Session()
    with SkProfiler():
      for i in range(1):
        t = translate(s, to=to, fr=fr)
    print t
    print type(t)

  test()

  #from PySide.QtCore import QCoreApplication, QTimer
  #app = QCoreApplication(sys.argv)
  #QTimer.singleShot(0, test)
  #app.exec_()

# EOF
