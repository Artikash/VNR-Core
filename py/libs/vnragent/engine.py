# coding: utf8
# engine.py
# 5/3/2014 jichi
# The logic in this file must be consistent with that in vnragent.dll.

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os
from glob import glob
from sakurakit.skdebug import dprint

# TODO: Apply this transformation for all paths at Python side
def _complete_path(path):
  """Repair remote path by padding leading '\\'
  @param  path  unicode
  @return  unicode  path
  """
  return '\\' + path if path and path[0] == '\\' else path

class Engine:
  def __init__(self, name='', wideChar=False, regionLocked=False, vnrlocale=False, **kwargs):
    self.name = name # str
    self.wideChar = wideChar # bool
    self.regionLocked = regionLocked # bool
    self.vnrlocale = vnrlocale # bool

  # Not used
  #def encoding(self): return 'utf-16' if self.wideChar else 'shift-jis'

class EngineFinder:
  def __init__(self, pid=0, exepath='', dirpath=''):
    """
    @param* pid  long  process id
    @param* exepath  unicode  executable path
    @param* dirpath  unicode  process directory path
    """
    if not exepath and pid:
      exepath = skwin.get_process_path(pid)
    if not dirpath and exepath:
      dirpath = os.path.dirname(exepath)
    self.pid = pid # long
    self.exepath = exepath # unicode
    self.dirpath = dirpath # unicode
    #self.processName = skwin.get_process_name(pid)

  def eval(self, e):
    """
    @param  e  list or str
    @return  bool
    """
    if not e:
      return False
    if isinstance(e, list):
      for it in e:
        if not self.eval(it):
          return False
      return True
    else: # e is str or unicode
      if '*' in e:
        return self._globs(e)
      else:
        return self._exists(e)

  def _globs(self, relpath):
    """
    @param  relpath  unicode
    @return  bool
    """
    return bool(self.dirpath and glob(_complete_path(os.path.join(self.dirpath, relpath))))

  def _exists(self, relpath):
    """
    @param  relpath  unicode
    @return  bool
    """
    return bool(self.dirpath) and os.path.exists(_complete_path(os.path.join(self.dirpath, relpath)))

# EOF
