# coding: utf8
# caching.py
# 8/12/2013 jichi

__all__ = 'CachingGameApi',

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

#from search import SearchApi
from soft import GameApi

def _htmlcacher(cls):
  from restful.offline import DataCacher
  return DataCacher(cls, suffix='.html')

#CachingSearchApi = _htmlcacher(SearchApi)
CachingGameApi = _htmlcacher(GameApi)

if __name__ == '__main__':
  # http://www.freem.ne.jp/win/game/8329
  # http://www.freem.ne.jp/win/game/3055
  cachedir = 'tmp'
  api = CachingGameApi(cachedir=cachedir, expiretime=86400)
  k = 748164
  k = 779363
  q = api.query(k)
  print q['title']
  print q['date']
  #for k,v in q.iteritems():
  #  print k, ':', v

# EOF
