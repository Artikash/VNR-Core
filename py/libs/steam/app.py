# coding: utf8
# app.py
# 2/6/2015 jichi

__all__ = 'AppApi',

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import re
from sakurakit import sknetio
from sakurakit.skstr import unescapehtml
#from sakurakit.skdebug import dwarn

class AppApi(object):
  API = "http://store.steampowered.com/app/%s"
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
    return sknetio.getdata(url, gzip=True, session=self.session)

  def query(self, id):
    """
    @param  id  str or int  appId
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
          ret['url'] = self.API % id # str

          img = "http://cdn.akamai.steamstatic.com/steam/apps/%s/header.jpg" % id
          ret['img'] = img if img in h else '' # str
          return ret

  #def _patch(self, kw, id):
  #  """
  #  @param  kw  {kw}
  #  @param  id  long
  #  """
  #  chara = kw['characters']
  #  if chara:
  #    for it in chara:
  #      it['img'] = 'http://www.getchu.com/brandnew/%s/c%schara%s.jpg' % (id, id, it['id'])

  def _parse(self, h):
    """
    @param  h  unicode  html
    @return  {kw}
    """
    title = self._parsetitle(h)
    if title:
      return {
        'title': title, # unicode or None
        #'writers': writers, # [unicode]
        #'artists': artists, # [unicode]
        #'sdartists': sdartists, # [unicode]
        #'musicians': list(self._iterparsemusicians(h)), # [unicode]
        #'brand': self._fixbrand(meta.get(u"ブランド") or meta.get(u"サークル")), # unicode or None
        #'genre': self._parsegenre(h), # unicode or None
        #'subgenres': subgenres, # [unicode]
        #'categories': categories, # [unicode]
        #'otome': u"乙女ゲー" in categories, # bool
        #'price': price,                     # int
        #'date': self._parsedate(h),         # str or None, such as 2013/10/25
        ##'imageCount': self._parseimagecount(h),
        #'sampleImages': list(self._iterparsesampleimages(h)), # [kw]
        #'descriptions': list(self._iterparsedescriptions(h)), # [unicode]
        #'characterDescription': self._parsecharadesc(h), # unicode
        ##'comics': list(self._iterparsecomics(h)),   # [kw]
        #'banners': list(self._iterparsebanners(h)), # [kw]
        #'videos': uniquelist(self._iterparsevideos(h)),   # [kw]

        ## Disabled
        #'characters': list(self._iterparsecharacters(h)) or list(self._iterparsecharacters2(h)), # [kw]
        ##'characters': [],
      }

  # Example: <div class="apphub_AppName">Hyperdimension Neptunia Re;Birth1</div>
  _re_title = re.compile('<div class="apphub_AppName">([^<]+)</div>')
  def _parsetitle(self, h):
    m = self._re_title.search(h)
    if m:
      return unescapehtml(m.group(1))

if __name__ == '__main__':
  api = AppApi()
  k = 282900
  print '-' * 10
  q = api.query(k)
  for k,v in q.iteritems():
    print k,':',v

# EOF
