# coding: utf8
# navertrans.py
# 1/22/2015 jichi
#
# Japanese and multilingual: http://livedoor-translate.naver.jp
# Korean only: http://translate.naver.com
#
# Example:
# API: http://translate.naver.com/translate.dic
# Post data: query=hello&srcLang=en&tarLang=ko&highlight=1&hurigana=1
#
# Only limited language pairs are supported.

if __name__ == '__main__':
  import sys
  sys.path.append('..')

import json
import requests
from sakurakit.skdebug import dwarn, derror
from sakurakit.sknetio import GZIP_HEADERS

session = requests # global session

NAVER_API = "http://translate.naver.com/translate.dic"

def translate(text, to='ko', fr='ja'):
  """Return translated text, which is NOT in unicode format
  @param  text  unicode not None
  @param  fr  unicode not None, must be valid language code
  @param  to  unicode not None, must be valid language code
  @return  unicode or None
  """
  #tok = self.__d.token
  #if tok:
  try:
    r = session.post(NAVER_API,
      #headers=HEADERS,
      data={
        'srcLang': fr[:2],
        'tarLang': to[:2],
        #'translateParams.langDetect': 'Y',
        'query': text,
        #'highlight': 1,
        #'hurigana': 1,
      }
    )

    ret = r.content
    if r.ok and len(ret) > 20:
      data = json.loads(ret)
      ret = data['resultData']
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

if __name__ == "__main__":

  def test():
    global session

    #s = u"オープニングやエンディングのアニメーションは単純に主人公を入れ替えた程度の物ではなく、タイトルロゴはもちろん金時や定春の行動や表情、登場する道具（万事屋の面々が乗る車のデザインなど）やクレジット文字など、細部に渡って変更がなされた。更に、坂田金時が『銀魂'』を最終回に追い込み新しいアニメ『まんたま』を始めようとした時にはエンディングや提供表示の煽りコメントが最終回を思わせる演出となり、『まんたま』でも専用のタイトルロゴとオープニングアニメーション（スタッフクレジット付き）が新造され、偽物の提供クレジットまで表示されるなど随所に至るまで徹底的な演出が行われた。また、テレビ欄では金魂篇終了回は『金魂'』最終回として、その翌週は新番組「銀魂'」として案内された。"
    s = u"お花の匂い"
    s = '"<html>&abcde"'
    s = u"그렇습니다"
    s = u"hello"

    fr = "ja"
    fr = "zhs"
    fr = "es"
    fr = "th"

    fr = "en"
    fr = "es"
    to = "en"


    #s = u"What are you doing?"
    #fr = "en"

    from sakurakit.skprof import SkProfiler

    #from qtrequests import qtrequests
    #from PySide.QtNetwork import QNetworkAccessManager
    #session = qtrequests.Session(QNetworkAccessManager())
    #with SkProfiler():
    #  for i in range(1):
    #    t = translate(s, to=to, fr=fr)
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
