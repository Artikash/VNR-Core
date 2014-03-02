# coding: utf8
# 11/28/2013 jichi

__all__ = ['CachingItemApi']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

#from search import SearchApi
from item import ItemApi
from review import ReviewApi

def _htmlcacher(cls):
  from restful.caching import DataCacher
  return DataCacher(cls, suffix='.html')

#CachingSearchApi =_htmlcacher(SearchApi)
CachingItemApi = _htmlcacher(ItemApi)
CachingReviewApi = _htmlcacher(ReviewApi)

if __name__ == '__main__':
  cachedir = 'tmp'
  api = CachingItemApi(cachedir=cachedir, expiretime=86400)
  k = 45242
  q = api.query(k)
  print q['image']
  #for k,v in q.iteritems():
  #  print k, ':', v

# EOF
