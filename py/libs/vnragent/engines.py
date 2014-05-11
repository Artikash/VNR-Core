# coding: utf8
# engines.py
# 5/3/2014 jichi
# The logic in this file must be consistent with that in vnragent.dll.

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os
from sakurakit.skdebug import dprint
#from sakurakit.skclass import memoized

ENINES = []
def engines():
  """
  @yield  Engine
  """
  if not ENINES:
    ENGINES = [
      #CMVSEngine(),
      MajiroEngine(),
    ]
  return ENGINES

class Engine(object): # placeholder
  NAME = ''
  ENCODING = ''

  def name(self): return self.NAME
  def encoding(self): return self.ENCODING

  # Pure virtual functions
  def match(self, pid):
    """
    @param  pid  long
    @return  bool
    """
    return False

  def inject(self, pid):
    """
    @param  pid  long
    @return  bool
    """
    return False

  # Helpers

  def getAppName(self, pid):
    """
    @param  pid  long
    @return  unicode or None
    """
    from sakurakit import skwin
    return skwin.get_process_name(pid)

  def getAppPath(self, pid):
    """
    @param  pid  long
    @return  unicode or None
    """
    from sakurakit import skwin
    return skwin.get_process_path(pid)

  def getAppDirectory(self, pid):
    """
    @param  pid  long
    @return  unicode or None
    """
    path = self.getAppPath(pid)
    if path:
      return os.path.dirname(path)

  def globAppDirectory(self, pattern, pid=0, path=''):
    """
    @param  pattern  str
    @return  [unicode path] or None
    """
    if path:
      path = os.path.dirname(path)
    if not path and pid:
      path = self.getAppDirectory(pid)
    if path:
      from glob import glob
      return glob(os.path.join(path, pattern))

  def globAppDirectories(self, patterns, pid=0, path=''):
    """Return all paths or None if failed
    @param  pattern  [str]
    @return  [unicode path] or None  if return list, it must have the same length as patterns
    """
    if path:
      path = os.path.dirname(path)
    if not path and pid:
      path = self.getAppDirectory(pid)
    if path:
      from glob import glob
      ret = []
      for pat in patterns:
        r = glob(os.path.join(path, pat))
        if not r:
          return None
        ret.append(r)
      return ret

class MajiroEngine(Engine):

  NAME = "Majiro" # str, override
  ENCODING = "shift-jis" # str, override

  def match(self, **kwargs): # override
    return bool(self.globAppDirectories(("data*.arc", "stream*.arc"), **kwargs))

class CMVSEngine(Engine):

  NAME = "CMVS" # str, override
  ENCODING = "shift-jis" # str, override

  def match(self, **kwargs): # override
    return bool(self.globAppDirectory("data/pack/*.cpz", **kwargs))

# EOF
