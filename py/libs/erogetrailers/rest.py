# coding: utf8
# rest.py
# 8/12/2013 jichi
#
# See: http://ketsuage.seesaa.net/article/263754550.html
# Example: http://erogetrailers.com/api?md=search_game&sw=%E3%83%AC%E3%83%9F%E3%83%8B%E3%82%BB%E3%83%B3%E3%82%B9&gameid=7998
# {
#  "searchWord":"レミニセンス",
#  "searchMode":"search",
#  "totalItems":2,
#  "items": [{
#   "id":7998,
#   "title":"レミニセンス ",
#   "romanTitle":"Reminiscence ",
#   "ecchi":true,
#   "brand":"てぃ～ぐる",
#   "platform":"PC",
#   "releaseDay":"2013年5月31日",
#   "releaseDayNumber":20130531,
#   "totalVideos":3,
#   "hp":"http://www.tigresoft.com/reminiscence/index.html",
#   "holyseal":"10548",
#   "erogamescape":"15986",
#   "amazon":"B00AT6K7OE",
#   "getchu":"718587",
#   "dmm":"",               # DL
#   "dmm2":"1781tig001",    # 通販
#   "gyutto":""
#  }

__all__ = ['RestApi']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import json, urllib2
from sakurakit.skdebug import dprint, dwarn
from sakurakit.skstr import unescapehtml

_PATCHES = { # {long id:kw}
  2220:  {'brand': u"CORE-DUSK"},   # メサイア (MESSIAH)
  2309:  {'erogamescape': '6102'},  # カスタム隷奴III
  4143:  {'title': u"Blaze of Destiny II The bginning of the fate"},
  9952:  {'erogamescape': '17943'}, # 学☆王 -THE ROYAL SEVEN STARS- +METEOR(P
  9981:  {'brand' :u"美蕾"},        # 星の王女
  #10489: {'dmm2': '543ka0066'},     # プレスタ！ ～Precious☆Star'sフェスティバル～
  10839: {'brand':u"Mink"},         # 夜勤病棟 復刻版+
}

# API is stateless
# Make this class so that _fetch could be overridden
class RestApi(object):
  URL = 'http://erogetrailers.com/api'

  EROGAMESCAPE_TYPE = 'erogamescape' # にするとエロゲー批評空間のゲームIDで検索
  HOLYSEAL_TYPE = 'holyseal' # にすると聖封のゲームIDで検索
  EROGETRAILERS_TYPE = 'erogetrailers' # にするとエロトレのゲームIDで検索

  #ERROR_MESSAGE = "\nFatal error:"

  def __init__(self):
    self.debug = False
    self.encoding = 'utf8'

  def query(self, text, type=None):
    """
    @param  text  unicode
    @param  type  str
    @return  list or None
    """
    try: return self._apply(sw=text, pg=type)
    except Exception, e: dwarn(e)

  def _parse(self, fp):
    """
    @param  fp  file pointer
    @return  list or None
    @raise
    """
    return self._patch(json.load(fp)['items'])

  def _patch(self, items):
    """
    @param  items  {kw}
    @return  {kw}
    @raise
    """
    for item in items:
      if item['romanTitle'] == '::inedited:: ':
        item['romanTitle'] = ''
      f = _PATCHES.get(item['id'])
      if f:
        for k,v in f.iteritems():
          item[k] = v

      for k in 'title', 'romanTitle', 'brand':
        t = item[k]
        if t:
          item[k] = unescapehtml(t).rstrip() # remove right most space
    return items

  def _apply(self, **params):
    """
    @return  list or None
    @raise
    """
    req = self._makereq(**params)
    r = self._fetch(**req)
    return self._parse(r)

  def _fetch(self, url):
    """
    @param  url  str
    @return  file object
    @raise
    """
    req = urllib2.Request(url)
    handler = urllib2.HTTPHandler(debuglevel=self.debug)
    opener = urllib2.build_opener(handler)
    return opener.open(req)

  def _makereq(self, **kw):
    """
    @param  kw
    @return  kw
    """
    return {'url':self._makeurl(**kw)}

  def _makeurl(self, **params):
    """
    @param  params  request params
    @return  str

    See: http://ketsuage.seesaa.net/article/263754550.html
    """
    params['md'] = 'search_game'

    # paramsのハッシュを展開
    request = ["%s=%s" % (k, urllib2.quote(self._encodeparam(v)))
        for k,v in params.iteritems()]

    ret = self.URL + "?" + "&".join(request)
    if self.debug:
      dprint(ret)
    return ret

  def _encodeparam(self, v):
    """
    @param  v  any
    @return  str
    """
    if isinstance(v, str):
      return v
    elif isinstance(v, unicode):
      return v.encode(self.encoding, errors='ignore')
    elif v is None:
      return ''
    else:
      return str(v) # May throw

if __name__ == '__main__':
  api = RestApi()
  t = u"レミニセンス"
  t = u"diaborosu"
  t = "2309" # カスタム隷奴III
  t = '415' # ::inedited::
  q = api.query(t, type=api.EROGETRAILERS_TYPE)
  print q

  #t = 9610
  #q = api.query(t, type=api.EROGETRAILERS_TYPE)
  #for it in q:
  #  print it['title']

# EOF
