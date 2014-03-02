# coding: utf8
# mecabtag.py
# 11/9/2013 jichi

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os
import MeCab
from sakurakit.skdebug import dprint, dwarn

# http://stackoverflow.com/questions/10299807/mecab-path-parameters-doesnot-accept-whitespace
def normalizepath(path):
  """
  @param  path  unicode
  @return  str not unicode
  """
  if not path:
    return ''
  path = path.encode('utf8', errors='ignore')
  if not os.path.exists(path):
    return ''
  #if os.name == 'nt': # not needed since I already use relative path
  #  import win32api
  #  path = win32api.GetShortPathName(path) # eliminate spaces on the path
  #  path = path.replace('\\', os.path.sep) #.lower()
  return path

def maketaggerargs(**kwargs):
  if not kwargs:
    return ''
  l = []
  for k,v in kwargs.iteritems():
    if v:
      if k in ('dicdir', 'userdic'):
        v = normalizepath(v)
      l.append('--%s %s' % (k, v))
  return ''.join(l)

def createtagger(args):
  """
  @param  args  str not unicode
  @return  MeCab.Tagger or None
  """
  try:
    ret = MeCab.Tagger(args)
    ret.parse("") # critical
    return ret
  except Exception, e:
    dwarn(e)

TAGGERS = {} # never delete
def gettagger(**kwargs):
  """Taggers are cached
  @param* dicdir  unicode  path
  @param* userdic  unicode  path
  @return  MeCab.Tagger or None
  """
  args = maketaggerargs(**kwargs)
  ret = TAGGERS.get(args)
  if not ret:
    ret = TAGGERS[args] = createtagger(args)
  return ret

# EOF
