# coding: utf8
# api.py
# 8/18/2013 jichi
#
# See: http://erogamescape.dyndns.org/~ap2/ero/toukei_kaiseki/select.php
# See: http://erogamescape.dyndns.org/~ap2/ero/toukei_kaiseki/tablelist.php
# See (characters from twitter): http://erogamescape.dyndns.org/~ap2/ero/toukei_kaiseki/doc/erogamescape_er_20110723.pdf
# See: http://tsubame30.exblog.jp/19548740/
# See: SELECT * from gamelist limit 1
# See: SELECT * from brandlist limit 1
#
# FIXME: Why csv does not work?!
# http://erogamescape.dyndns.org/~ap2/ero/toukei_kaiseki/create_csv.php?SQL=SELECT+gamename+from+gamelist+limit+100
#
# Example: SELECT * from gamelist limit 1
#    id	gamename	furigana	sellday	brandname	median	stdev	creater	kansouurl	checked	hanbaisuu	average2	median2	count2	comike	shoukai	model	checked2	erogame	galge	elfics	banner_url	admin_checked	max2	min2	gyutto_enc	gyutto_id	dmm	dmm_genre	dmm_genre_2	erogametokuten	total_play_time_median	time_before_understanding_fun_median	dlsite_id	dlsite_domain	the_number_of_uid_which_input_pov	the_number_of_uid_which_input_play	total_pov_enrollment_of_a	total_pov_enrollment_of_b	total_pov_enrollment_of_c	trial_url	trial_h	http_response_code	okazu	axis_of_soft_or_hard	trial_url_update_time	genre	twitter	erogetrailers	tourokubi	digiket	dmm_sample_image_count	dlsite_sample_image_count	gyutto_sample_image_count	digiket_sample_image_count	twitter_search	tgfrontier	gamemeter	twitter_data_widget_id	twitter_data_widget_id_before	twitter_data_widget_id_official
#    9426	青のはる。	アオノハル	2007-04-23	1881					t						http://www107.sakura.ne.jp/~nyannyan/aono_haru/aono_haru_main.htm	MOBILE	t	f																	0	0	0	0	0			200			0001-01-01 00:00:00			0			-1	-1	-1	-1		0

__all__ = ['Api']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import re
from datetime import datetime
from sakurakit import sknetio, skstr

DEFAULT_HOST = "http://erogamescape.dyndns.org/~ap2/ero/toukei_kaiseki"

def resethost(): sethost(DEFAULT_HOST)
def sethost(url): Api.HOST = url

def __unicode(t):
  """
  @param  t  str
  @return  unicode or None
  """
  return skstr.unescapehtml(t).decode('utf8', errors='ignore').strip() if t else None

def __str(t):
  """
  @param  t  str
  @return  str or None
  """
  return skstr.unescapehtml(t).strip() if t else None

def __int(t):
  """
  @param  t  str
  @return  int or None
  """
  if t:
    try: return int(t)
    except ValueError: pass

def __long(t):
  """
  @param  t  str
  @return  long or None
  """
  if t:
    try: return long(t)
    except ValueError: pass

def __bool(t):
  """
  @param  t  str
  @return  bool or None
  """
  return True if t == 't' else False if t == 'f' else None

def __date(t):
  """
  @param  t  str
  @return  datetime or None
  """
  if t:
    try: return datetime.strptime(t, '%Y-%m-%d')
    except ValueError: pass

_FIELDS = {
  'id': __int,         # game id
  #'brandname': __int,  # brand id
  'gamename': __unicode, # title
  'furigana': __unicode, # katagana
  'sellday': __date,
  'erogame': __bool,    # アダルト
  'okazu': __bool,      # 抜きゲー
  'genre': __unicode,   # getchu slogan

  'banner_url': __str,
  'trial_url': __str,
  'trial_h': __bool,

  'total_play_time_median': __int,               # hours
  'time_before_understanding_fun_median': __int, # hours
  #'stdev': __int,
  'median': __int,      # absolute median
  #'median2': __int,    # after filtering
  #'average2': __int,
  'count2': __int,      # data count
  #'min2': __int,
  #'max2': __int,

  'twitter': __unicode, # twitter user name
  'twitter_search': __unicode,  # twitter search key words
  'twitter_data_widget_id': __long,
  'twitter_data_widget_id_before': __long, # before release
  'twitter_data_widget_id_official': __long,

  'erogetrailers': __long,

  'gyutto_id': __long,
  'gyutto_enc': __str,

  'dlsite_id': __str,
  'dlsite_domain': __str,

  'dmm': __str,
  'dmm_genre': __str,
  'dmm_genre_2': __str,

  'digiket': __str, # such as: ITM0072787
  'erogametokuten': __long,

  'shoukai': __str,    # homepage

  #'tgfrontier': __long, # such as: http://tg-frontier.jp/soft.aspx?s=6668
  #'gamemeter': __str,   # such as: http://gamemeter.net/g/b5885fc65b

  #'model': __str,      # platform
  #'comike': __long,
  #'galge': TODO,
  #'elfics': TODO,
}

