# coding: utf8
# vnragent.py
# 5/3/2014 jichi
# The logic in this file must be consistent with that in vnragent.dll.

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

from sakurakit.skdebug import dprint

def match(**kwargs):
  """
  @param* pid  long
  @param* path  unicode  file executable
  @return  Engine or None
  """
  #return True
  from engines import engines
  for eng in engines():
    if eng.match(**kwargs):
      return eng
  return None

# EOF
