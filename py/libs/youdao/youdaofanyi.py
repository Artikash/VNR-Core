# coding: utf8
# youdaofanyi.py
# 9/9/2013 jichi
#
# See: http://shared.ydstatic.com/fanyi/v2.3.3/scripts/fanyi.js
# See: http://idning.googlecode.com/svn/trunk/seo/porginal/youdao_porginal.py
#
# Coffee:
#   m = ->
#     l.translate
#       type: a("#customSelectVal").val()
#       i: u.utf8_decode(n.val())
#       doctype: "json"
#       xmlVersion: "1.6"
#       keyfrom: "fanyi.web"
#       ue: "UTF-8"
#       typoResult: not 0
#
#   u = (t) ->
#     __rl_event "translate_text"
#     n = "translate?smartresult=dict&smartresult=rule&smartresult=ugc&sessionFrom=" + r
#     o.ajax
#       type: "POST"
#       contentType: "application/x-www-form-urlencoded; charset=UTF-8"
#       url: n
#       data: t
#       dataType: "json"
#       success: (e) ->
#         s t, e
#       error: (t) ->
#         e("/tips").transRequestError t
#
#   t.fn.sel.defaults =
#     number: 13
#     hintList: [
#       val: "AUTO"
#       liText: "自动检测语言"
#       liClass: "normal"
#     ,
#       val: "ZH_CN2EN"
#       liText: "中文&nbsp; » &nbsp;英语"
#       liClass: "isfl topBorder1"
#     ,
#       val: "EN2ZH_CN"
#       liText: "英语&nbsp; » &nbsp;中文"
#       liClass: "isfl rightBorder topBorder"
#     ,
#       val: "ZH_CN2JA"
#       liText: "中文&nbsp; » &nbsp;日语"
#       liClass: "isfl"
#     ,
#       val: "JA2ZH_CN"
#       liText: "日语&nbsp; » &nbsp;中文"
#       liClass: "isfl rightBorder"
#     ,
#       val: "ZH_CN2KR"
#       liText: "中文&nbsp; » &nbsp;韩语"
#       liClass: "isfl"
#     ,
#       val: "KR2ZH_CN"
#       liText: "韩语&nbsp; » &nbsp;中文"
#       liClass: "isfl rightBorder"
#     ,
#       val: "ZH_CN2FR"
#       liText: "中文&nbsp; » &nbsp;法语"
#       liClass: "isfl"
#     ,
#       val: "FR2ZH_CN"
#       liText: "法语&nbsp; » &nbsp;中文"
#       liClass: "isfl rightBorder"
#     ,
#       val: "ZH_CN2RU"
#       liText: "中文&nbsp; » &nbsp;俄语"
#       liClass: "isfl"
#     ,
#       val: "RU2ZH_CN"
#       liText: "俄语&nbsp; » &nbsp;中文"
#       liClass: "isfl rightBorder"
#     ,
#       val: "ZH_CN2SP"
#       liText: "中文&nbsp; » &nbsp;西班牙语"
#       liClass: "isfl"
#     ,
#       val: "SP2ZH_CN"
#       liText: "西班牙语&nbsp; » &nbsp;中文"
#       liClass: "isfl rightBorder"
#     ]
#     containerId: "customSelectOption"
#     init: ->

if __name__ == '__main__':
  import sys
  sys.path.append('..')

import requests
#from time import time
from sakurakit.skdebug import dwarn, derror
#from sakurakit.sknetio import GZIP_HEADERS

session = requests # global session

# Example:
#FANYI_API = "http://fanyi.youdao.com/translate?smartresult=dict&smartresult=rule&smartresult=ugc&sessionFrom=null"
FANYI_API = "http://fanyi.youdao.com/translate"


# fanyi.coffee:
#   t.LANGUAGETYPES =
#     AUTO: "自动检测语言"
#     EN2ZH_CN: "英语 &raquo; 中文"
#     ZH_CN2EN: "中文 &raquo; 英语"
#     JA2ZH_CN: "日语 &raquo; 中文"
#     ZH_CN2JA: "中文 &raquo; 日语"
#     FR2ZH_CN: "法语 &raquo; 中文"
#     ZH_CN2FR: "中文 &raquo; 法语"
#     KR2ZH_CN: "韩语 &raquo; 中文"
#     ZH_CN2KR: "中文 &raquo; 韩语"
#     RU2ZH_CN: "俄语 &raquo; 中文"
#     ZH_CN2RU: "中文 &raquo; 俄语"
#     SP2ZH_CN: "西语 &raquo; 中文"
#     ZH_CN2SP: "中文 &raquo; 西语"
FANYI_TYPES = {
  'enzh': 'EN2ZH_CN',
  'zhen': 'ZH_CN2EN',
  'jazh': 'JA2ZH_CN',
  'zhja': 'ZH_CN2JA',
  'frzh': 'FR2ZH_CN',
  'zhfr': 'ZH_CN2FR',
  'kozh': 'KR2ZH_CN',
  'zhko': 'ZH_CN2KR',
  'ruzh': 'RU2ZH_CN',
  'zhru': 'ZH_CN2RU',
  'eszh': 'SP2ZH_CN',
  'zhes': 'ZH_CN2SP',
}
def _typeof(to, fr):
  """
  @param  text  unicode not None
  @param  fr  unicode not None
  @param  to  unicode not None
  @return  unicode not None
  """
  return FANYI_TYPES.get(fr[:2] + to[:2]) or 'AUTO'

_translate_result = "result="
def translate(text, to='zhs', fr='ja'):
  """Translate from/into simplified Chinese.
  @param  text  unicode not None
  @param  fr  unicode not None
  @param  to  unicode not None
  @return  unicode or None
  """
  #tok = self.__d.token
  #if tok:
  try:
    r = session.post(FANYI_API,
      #headers=GZIP_HEADERS,
      data = {
        'doctype': 'text', # or json, xml, etc
        'type': _typeof(to, fr),
        'i':  text, # utf8
      }
    )
    ret = r.content
    # Example reply:
    #   errorcode=0
    #   result=hello world
    if r.ok and len(ret) > 10:
      ret = ret.decode('utf8', errors='ignore')
      i = ret.index(_translate_result) # may throw value error
      return ret[i+len(_translate_result):].rstrip() # rstrip to remove right "\n"

  #except socket.error, e:
  #  dwarn("socket error", e.args)
  except requests.ConnectionError, e:
    dwarn("connection error", e.args)
  except requests.HTTPError, e:
    dwarn("http error", e.args)
  #except UnicodeDecodeError, e:
  #  dwarn("unicode decode error", e)
  except (ValueError, IndexError), e:
    dwarn("text format error", e)
  except Exception, e:
    derror(e)
  dwarn("failed")
  try: dwarn(r.url)
  except: pass

if __name__ == "__main__":
  #t = translate(u"Hello World!\nhello", to='zhs', fr='en')
  #t = translate(u"Hello World!\nhello", to='zhs', fr='ja')

  t = translate(u"こんにちは？", to='zhs', fr='ja')
  #t = translate(u"神楽", to='zhs', fr='ja')

  from PySide.QtGui import *
  a = QApplication(sys.argv)
  w = QLabel(t)
  w.show()
  a.exec_()

# EOF
