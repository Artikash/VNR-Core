# coding: utf8
# baidutts.py
# 10/7/2014 jichi
#
# Example: http://tts.baidu.com/text2audio?lan=jp&ie=UTF-8&text=hello

import urllib
import baidudef

API = "http://tts.baidu.com/text2audio"

def url(text, language, encoding='UTF-8'):
  """
  @param  text  unicode  not None
  @param  language  str  not None
  @param* encoding  utf8 or UTF-8 or None
  @return  unicode or str not None
  """
  if language:
    if isinstance(text, unicode):
      text = urllib.quote(text.encode(encoding, errors='ignore'))
    if text:
      language = baidudef.bdlang(language)
      return API + "?ie=%s&tl=%s&q=%s" % (encoding, language, text)
  return ''

if __name__ == '__main__':
  print url("hello", 'ja')
  print url(u"こんにちは", 'ja')

# EOF
