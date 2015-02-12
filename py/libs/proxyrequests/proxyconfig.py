# coding: utf8
# proxyconfig.py
# 1/2/2015 jichi

WEBPROXY_CONFIG = { # faster, but jittered
  'host': 'http://webproxy.nu',
  'referer': 'http://webproxy.nu',
  'postkey': 'x',
  'region': 'ja',
}

USWEBPROXY_CONFIG = {
  'host': 'http://uswebproxy.nu',
  'referer': 'http://uswebproxy.nu',
  'postkey': 'x',
  'region': 'en',
}

WEBSERVER_CONFIG = { # slower, but not jittered
  'host': 'http://web-server.se',
  'referer': 'http://web-server.se',
  'postkey': 'q',
  'region': 'ja',
}

# EOF
