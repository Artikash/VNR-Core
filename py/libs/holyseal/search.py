# coding: utf8
# search.py
# 6/18/2014 jichi

__all__ = ['SearchApi']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import re
from sakurakit import sknetio, skstr
from sakurakit.skdebug import dwarn

class SearchApi(object):

  API = "http://holyseal.net/cgi-bin/mlistview.cgi?word=%s"
  ENCODING = 'sjis'

  def _makereq(self, text):
    """
    @param  kw
    @return  kw
    """
    return {'url':self._makeurl(text)}

  def _makeurl(self, text):
    """
    @param  text  unicode
    @return  unicode
    """
    return self.API % text

  def _fetch(self, url):
    """
    @param  url  str
    @return  str
    """
    return sknetio.getdata(url, gzip=True)

  def query(self, text):
    """
    @param  text  unicode
    @return  {kw} or None
    """
    text = text.encode(self.ENCODING, errors='ignore')
    if text:
      url = self._makeurl(text)
      h = self._fetch(url)
      if h:
        h = h.decode(self.ENCODING, errors='ignore')
        if h:
          return self._iterparse(h)

  # Example:  http://holyseal.net/cgi-bin/mlistview.cgi?word=dive
  #
  # 検索結果（タイトル）</strong> [<a href="mlistview.cgi?prdcode=10890">消す</a>]</p>
  # <p class="selc"><strong>2012 年</strong></p><p class="sellst3">･<strong>Dolphin Divers</strong></p>
  # <p class="selc"><strong>2010 年</strong></p><p class="sellst1">･<a href="mlistview.cgi?prdcode=9457&amp;word=dive" target="_self"><span class="prd">BALDR SKY DiveX “DREAM WORLD”</span></a></p>
  # <p class="selc"><strong>2009 年</strong></p><p class="sellst1">･<a href="mlistview.cgi?prdcode=8797&amp;word=dive" target="_self"><span class="prd">BALDR SKY Dive2 “RECORDARE”</span></a></p>
  # <p class="sellst1">･<a href="mlistview.cgi?prdcode=7158&amp;word=dive" target="_self"><span class="prd">BALDR SKY Dive1 “LostMemory”</span></a></p>
  # </div><br><div class="fr"><p class="idx"><strong>

  _rx_first_title = re.compile(u'<p class="sellst3">･<strong>([^<]*?)</strong></p>')
  _rx_first_id = re.compile(u'prdcode=([0-9]+?)">消す</a>')
  _rx_year = re.compile(u'<strong>([0-9]{4}) 年</strong>')
  _rx_product = re.compile('prdcode=([0-9]+)[^>]*?><span class="prd">([^<]*?)</span>')
  def _iterparse(self, h):
    """
    @param  h  unicode
    @yield  {kw}
    """
    try:
      start = h.find(u"検索結果（タイトル）") # int
      stop = h.find(u"ブランド別製品リスト") # int
      if start > 0 and stop > start:
        h = h[start:stop]

        years = [] # [int year, int start]
        for m in self._rx_year.finditer(h):
          years.append((
            int(m.group(1)),
            m.start(),
          ))
        if not years:
          dwarn("cannot find release years, maybe, unknown years")


        id0 = title0 = None

        # first, yield the matched game
        m = self._rx_first_id.search(h)
        if m:
          id0 = int(m.group(1))
          if id0:
            m = self._rx_first_title.search(h)
            if m:
              title0 = skstr.unescapehtml(m.group(1))

        if id0 and title0:
          year0 = years[0][0] if years else None
          yield {
            'id': id0,
            'title': title0,
            'year': year0,
          }

          # then, parse index of years
          # iterparse and compare index against year index
          for m in self._rx_product.finditer(h):
            id = int(m.group(1))
            title = skstr.unescapehtml(m.group(2))
            year = None
            if years:
              for y,start in years:
                if start > m.start():
                  break
                year = y
            yield {
              'id': id,
              'title': title,
              'year': year
            }

    except ValueError: # raised by int()
      dwarn("failed to convert to int")

if __name__ == '__main__':
  api = SearchApi()
  k = 'dive' # http://holyseal.net/cgi-bin/mlistview.cgi?word=dive
  k = u'レミニ'
  q = api.query(k)
  if q:
    for it in q:
      print it

# EOF
