# coding: utf8
# iciba.py
# 8/7/2014 jichi
# See: jp.iciba.com
# Only en-ja-zhs are supported.

if __name__ == '__main__':
  import sys
  sys.path.append('..')

import json
import requests
from sakurakit.skdebug import dwarn, derror

# Either of the two are OK.
# But the difault type are different
ICIBA_API = "http://jp.iciba.com/api.php"
#ICIBA_API = "http://fy.iciba.com/api.php"

# Coffee source code:
#  _query = ->
#    return false  if jQuery.trim(jQuery("#inputC").val()).length is 0
#    jQuery("#copy-botton").css "display", "none"
#    jQuery.ajax
#      type: "post"
#      dataType: "json"
#      url: "api.php"
#      data: "q=" + encodeURIComponent(jQuery(".inputC").val()) + "&type=" + jQuery.mod
#      success: (msg) ->
#        if msg.error is 0
#          unless msg.outstr is "0"
#            jQuery(".tip").css "display", "block"
#            jQuery("#resultC").attr "class", "resultC"
#            jQuery(".tip").html "提示：“" + msg.outstr + "”以后的内容没有被翻译，单次翻译最多5000字"
#            jQuery("#baiDu").css "display", "block"
#
#          #setcopy(msg.retcopy);
#          else if msg.outstr is "0"
#            jQuery(".tip").css "display", "none"
#            jQuery("#resultC").attr "class", "resultC resultC2"
#            jQuery("#baiDu").css "display", "block"
#
#          #setcopy(msg.retcopy);
#          jQuery(".h2 h2 span").html msg.type
#          jQuery(".resultIndex").css "display", "none"
#          jQuery("#yj").css "display", "none"
#          jQuery(".resultM").css "display", "block"
#          jQuery("#resultC").html msg.ret
#          jQuery("#clean").css "display", "block"
#        else
#          alert "系统出错"
#        return
#
# Request: q=%E4%BD%A0%E5%A5%BD&type=zh-jp
# Response: {"error":0,"outstr":0,"from":"jp","to":"zh","type":"\u65e5 -> \u4e2d","ret":"\u4f60\u597d\u3002<br \/><br \/>","retcopy":"\u4f60\u597d\u3002"}

_LANG = {
  'en': 'en',
  'ja': 'jp',
  'zhs': 'zh',
  'zht': 'zh',
}
def _lang(lang): return _LANG.get(lang) or 'en' # str -> str

def translate(text, to='zhs', fr='ja'):
  """Return translated text, which is NOT in unicode format
  @param  text  unicode not None
  @param  fr  unicode not None, must be valid language code
  @param  to  unicode not None, must be valid language code
  @return  unicode or None
  """
  try:
    api = ICIBA_API
    r = requests.post(api, data={
      'q': text,
      'type': '-'.join((
        _lang(fr),
        _lang(to),
      ))
    })

    ret = r.content
    if r.ok and len(ret) > 20 and ret[0] == '{' and ret[-1] == '}':
      #ret = ret.decode('utf8')
      js = json.loads(ret)
      return js['retcopy']

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
  #t = translate(u"こん\nにちは！", to='zhs', fr='ja')
  #t = translate(u"你好！", to='zhs', fr='ja')
  t = translate(u"こん\nにちは！", to='en', fr='ja')
  print t

# EOF
