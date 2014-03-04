# coding: utf8
# powertrans_online.py
# 12/27/2012 jichi
#
# See: http://www.lec.com/translate-demos.asp

if __name__ == '__main__':
  import sys
  sys.path.append('..')

import requests
from sakurakit.skdebug import dwarn, derror
from sakurakit.sknetio import GZIP_HEADERS
#from sakurakit.skstr import escapehtml

import requests
LEC_API = "http://www.lec.com/translate-demos.asp"

def _make_post(text, to, fr):
  """
  @param  text  unicode
  @param  to  unicode
  @param  fr  unicode
  @return  dict
  """
  # See source code: http://www.lec.com/translate-demos.asp
  ret = {
    'DoTransText': 'go',
    #'SourceText': escapehtml(text), # FIXME: escapehtml cannot fix the illegal characters
    'SourceText': text,
    'selectTargetLang': to[:2],
    'selectSourceLang': fr[:2],
  }
  if to == 'zhs':
    ret['simplifiedChinese'] = 0
  return ret

# Example:
# <textarea ReadOnly class="inputtext" cols="30" id="TranslationText" name="TranslationText" rows="6"  style="width: 323px""wrap="soft">
# Teclear su texto aquí ><
# </textarea>
def _parse(text): # html -> html
  """
  @param  text  unicode
  @return  unicode
  """
  pos = text.find(' id="TranslationText" ')
  if pos > 0:
    text = text[pos+1:]
    pos = text.find('>')
    if pos > 0:
      text = text[pos+1:]
      pos = text.find("</textarea>")
      if pos > 0:
        text = text[:pos]
        return text.strip()
  dwarn("failed to parse text")
  return ""

def translate(text, to='en', fr='ja'):
  """Return translated text, which is NOT in unicode format
  @param  text  unicode not None
  @param  fr  unicode not None, must be valid language code
  @param  to  unicode not None, must be valid language code
  @return  unicode or None
  """
  try:
    r = requests.post(LEC_API,
        headers=GZIP_HEADERS,
        data=_make_post(text, to, fr))

    #print r.headers['Content-Type']
    ret = r.content

    # return error message if not r.ok
    # example response: {"t":[{"text":"hello"}]}
    if r.ok and len(ret) > 100:
      ret = _parse(ret)
    else:
      dwarn("return content too short")
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

  return ""

if __name__ == '__main__':
  #t = u"あのね  すもももももももものうち><" # Fixme: illegal html characters does not work
  t = u"あのね  すもももももももものうち"
  print translate(t)

  #print translate(t, 'ko')

  #print translate(t, 'zh')
  #print translate(t, 'zhs')

# EOF

#import urllib, urllib2
#url = createUrl(text, to=INFOSEEK_LCODE[to], fr=INFOSEEK_LCODE[fr])
#try:
#  # See: http://stackoverflow.com/questions/3465704/python-urllib2-urlerror-http-status-code
#  response = urllib2.urlopen(url)
#  ret = response.read()
#except urllib2.HTTPError, e:
#  dwarn("http error code =", e.code)
#  return ""
#except urllib2.URLError, e:
#  dwarn("url error args =", e.args)
#  return ""
#finally:
#  try: response.close()
#  except NameError: pass
