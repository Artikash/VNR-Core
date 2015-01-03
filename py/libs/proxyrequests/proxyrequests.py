# coding: utf8
# proxyrequests.py
# 1/2/2015 jichi

__all__ = 'Session'

import requests

class Session(object):
  def __init__(self, config, session=None):
    """
    @param  config  {'host': str, 'postkey': str, 'referer': str}
    @param* session  requests.Session
    """
    self.config = config
    self.session = session or requests.Session()

  def _proxyurl(self, url):
    """
    @param  url  str
    @return  str or None
    """
    host = self.config['host']
    data = {self.config['postkey']: url}
    headers = {'Referer': self.config.get('referer') or self.config['host']}
    r = self.session.post(host, data=data, headers=headers, allow_redirects=False)
    return r.headers.get('location')

  def get(self, url, *args, **kwargs):
    url = self._proxyurl(url)
    if not url:
      return requests.Response()
    #print url
    headers = kwargs.get('headers') or {}
    headers['referer'] = self.config.get('referer') or self.config['host']
    return self.session.get(url, headers=headers, **kwargs)

  def post(self, url, *args, **kwargs):
    url = self._proxyurl(url)
    if not url:
      return requests.Response()
    #print url
    headers = kwargs.get('headers') or {}
    headers['referer'] = self.config.get('referer') or self.config['host']
    return self.session.post(url, *args, **kwargs) if url else requests.Response()

if __name__ == '__main__':
  #import sys
  #sys.path.append('..')
  import proxyconfig
  url = "http://erogamescape.dyndns.org/~ap2/ero/toukei_kaiseki/select.php"
  s = Session(proxyconfig.WEBPROXY_CONFIG)
  r = s.get(url)
  print r.content

# EOF
