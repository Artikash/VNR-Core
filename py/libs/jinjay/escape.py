# coding: utf8
# 12/11/2014
# jinjay/escape.py

ILLEGAL_BOUND_CHARS = """ "'-:""" # space, single quote, double quote

def ystr(s): # unicode -> unicode
  if not s:
    return ''
  if '\n' in s or ':' in s or s[0] in ILLEGAL_BOUND_CHARS or s[-1] in ILLEGAL_BOUND_CHARS:
    s = s.replace('\\', '\\\\').replace('\n', '\\n').replace('"', '\\"')
    s = '"' + s + '"'
  return s

# EOF
