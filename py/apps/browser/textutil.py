# coding: utf8
# textutil.py
# 3/21/2014 jichi

#import re

def completeurl(url): # str -> str
  url = url.strip()
  if '://' not in url:
    url = 'http://' + url
  return url

def simplifyurl(url): # str -> str
  if url and url.startswith('http://'):
    url = url[len('http://'):]
  if url and url[-1] == '/':
    url = url[:-1]
  return url

# EOF
