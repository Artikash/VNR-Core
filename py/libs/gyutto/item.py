# coding: utf8
# item.py
# 11/28/2013 jichi

__all__ = ['ItemApi']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import re
from sakurakit import sknetio, skstr
#from sakurakit.skcontainer import uniquelist
from sakurakit.skdebug import dwarn

class ItemApi(object):
  HOST = "http://gyutto.com"
  IMAGE_HOST = "http://image.gyutto.com"
  API = HOST + "/i/item%s"
  ENCODING = 'sjis'
  COOKIES = {'adult_check_flag':'1'} #, 'user_agent_flat':'1'}

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
    # Disable redirects for gyutto items
    return sknetio.getdata(url, gzip=True, cookies=self.COOKIES, allow_redirects=False)

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
          ret['id'] = id # str or int
          ret['url'] = url
          return ret

  def _parse(self, h):
    """
    @param  h  unicode  html
    @return  {kw}
    """
    title = self._parsetitle(h)
    if title:
      return {
        'title': title,
        'image': self._parseimage(h),
        'theme': self._parsetheme(h),
        'tags': list(self._iterparsetags(h)),
        'sampleImages': list(self._iterparsesampleimages(h)),
      }

  def __makemetarx(name):
    """
    @param  name  str
    @return  re
    """
    return re.compile(r'<meta %s content="(.*?)"' % name)

  def _parsemeta(self, rx, h):
    """
    @param  rx  re
    @param  h  unicode  html
    @return  unicode or None
    """
    m = rx.search(h)
    if m:
      return m.group(1)

  _rx_meta_keyword = __makemetarx('name="keyword"')
  def _parsemetakeyword(self, h):
    """
    @param  h  unicode  html
    @return  unicode or None
    """
    return self._parsemeta(self._rx_meta_keyword, h)

  def _parsetitle(self, h):
    """
    @param  h  unicode  html
    @return  unicode or None
    """
    t = self._parsemetakeyword(h)
    if t:
      return skstr.unescapehtml(t.partition(',')[0])
  # Examplee:
  # <dt>作品テーマ</dt>
  # <dd>演劇de恋するアドベンチャー</dd>
  # </dl>
  _rx_theme = re.compile(ur'''<dt>作品テーマ</dt>
<dd>(.+?)</dd>''')
  def _parsetheme(self, h):
    """
    @param  h  unicode  html
    @return  str  URL or None
    """
    m = self._rx_theme.search(h)
    if m:
      return skstr.unescapehtml(m.group(1))

  _rx_image = re.compile(r'/data/item_img/[0-9]+/([0-9]+)/\1\.jpg')
  def _parseimage(self, h):
    """
    @param  h  unicode  html
    @return  str  URL or None
    """
    m = self._rx_image.search(h)
    if m:
      return self.IMAGE_HOST + m.group()

  _rx_sampleimage = re.compile(r'/data/item_img/[0-9]+/[0-9]+/[0-9]+_[0-9]+.jpg')
  def _iterparsesampleimages(self, h):
    """
    @param  h  unicode  html
    @yield  str  URL
    """
    for m in self._rx_sampleimage.finditer(h):
      yield self.IMAGE_HOST + m.group()

  # Example
  # <dd><a href="http://gyutto.com/search/search_list.php?genre_id=20754&category_id=6&set_category_flag=1">メイド</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=18753&category_id=6&set_category_flag=1">恋愛</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16609&category_id=6&set_category_flag=1">貧乳・微乳</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16565&category_id=6&set_category_flag=1">巨乳・爆乳</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16604&category_id=6&set_category_flag=1">妹</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16621&category_id=6&set_category_flag=1">メイド服</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16564&category_id=6&set_category_flag=1">学園</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16561&category_id=6&set_category_flag=1">コメディ</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=15983&category_id=6&set_category_flag=1">アドベンチャー</a><br>
  _rx_tag = re.compile(r'genre_id=.*?>(.*?)</a>')
  _rx_tag_delims = re.compile(ur'[・+]')
  def _iterparsetags(self, h):
    """
    @param  h  unicode  html
    @yield  str
    """
    s = set()
    for m in self._rx_tag.finditer(h):
      t = skstr.unescapehtml(m.group(1))
      if t not in s:
        s.add(t)
        for it in self._rx_tag_delims.split(t):
          yield it

  # Example
  # <dt>ジャンル</dt>
  # <dd><a href="http://gyutto.com/search/search_list.php?genre_id=20754&category_id=6&set_category_flag=1">メイド</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=18753&category_id=6&set_category_flag=1">恋愛</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16609&category_id=6&set_category_flag=1">貧乳・微乳</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16565&category_id=6&set_category_flag=1">巨乳・爆乳</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16604&category_id=6&set_category_flag=1">妹</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16621&category_id=6&set_category_flag=1">メイド服</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16564&category_id=6&set_category_flag=1">学園</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=16561&category_id=6&set_category_flag=1">コメディ</a>、<a href="http://gyutto.com/search/search_list.php?genre_id=15983&category_id=6&set_category_flag=1">アドベンチャー</a><br>
  # </dd>
  # </dl>
  #def __maketablerx(name):
  #  return re.compile("<dl>%s</dl>.*?</dd>", re.DOTALL)

if __name__ == '__main__':
  api = ItemApi()
  k = 45242
  k = 16775

  print '-' * 10
  q = api.query(k)
  #print q['title']
  #print q['image']
  #print q['sampleImages']
  for it in q['tags']:
    print it
  print q['theme']
  #print q['videos']

# EOF
