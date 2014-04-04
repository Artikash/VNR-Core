# coding: utf8
# textutil.py
# 3/21/2014 jichi

#import re

def completeurl(url): # str -> str
  url = url.strip()
  if '://' not in url and not url.startswith('about:'):
    url = 'http://' + url
  return url

def simplifyurl(url): # str -> str
  if not url:
    return ''
  if url.startswith('http://'):
    url = url[len('http://'):]
  if url and url[-1] == '/':
    url = url[:-1]
  return url.strip()

def elidetext(t, maxsize=20):
  if len(t) <= maxsize:
    return t
  else:
    return t[:maxsize - 4] + ' ...'

# EOF