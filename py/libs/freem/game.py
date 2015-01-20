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
from sakurakit.skstr import multireplacer, unescapehtml
#from sakurakit.skdebug import dwarn

class GameApi(object):
  HOST = "http://www.freem.ne.jp"
  QUERY_PATH = "/win/game/%s"

  API = HOST + QUERY_PATH
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
          img = "brandnew/%s/c%spackage.jpg" % (id, id)    # str, example: www.getchu.com/brandnew/756396/c756396package.jpg
          ret['img'] = self.HOST + img if img in h else '' # str
          #self._patch(ret, id)
          return ret

  def _parse(self, h):
    """
    @param  h  unicode  html
    @return  {kw}
    """
    meta = self._parsemetadesc(h)
    if meta:
      title = meta.get(u"タイトル")
      if not title:
        title = self._parsetitle(h)
      if title:
        title = self._fixtitle(title)
        try: price = int(self._parseprice(h))
        except (ValueError, TypeError): price = 0

        # Use fillter.bool to remove empty scenario
        try: writers = filter(bool, meta[u"シナリオ"].replace(u"、他", '').split(u'、'))
        except Exception: writers = []

        artists = []
        sdartists = []
        t = meta.get(u"原画")
        if t:
          sd1 = u"（SD原画）"
          sd2 = u"（アバター）"
          for it in t.split(u'、'):
            if it and it != u"他":
              if it.endswith(sd1):
                it = it.replace(sd1, '')
                sdartists.append(it)
              elif it.endswith(sd2):
                it = it.replace(sd2, '')
                sdartists.append(it)
              else:
                artists.append(it)

        # See: http://www.getchu.com/pc/genre.html
        subgenres = uniquelist(self._iterparsesubgenres(h))
        categories = uniquelist(self._iterparsecateories(h))
        return {
          'title': title, # unicode or None
          'writers': writers, # [unicode]
          'artists': artists, # [unicode]
          'sdartists': sdartists, # [unicode]
          'musicians': list(self._iterparsemusicians(h)), # [unicode]
          'brand': self._fixbrand(meta.get(u"ブランド") or meta.get(u"サークル")), # unicode or None
          'genre': self._parsegenre(h), # unicode or None
          'subgenres': subgenres, # [unicode]
          'categories': categories, # [unicode]
          'otome': u"乙女ゲー" in categories, # bool
          'price': price,                     # int
          'date': self._parsedate(h),         # str or None, such as 2013/10/25
          #'imageCount': self._parseimagecount(h),
          'sampleImages': list(self._iterparsesampleimages(h)), # [kw]
          'descriptions': list(self._iterparsedescriptions(h)), # [unicode]
          'characterDescription': self._parsecharadesc(h), # unicode
          #'comics': list(self._iterparsecomics(h)),   # [kw]
          'banners': list(self._iterparsebanners(h)), # [kw]
          'videos': uniquelist(self._iterparsevideos(h)),   # [kw]

          # Disabled
          'characters': list(self._iterparsecharacters(h)) or list(self._iterparsecharacters2(h)), # [kw]
          #'characters': [],
        }

if __name__ == '__main__':
  api = GameApi()
  # http://www.freem.ne.jp/win/game/8329
  k = 8329
  # http://www.freem.ne.jp/win/game/3055
  k = 3055
  print '-' * 10
  q = api.query(k)
  for it in q['characters']:
    for k,v in it.iteritems():
      print k,':',v

# EOF
