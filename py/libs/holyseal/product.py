# coding: utf8
# product.py
# 4/9/2014 jichi

__all__ = ['ProductApi']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import re
from sakurakit import sknetio, skstr
from sakurakit.skdebug import dwarn

class ProductApi(object):

  API = "http://holyseal.net/cgi-bin/mlistview.cgi?prdcode=%s"
  ENCODING = 'sjis'

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
    return sknetio.getdata(url, gzip=True)

  def query(self, id):
    """
    @param  id  str or int  prdcode
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
          ret['url'] = url
          return ret

  def _parse(self, h):
    """
    @param  h  unicode  html
    @return  {kw}
    """
    return {
      'title': self._parsetitle(h),     # unicode
      'banner': self._parsebanner(h),   # str url or None
      'ecchi': self._parseecchi(h),     # bool
      'date': self._parsedate(h),       # str or None  such as 2013/08/23
      'genre': self._parsegenre(h),     # unicode or None  slogan
    }

  # u"15 歳以上" or u"18 歳以上"
  def _parseecchi(self, h):
    """
    @param  h  unicode  html
    @return  bool
    """
    return u"15 歳以上" not in h

  # <title>[Holyseal ～聖封～] ミラー／転載 ≫ CUBE ≫ your diary</title>
  _rx_title = re.compile(ur'≫([^≫<]*?)</title>')
  def _parsetitle(self, h):
    """
    @param  h  unicode  html
    @return  unicode or None
    """
    m = self._rx_title.search(h)
    if m:
      return skstr.unescapehtml(m.group(1)).strip() # there is a space in the beginning

  # The height is always 550
  # <p align="center"><img src="http://www.cuffs-cube.jp/products/yourdiary_h/download/banner/bn_600x160_kanade.jpg" width="550" height="146" border="0" alt=""></p>
  _rx_banner = re.compile(r'<p align="center"><img src="(.*?)" width="550"')
  def _parsebanner(self, h):
    """
    @param  h  unicode  html
    @return  unicode or None
    """
    m = self._rx_banner.search(h)
    if m:
      return skstr.unescapehtml(m.group(1))

  # Example:
  # <tr class="minfo">
  #  <th class="idx" width="55">発売日</th>
  #  <td colspan="2" width="495">2013/08/23</td>
  # </tr>
  def __makeinforx(key):
    pat = r'\s*'.join((
      r'<tr class="minfo">',
      r'<th [^>]*>%s</th>' % key,
      r'<td [^>]*>(.*?)</td>',
    ))
    return re.compile(pat, re.DOTALL)

  _rx_info_genre = __makeinforx(u"ジャンル")
  def _parsegenre(self, h):
    """
    @param  h  unicode  html
    @return  unicode or None
    """
    m = self._rx_info_genre.search(h)
    if m:
      return skstr.unescapehtml(m.group(1))

  _rx_info_date = __makeinforx(u"発売日")
  def _parsedate(self, h):
    """
    @param  h  unicode  html
    @return  unicode or None
    """
    m = self._rx_info_date.search(h)
    if m:
      return skstr.unescapehtml(m.group(1))

if __name__ == '__main__':
  api = ProductApi()
  k = 9550 # http://holyseal.net/cgi-bin/mlistview.cgi?prdcode=9550
  q = api.query(k)
  print q['title']
  print q['banner']
  print q['ecchi']
  print q['date']
  print q['genre']

# EOF