class Api(object):
  HOST = DEFAULT_HOST

  SELECT_URL = "/select.php"

  def query(self, id=0, **kwargs):
    """
    @param* id  int or str
    @param* text  int or str
    @param* limit  int
    @return  [kw] or None
    """
    req = self._makereq(id=id, **kwargs)
    h = self._fetch(**req)
    if h:
      ret = self._parse(h)
      if ret:
        for it in ret:
          self._format(it)
          self._fix(it)
        return ret

  def _fetch(self, **kwargs):
    """
    @return  str
    """
    return sknetio.postdata(self.HOST + self.SELECT_URL, gzip=True, data=kwargs)

  def _makereq(self, *args, **kwargs):
    """
    @return  kw
    """
    return {'SQL':self._makesql(*args, **kwargs)}

  def _makesql(self, id=None, text=None, limit=30):
    """
    @param* id  int or str
    @param* text  int or str
    @param* limit  int
    @return  str
    """
    fields =  ','.join(_FIELDS)
    if id:
      return "%s from gamelist where id=%s" %(fields, id) # limit is ignored
    if text:
      text = self._escapetext(text)
      return "%s from gamelist where gamename like '%s' limit %s" %(fields, text, limit)
    return ''

  _rx_escapetext = re.compile(ur'[ ,～〜~‐-]')
  def _escapetext(self, text):
    return '%' + self._rx_escapetext.sub('%', text) + '%'

  def _format(self, kw):
    """
    @param  kw
    """
    for k,v in _FIELDS.iteritems():
      kw[k] = v(kw[k])

  def _fix(self, kw):
    """
    @param  kw
    """
    count = kw['count'] = kw['count2']
    median = kw['median']
    if median and (not count or (count < 3 and median > 70)): # ignore median if the count is too small
      kw['median'] = None
    #elif not kw['median'] and kw['average2']:
    #  kw['median'] = kw['average2']
    kw['playtime'] = kw['total_play_time_median']
    kw['funtime'] = kw['time_before_understanding_fun_median']
    kw['url'] = 'http://erogamescape.dyndns.org/~ap2/ero/toukei_kaiseki/game.php?game=%s' % kw['id']

  def _parse(self, h):
    """
    @param  h  unicode  html
    @return  kw or None
    """

    return [{k:v for k,v in zip(_FIELDS, self._iterparsetd(tr))}
        for index,tr in enumerate(self._iterparsetr(h)) if index > 0] # skip the first element which is the header

  def __maketagrx(name):
    """
    @param  name  str
    @return  re
    """
    return re.compile(r"<%s>(.*?)</%s>" % (name, name), re.IGNORECASE)

  _rx_td = __maketagrx('td')
  _rx_tr = __maketagrx('tr')

  def __iterparse(self, rx, h):
    """
    @param  rx  re
    @param  h  unicode
    @yield  unicode
    """
    for m in rx.finditer(h):
      yield m.group(1)

  def _iterparsetd(self, h): return self.__iterparse(self._rx_td, h)
  def _iterparsetr(self, h): return self.__iterparse(self._rx_tr, h)

if __name__ == '__main__':
  api = Api()
  t = 18942
  t = 17716
  t = 2294
  t = 15986
  q = api.query(t)
  #print q
  for it in q:
    print it['dmm']
    print it['furigana']
    print it['digiket']
    print it['erogametokuten']
    #print it['twitter_data_widget_id_before']

  #t = 9610
  #q = api.query(t, type=api.EROGETRAILERS_TYPE)
  #for it in q:
  #  print it['title']

# EOF
