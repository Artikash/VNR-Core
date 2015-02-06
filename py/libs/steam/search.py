# coding: utf8
# search.py
# 2/6/2015 jichi

__all__ = 'SearchApi',

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import re
from sakurakit import sknetio
#from sakurakit.skdebug import dwarn
from sakurakit.skstr import unescapehtml

class SearchApi(object):
  # Example: http://store.steampowered.com/search?term=Hyperdimension&sort_by=_ASC&category1=998
  API = "http://store.steampowered.com/search" #?term=Hyperdimension&sort_by=_ASC&category1=998
  ENCODING = 'utf8'

  session = None # requests.Session or None

  CATEGORY_GAME = 998
  SORT_ASC = '_ASC'

  def _makereq(self, text, sort, category=CATEGORY_GAME):
    """
    @param  text  str
    @param  sort  str
    @param  category  str
    @return  kw
    """
    return {'term':text, 'sort_by':sort, 'category1':category}

  def _fetch(self, **params):
    """
    @param  params  kw
    @return  str
    """
    return sknetio.getdata(self.API, gzip=True, params=params, session=self.session) #, cookies=self.COOKIES)

  def query(self, text, sort=SORT_ASC, category=CATEGORY_GAME):
    """
    @param  id  str or int  softId
    @param  sort  str
    @param  category  int
    @yield  {kw} or None
    """
    req = self._makereq(text, sort=sort, category=category)
    h = self._fetch(**req)
    if h:
      h = h.decode(self.ENCODING, errors='ignore')
      if h:
        return self._iterparse(h)

  # Example: TODO
  # http://store.steampowered.com/search?term=Birth&sort_by=_ASC&category1=998
  _rx_parse = re.compile(
    r'/soft.phtml\?id=([0-9]+?)" class="blueb">([^<]+?)</A>'
    r'.*?'
    r'<!--PRICE-->'
  , re.IGNORECASE|re.DOTALL)
  _rx_media = re.compile(ur'メディア：([^<]+?)<!--MEDIA-->')
  _rx_date = re.compile(ur'発売日：([0-9/]+)<!--発売日-->')
  _rx_price = re.compile(ur'定価：\s*￥([0-9,]+)')
  _rx_brand = re.compile(ur'ブランド名：(.*?)<!--BRAND-->')
  _rx_brand2 = re.compile(r'>([^<]+)<')
  def _iterparse(self, h):
    """
    @param  h  unicode
    @yield  {kw}
    """
    for m in self._rx_parse.finditer(h):
      key = m.group(1)
      title = m.group(2)
      if key and title:
        url = "http://getchu.com/soft.phtml?id=%s" % key
        img = '/brandnew/%s/c%spackage' % (key, key)
        img = self.HOST + img + '.jpg' if img in h else ''

        item = {
          'id': key,
          'url': url,
          'img': img,
          'title': unescapehtml(title),
        }

        hh = m.group()
        mm = self._rx_media.search(hh)
        item['media'] = unescapehtml(mm.group(1)).strip() if mm else '' # strip

        mm = self._rx_date.search(hh)
        item['date'] = mm.group(1) or ''

        mm = self._rx_price.search(hh)
        try: item['price'] = int(mm.group(1).replace(',', ''))
        except (KeyError, ValueError, AttributeError): item['price'] = 0

        mm = self._rx_brand.search(hh)
        brand = mm.group(1) if mm else ''
        if brand:
          mm = self._rx_brand2.search(brand)
          if mm:
            brand = mm.group(1)
        item['brand'] = unescapehtml(brand).strip() if brand else '' # strip
        yield item

if __name__ == '__main__':
  api = SearchApi()
  t = "birth"
  for it in api.query(t):
    print '-' * 10
    for k,v in it.iteritems():
      print k, ':', v

# EOF
