# coding: utf8
# search.py
# 9/28/2014 jichi

__all__ = ['SearchApi']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import re
from sakurakit import sknetio
#from sakurakit.skdebug import dwarn
from sakurakit.skstr import unescapehtml, urlencode

DEFAULT_HOST = "http://www.toranoana.jp"
_QUERY_PATH = "/cgi-bin/R2/allsearch.cgi"

def resethost():
  sethost(DEFAULT_HOST)
def sethost(url):
  api = SearchApi
  api.HOST = url
  api.API = api.HOST + _QUERY_PATH

class SearchApi(object):
  HOST = DEFAULT_HOST
  API = HOST + _QUERY_PATH

  #DOMAINS = 'home', 'soft', 'comic', 'pro', 'maniax', 'girls'
  DOMAINS = 'home', 'soft', 'pro', 'maniax', 'girls'

  ENCODING = 'sjis'
  COOKIES = {'afg':'0'}

  session = None # requests.Session or None

  def _makereq(self, **kwargs):
    """
    @return  kw
    """
    return {'url':self.API, 'data':kwargs}

  def _fetch(self, url, data):
    """
    @param  url  str
    @return  str
    """
    return sknetio.postdata(url, data=data, gzip=True, session=self.session, cookies=self.COOKIES)

  def query(self, text, domain=DOMAINS[0]):
    """
    @param  tet  str
    @yield  {kw} or None
    """
    text = text.encode(self.ENCODING, errors='ignore')
    if text:
      req = self._makereq(
        search=text,
        item_kind=2103,
        bl_fg=0,
        #obj=0, # 男性向／女性向: 0=both, 1=m, 2=f
        #adl=0, # 一般／18禁: 0=both, 1=15, 2=18
      )
      h = self._fetch(**req)
      print h
      if h:
        h = h.decode(self.ENCODING, errors='ignore')
        if h:
          return self._iterparse(h)

  # Example:
  # http://www.getchu.com/php/nsearch.phtml?genre=all&search_keyword=レミニセンス
  #
  # <A HREF="../soft.phtml?id=718587" class="blueb">レミニセンス 初回限定版</A><a href="https://order.zams.biz/comike/mypage/af_wish_list.phtml?action=add&id=718587" title="お気に入りに追加"><img class="lazy" src="/common/images/space.gif" data-original="/common/images/favorite.gif" class="favorite"></a><a href="/php/nsearch.phtml?search_reference_id=718587" title="この商品の関連を開く"><img class="lazy" src="/common/images/space.gif" data-original="/common/images/relation.gif" class="relation"></a>
  # </TD></TR>
  # <TR><TD align="left" valign="top" width="100%" style="line-height:1.35;">
  # <p><span class="orangeb">[PCゲーム・アダルト]</span><br>
  # 発売日：2013/05/31<!--発売日--><BR>
  # ブランド名： <A href="http://www.tigresoft.com/" class="blue" target="_blank" >てぃ～ぐる</A><!--BRAND--><BR>
  # メディア： DVD-ROM<!--MEDIA--><BR>
  # 定価： 税込￥9,240(税抜￥8,800)<!--PRICE--></p>
  # <p>
  # 価格：<SPAN class="redb">￥6,980</SPAN><SPAN class="black">　コムポイント：209</SPAN>
  # </p>
  #
  _rx_parse = re.compile(
    #r'class="work_border"'
    r'class="work_thumb"'
    r'.*?'
    r'class="work_rankin"'
  , re.DOTALL)

  _rx_url = re.compile(
"""class="work_name">
<a href="([^"]+?)">
([^<]+?)</a>""")

  _rx_title = re.compile(
"""class="work_name">
<a href="([^"]+?)">
([^<]+?)</a>""")

  _rx_brand = re.compile(
"""class="maker_name">
.*>([^<]+?)</a>""")

  _rx_price = re.compile(r'class="work_price">([0-9,]+)')

  # <img src="//img.dlsite.jp/modpub/images2/work/doujin/RJ125000/RJ124710_img_sam.jpg"
  _rx_img = re.compile(r'//img\.dlsite\.jp/[0-9a-zA-Z/_]*_img_sam.jpg')
  def _iterparse(self, h):
    """
    @param  h  unicode
    @yield  {kw}
    """
    for m in self._rx_parse.finditer(h):
      hh = m.group()
      mm = self._rx_url_name.search(hh)
      if mm:
        url = mm.group(1)
        name = unescapehtml(mm.group(2))

        mm = self._rx_brand.search(hh)
        brand = unescapehtml(mm.group(1)) if mm else None

        mm = self._rx_price.search(hh)
        try: price = int(mm.group(1))
        except: price = 0

        mm = self._rx_img.search(h)
        img = 'http:' + mm.group().replace('_img_sam.jpg', '_img_main.jpg') if mm else ''

        yield {
          'url': url,     # str not None
          'image': img, # str or None
          'title': name,   # unicode not None
          'brand': brand, # unicode or None
          'price': price, # int not None
          'date': date,  # datetime or None
        }

if __name__ == '__main__':
  api = SearchApi()
  t = u"レミニセンス"
  for it in api.query(t):
    print '-' * 10
    for k,v in it.iteritems():
      print k, ':', v

# EOF
