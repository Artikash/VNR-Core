# coding: utf8
# trans.py
# 3/17/2014 jichi
# http://sakuradite.com/topic/166
# http://translate.ru
# http://online-translator.com
# https://github.com/mishin/gists/blob/master/4translate_tmp.pl
# http://kbyte.ru/ru/Forums/Show.aspx?id=14948

if __name__ == '__main__':
  import sys
  sys.path.append('..')

import json
import requests
from sakurakit.skdebug import dwarn, derror
#from sakurakit.sknetio import GZIP_HEADERS

#RU_API = "http://translation.ru/Default.aspx/Text?prmtlang=ru"
#RU_API = "http://logrus.ru" "/services/TranslationService.asmx/GetTranslateNew"
RU_API = "http://www.translate.ru" "/services/TranslationService.asmx/GetTranslateNew"

RU_HEADERS = {'Content-Type':'application/json'}

# https://github.com/mishin/gists/blob/master/4translate_tmp.pl
# $.ajax({
#    type: "POST",
#    contentType: "application/json; charset=utf-8",
#    url: "/services/TranslationService.asmx/GetTranslateNew",
#    data: "{ dirCode:'"+dir+"', template:'"+templ+"', text:'"+text+"', lang:'en', limit:"+maxlen+",useAutoDetect:false, key:'"+key+"',  ts:'"+TS+"', tid:'"+tmpID+"',IsMobile:false}",
#    dataType: "json",
#    success: function(res){
#      SetValsAfterTr(res);
#
#    //added 28.10.13
#    RefreshAdv_inRes();
#    //-----//------
#
#    },
#    error: function (XMLHttpRequest, textStatus, errorThrown) {
#      GetErrMsg("Sorry, the service is temporarily unavailable. Please try again later.");
#    }
# });

def translate(text, to='ru', fr='ja'):
  """Return translated text, which is NOT in unicode format
  @param  text  unicode not None
  @param  fr  unicode not None, must be valid language code
  @param  to  unicode not None, must be valid language code
  @return  unicode or None

  Returned text is not decoded, as its encoding can be guessed.
  """
  try:
    r = requests.post(RU_API,
      headers=RU_HEADERS,
      data=json.dumps({
        'dirCode': fr[0] + to[0], # dir
        'template': 'General', # templ
        'text': text,
        'lang': to[:2],
        'limit': 3000, # maxlen
        'useAutoDetect': False,
        'key':  '',
        'ts': 'MainSite', # TS
        'tid': '', # tmpID,
        'IsMobile': False,
      })
    )

    #print r.headers['Content-Type']
    ret = r.content

    # return error message if not r.ok
    # example response: {"t":[{"text":"hello"}]}
    if r.ok and ret and ret[0] == '{' and ret[-1] == '}':
      # Unicode char, see: http://schneide.wordpress.com/2009/05/18/the-perils-of-u0027/
      #ret = __repl(ret[15:-4])
      ret = json.loads(ret)['result']
      return ret.replace('<br/>', '\n')

  #except socket.error, e:
  #  dwarn("socket error", e.args)
  except requests.ConnectionError, e:
    dwarn("connection error", e.args)
  except requests.HTTPError, e:
    dwarn("http error", e.args)
  except (ValueError|KeyError), e:
    dwarn("invalid response json", e.args)
  except Exception, e:
    derror(e)

  dwarn("failed")

  try: dwarn(r.url)
  except: pass

if __name__ == '__main__':
  t = translate(u"あのね  すもももももももものうち", 'en')
  print type(t), t
  t = translate(u"hello", 'ja', 'en')
  print type(t), t

  t = u"""あのね
だめなの
><"""
  print translate(t, 'en', 'ja')


# EOF
