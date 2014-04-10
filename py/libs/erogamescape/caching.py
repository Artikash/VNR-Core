# coding: utf8
# caching.py
# 8/12/2013 jichi

__all__ = ['CachingApi']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

from restful.caching import DataCacher
from api import Api

CachingApi = DataCacher(Api, suffix='.html')

if __name__ == '__main__':
  api = CachingApi("s:/tmp/scape", expiretime=86400)
  t = 15986
  q = api.query(t)
  print q

# EOF
