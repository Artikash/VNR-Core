# coding: utf8
# proxyconfig.py
# 1/2/2015 jichi
# See: https://twitter.com/bypassproxy

# Alternative proxies:
# http://spy.pink
# http://freeproxy.blue
# http://netherlandsproxy.pw
JPWEBPROXY_CONFIG = { # faster, but jittered
  'host': 'http://netherlandsproxy.pw',
  'referer': 'http://netherlandsproxy.pw',
  'postkey': 'x',
  'region': 'ja',
}

USWEBPROXY_CONFIG = { # faster, but jittered
  'host': 'http://japanwebproxy.com', # cannot access erogamescape
  'referer': 'http://japanwebproxy.com',
  'postkey': 'x',
  'region': 'ja',
}

# Currently not supported yet
#USWEBPROXY_CONFIG = {
#  #'host': 'http://www.unblock-proxy.us',
#  #'referer': 'http://www.unblock-proxy.us',
#  'host': 'http://www.uswebproxy.com',
#  'referer': 'http://www.uswebproxy.com',
#  'postkey': 'x',
#  'region': 'en',
#}

# EOF
