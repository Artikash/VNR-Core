# coding: utf8
# search.py
# 8/4/2013 jichi

__all__ = ['SearchApi']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import re
from sakurakit import sknetio, skstr
from sakurakit.skdebug import dwarn

class SearchApi(object):
  # Example: http://gyutto.com/search/search_list.php?category_id=6&set_category_flag=1&search_keyword=%83%89%83%93%83X
  HOST = 'http://gyutto.com'
  API = HOST + '/search/search_list.php'

  ENCODING = 'sjis'
  COOKIES = {'adult_check_flag':'1'}

  PC_GAME_CATEGORY_ID = 6
  DOUJIN_GAME_CATEGORY_ID = 10

  GAME_CATEGORIES = PC_GAME_CATEGORY_ID, DOUJIN_GAME_CATEGORY_ID

  def _makereq(self, text, category_id):
    """
    @param  text  str
    @param  category_id int
    @return  kw
    """
    text = text.encode(self.ENCODING, errors='ignore')
    ret = {'search_keyword':text}
    if category_id:
      ret['category_id'] = category_id
      ret['set_category_flag'] = 1
    return ret

  def _fetch(self, **params):
    """
    @param  params  kw
    @return  str
    """
    return sknetio.getdata(self.API, gzip=True, params=params, cookies=self.COOKIES)

  def query(self, text, category_id=0):
    """
    @param  id  str or int  softId
    @param* category_id  int
    @yield  {kw} or None
    """
    req = self._makereq(text, category_id=category_id)
    h = self._fetch(**req)
    if h:
      h = h.decode(self.ENCODING, errors='ignore')
      if h:
        return self._iterparse(h)

  # Example:
  # http://gyutto.com/search/search_list.php?category_id=6&sub_category_id=&set_category_flag=1&mode=search&sub_category_id=11&search_item_search_id=122&search_keyword=%83%89%83%93%83X&search.x=0&search.y=0
  #
  # <li>
  # <dl class="ItemBox">
  # <dd class="DefiPhotoName">
  # <a href="http://gyutto.com/i/item125070"><span><img src="/data/item_img/1250/125070/125070_p_s2.jpg" width="100" border="0" alt="ランス9 ヘルマン革命" /></span>
  # <span class="Alert"></span>ランス9 ヘルマン革命</a></dd>
  # <dd class="DefiAuthor">[&nbsp;<a href="http://gyutto.com/search/search_list.php?mode=search&brand_id=381&category_id=6&set_category_flag=1">アリスソフト</a>&nbsp;]</dd>
  # <dd class="DefiPrice">7,344円</dd>
  # <dd class="DefiPoint">最大10%還元</dd>
  # </dl>
  # </li>
  _rx_parse = re.compile(
    r'/soft.phtml\?id=([0-9]+?)" class="blueb">([^<]+?)</A>'
    r'.*?'
    r'<!--PRICE-->'
  , re.IGNORECASE|re.DOTALL)
  _rx_id = re.compile(r'http://gyutto.com/i/item(\d+)')
  _rx_img = re.compile(r'<img src="(/data/item_img/\d+/\d+/\d+)_p_s2.jpg"')
  _rx_title = re.compile(r'<span class="Alert"></span>([^<]+?)</a></dd>')
  _rx_brand = re.compile(r'>([^<]+?)</a>&nbsp;\]</dd>')
  _rx_price = re.compile(ur'<dd class="DefiPrice">([0-9,]+)?円</dd>')
  def _iterparse(self, h):
    """
    @param  h  unicode
    @yield  {kw}
    """
    START = '<dl class="ItemBox">'
    STOP = '</dl>'
    stop = 0
    while True:
      start = h.find(START, stop)
      if start == -1:
        break
      stop = h.find(STOP, start)
      if stop == -1:
        break

      hh = h[start:stop]

      try: key = long(self._rx_id.search(hh).group(1))
      except: break

      item = {
        'id': key,
        'url': "http://gyutto.com/i/item%s" % key,
      }

      try: item['title'] = self._rx_title.search(hh).group(1)
      except: break

      try: item['brand'] = self._rx_brand.search(hh).group(1)
      except: pass

      img = ''
      m = self._rx_img.search(hh)
      if m:
        item['image'] = self.HOST + m.group(1) + '.jpg'

      m = self._rx_price.search(hh)
      try: item['price'] = int(m.group(1).replace(',', ''))
      except: pass

      yield item

if __name__ == '__main__':
  api = SearchApi()
  t = u"ランス"
  for it in api.query(t): # reverse order
    #print '-' * 10
    for k,v in it.iteritems():
      print k, ':', v

# EOF
