# coding: utf8
# gameengine.py
# 10/3/2013 jichi
# Windows only

def inject(pid):
  """
  @param  pid  long
  @return  bool
  """
  from engines import engines
  for eng in engines():
    if eng.match(pid):
      return eng.inject(pid)
  return False

# EOF
