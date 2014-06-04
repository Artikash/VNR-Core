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

UTF16_ENCODING = 'utf-16'
SJIS_ENCODING = 'shift-jis'

ENINES = []
def engines():
  """
  @yield  Engine
  """
  if not ENINES:
    ENGINES = [
      AmuseCraftEngine(),
      BGIEngine(),
      EushullyEngine(),
      MajiroEngine(),
      SiglusEngine(),
      SilkysEngine(),
      #RejetEngine(),
    ]
  return ENGINES

class Engine(object): # placeholder
  NAME = ''
  ENCODING = ''
  REGION_LOCKED = False # if it is locked to Japanese and cannot be run in other locale

  def name(self): return self.NAME
  def encoding(self): return self.ENCODING
  def isRegionLocked(self): return self.REGION_LOCKED

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

  def glob(self, pattern, **kwargs):
    if isinstance(pattern, list) or isinstance(pattern, tuple):
      return self.globAppDirectories(pattern, **kwargs)
    else:
      return self.globAppDirectory(pattern, **kwargs)

  def globs(self, *args, **kwargs): return bool(self.glob(*args, **kwargs))

  def existsAppFile(self, relpath, pid=0, path=''):
    """
    @param  relpaths  unicode
    @return  bool
    """
    if path:
      path = os.path.dirname(path)
    if not path and pid:
      path = self.getAppDirectory(pid)
    return bool(path) and os.path.exists(os.path.join(path, relpath))

  def existsAppFiles(self, relpaths, pid=0, path=''):
    """
    @param  relpaths  [unicode]
    @return  bool
    """
    if path:
      path = os.path.dirname(path)
    if not path and pid:
      path = self.getAppDirectory(pid)
    if not path or not relpaths:
      return False
    for it in relpaths:
      if not os.path.exists(os.path.join(path, it)):
        return False
    return True

  def exists(self, pattern, **kwargs):
    if isinstance(pattern, list) or isinstance(pattern, tuple):
      return self.existsAppFiles(pattern, **kwargs)
    else:
      return self.existsAppFile(pattern, **kwargs)

# 4/20/2014 jichi
class MajiroEngine(Engine):

  NAME = "Majiro" # str, override
  ENCODING = SJIS_ENCODING # str, override

  def match(self, **kwargs): # override
    return self.globs(("data*.arc", "stream*.arc"), **kwargs)

# 5/11/2014 jichi
class BGIEngine(Engine):

  NAME = "BGI" # str, override
  ENCODING = SJIS_ENCODING # str, override
  REGION_LOCKED = True # it cannot be launched using AppLocale in Chinese locale

  def match(self, **kwargs): # override
    return self.globs("BGI.*", **kwargs)

# 5/25/2014 jichi
class RejetEngine(Engine):

  NAME = "Rejet" # str, override
  ENCODING = SJIS_ENCODING # str, override

  def match(self, **kwargs): # override
    return self.exists(("gd.dat", "pf.dat", "sd.dat"), **kwargs)

# 5/25/2014 jichi
class SiglusEngine(Engine):

  NAME = "SiglusEngine" # str, override
  ENCODING = UTF16_ENCODING # str, override

  def match(self, **kwargs): # override
    return self.exists("SiglusEngine.exe", **kwargs)

# 5/31/2014 jichi
class SilkysEngine(Engine):

  NAME = "Silkys" # str, override
  ENCODING = SJIS_ENCODING # str, override

  REGION_LOCKED = True # It simply cannot display gbk, which become "?". Need hijack encoding or gdi functions

  def match(self, **kwargs): # override
    #return self.exists("Silkys.exe", **kwargs)
    return self.exists(("data.arc", "effect.arc", "mes.arc"), **kwargs)

# 6/1/2014 jichi
class EushullyEngine(Engine):

  NAME = "Eushully" # str, override
  ENCODING = SJIS_ENCODING # str, override

  REGION_LOCKED = True # text become ????, and it cannot translate name

  def match(self, **kwargs): # override
    return self.exists("AGERC.DLL", **kwargs)

# 6/3/2014 jichi
class AmuseCraftEngine(Engine):

  NAME = "AMUSE CRAFT" # str, override
  ENCODING = SJIS_ENCODING # str, override

  def match(self, **kwargs): # override
    return self.globs("*.pac", **kwargs) and self.exists(("dll/resource.dll", "dll/pal.dll"), **kwargs)

# EOF

## 5/10/2014 jichi
#class CMVSEngine(Engine):
#
#  NAME = "CMVS" # str, override
#  ENCODING = SJIS_ENCODING # str, override
#
#  def match(self, **kwargs): # override
#    return bool(self.glob("data/pack/*.cpz", **kwargs))
#
## 5/11/2014 jichi
#class YurisEngine(Engine):
#
#  NAME = "YU-RIS" # str, override
#  ENCODING = SJIS_ENCODING # str, override
#
#  def match(self, **kwargs): # override
#    return bool(
#      self.glob("*.ypf", **kwargs) or self.glob("pac/*.ypf", **kwargs)
#    ) and not self.exists("noblesse.exe", **kwargs)
#
## 5/11/2014 jichi
#class GXPEngine(Engine):
#
#  NAME = "GXP" # str, override
#  ENCODING = UTF16_ENCODING # str, override
#
#  def match(self, **kwargs): # override
#    return bool(self.glob("*.gxp", **kwargs))
