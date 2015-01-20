# coding: utf8
# game.py
# 10/20/2013 jichi

__all__ = 'GameApi',

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import re
from sakurakit import sknetio
from sakurakit.skcontainer import uniquelist
from sakurakit.skstr import unescapehtml
#from sakurakit.skdebug import dwarn

class GameApi(object):
  QUERY_HOST = "http://www.freem.ne.jp"
  QUERY_PATH = "/win/game/%s"

  API = QUERY_HOST + QUERY_PATH
  ENCODING = 'utf8'

  session = None # requests.Session or None

  def _makereq(self, id):
    """
    @param  kw
    @return  kw
    """
    return {'url':self._makeurl(id)}

  def _makeurl(self, id):
    """
    @param  id  int
    @return  str
    """
    return self.API % id

  def _fetch(self, url):
    """
    @param  url  str
    @return  str
    """
    return sknetio.getdata(url, gzip=True, session=self.session) #, cookies=self.COOKIES)

  def query(self, id):
    """
    @param  id  str or int  softId
    @return  {kw} or None
    """
    url = self._makeurl(id)
    h = self._fetch(url)
    if h:
      h = h.decode(self.ENCODING, errors='ignore')
      if h:
        ret = self._parse(h)
        if ret:
          ret['id'] = long(id)
          ret['url'] = "http://freem.ne.jp/win/game/%s" % id # strip off www.

          img = 'http://pic.freem.ne.jp/win/%s.jpg' % id
          ret['img'] = img if img in h else ''

          ret['sampleImages'] = list(self._iterparsesampleimages(h, id)) # [unicode]
          return ret

  def _parse(self, h):
    """
    @param  h  unicode  html
    @return  {kw}
    """
    title, slogan = self._parsemetadesc(h)
    if title:
      title = self._fixtitle(title)
      otome = u'ＢＬゲーム' in h
      bl = u'女性向' in h
      ecchi = u'全年齢' in h
      return {
        'title': title, # unicode
        'slogan': slogan, # unicode or None
        'otome': otome or bl, # bool
        'ecchi': ecchi, # bool
        'brand': self._parsebrand(h), # unicode or None
        'date': self._parsedate(h),  # str or None, such as 2013-10-25
        'filesize': self._parsesize(h), # int
        'description': self._parsedesc(h), # unicode or None
        'videos': uniquelist(self._iterparsevideos(h)),   # [kw]
      }

  # Example: RPGを初めて遊ぶ人のためのRPG ver1.32
  _re_fixtitle = re.compile(' ver[0-9. ]+$')
  def _fixtitle(self, t):
    """
    @param  t  unicode
    @return  unicode
    """
    return self._re_fixtitle.sub('', t)

  # Example: <meta name="description" content="「赤い森の魔女」：樵の少年と魔女のお話" />
  _re_meta = re.compile(ur'<meta name="description" content="「([^」]+)」(?:：([^"]+))?"')
  def _parsemetadesc(self, h):
    """
    @param  h  unicode  html
    @return  (unicode title, unicode slogan)
    """
    title = slogan = None
    m = self._re_meta.search(h)
    if m:
      title = m.group(1)
      slogan = m.group(2)
      if title:
        title = unescapehtml(title)
        if slogan:
          slogan = unescapehtml(slogan)
    return title, slogan

  # Example: <p><a href="/brand/1666">mint wings</a></p>
  _re_brand = re.compile(r'href="/brand/\d+">([^<]+)<')
  def _parsebrand(self, h):
    """
    @param  h  unicode  html
    @return  unicode or None
    """
    m = self._re_brand.search(h)
    if m:
      return unescapehtml(m.group(1))

  # Example: ■登録日<br />2015-01-11<br />
  _re_date = re.compile(ur'■登録日<br />([0-9-]+)<')
  def _parsedate(self, h):
    """
    @param  h  unicode  html
    @return  unicode or None
    """
    m = self._re_date.search(h)
    if m:
      return m.group(1)

  # Example:
  # ■容量<br />
  # 16,121 KByte<br />
  _re_size = re.compile(ur'■容量<br />\n*\r*\s*([0-9,]*) KByte')
  def _parsesize(self, h):
    """
    @param  h  unicode  html
    @return  long not None
    """
    m = self._re_size.search(h)
    if m:
      try: return 1024 * long(m.group(1).replace(',', ''))
      except: pass
    return 0

  # Example: http://pic.freem.ne.jp/win/123_2.jpg
  def _iterparsesampleimages(self, h, id):
    """
    @param  h  unicode  html
    @param  id  long
    @yield  unicode
    """
    x = re.compile(r"http://pic.freem.ne.jp/win/%s_\d+.jpg" % id)
    for m in x.finditer(h):
      yield m.group()

  # Example: https://www.youtube.com/watch?v=-Xsa47nj8uk
  _re_youtube = re.compile(r'youtube.com/watch\?v=([0-9a-zA-Z_-]+)')
  def _iterparsevideos(self, h): # the method apply to all case
    """
    @param  h  unicode  html
    @yield  unicode
    """
    if 'youtube.com' in h:
      for m in self._re_youtube.finditer(h):
        yield m.group(1)

  # Example:
  # <!-- ■ゲーム説明スペース開始 -->
  # <div id="gameExplain">
  # </div>
  # <!-- //□ゲーム説明スペース終了 -->
  _re_desc = re.compile(
    ur'<!-- ■ゲーム説明スペース開始 -->'
    r'(.*?)'
    ur'<!-- //□ゲーム説明スペース終了 -->'
  , re.DOTALL)
  def _parsedesc(self, h):
    """
    @param  h  unicode  html
    @param  id  long
    @yield  unicode or None
    """
    m = self._re_desc.search(h)
    if m:
      return m.group(1)

if __name__ == '__main__':
  api = GameApi()
  k = 8329 # http://www.freem.ne.jp/win/game/8329
  k = 3055 # http://www.freem.ne.jp/win/game/3055
  k = 7190 # http://www.freem.ne.jp/win/game/7190
  # Youtube Video
  print '-' * 10
  q = api.query(k)
  for k,v in q.iteritems():
    print k, ':', v

# EOF
