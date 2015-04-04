# coding: utf8
# 2ztrans.py
# 6/7/2013 jichi

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os
from sakurakit import skpaths, skos
from sakurakit.skclass import memoizedproperty
from sakurakit.skdebug import dprint, dwarn

class _Engine(object):
  def __init__(self):
    self.dllLoaded = False
    self.pathLoaded = False

  @memoizedproperty
  def loader(self):
    import j2kengine
    return j2kengine.Loader()

  def loadDll(self):
    self.loader.init()
    self.dllLoaded = self.loader.isInitialized()
    dprint("ok = %s" % self.dllLoaded)

  def loadPath(self):
    path = self.registryLocation()
    if path and os.path.exists(path):
      skpaths.append_path(path)
    self.pathLoaded = True

  def destroy(self):
    if self.dllLoaded:
      self.loader.destroy()
      dprint("pass")

  @staticmethod
  def registryLocation():
    """
    @return  unicode or None
    """
    if not skos.WIN:
      return
    REG_PATH   = r"Software\ChangShin\ezTrans"
    REG_KEY    = r"FilePath"
    import _winreg
    try:
      with _winreg.ConnectRegistry(None, _winreg.HKEY_CURRENT_USER) as reg: # computer_name = None
        with _winreg.OpenKey(reg, REG_PATH) as key:
          return _winreg.QueryValueEx(key, REG_KEY)[0]
    except WindowsError: pass

class Engine(object):

  def __init__(self):
    self.__d = _Engine()

  def __del__(self):
    self.destroy()

  def destroy(self): self.__d.destroy()
  def isLoaded(self): return self.__d.dllLoaded

  def load(self):
    """
    @return  bool
    """
    d = self.__d
    if not d.pathLoaded:
      d.loadPath()
    if not d.dllLoaded:
      d.loadDll()
    return self.isLoaded()

  def translate(self, text, ehnd=True):
    """
    @param  text  unicode
    @param* ehnd  whenther enable ehnd
    @return   unicode
    @throw  RuntimeError
    """
    if not self.isLoaded():
      self.load()
      if not self.isLoaded():
        raise RuntimeError("Failed to load ezTrans dll")
    return self.__d.loader.translate(text, ehnd=ehnd)

  def warmup(self):
    #try: self.translate(u" ")
    try: self.translate(u"あ")
    except Exception, e: dwarn(e)

  @staticmethod
  def location():
    """
    @return  str or None
    """
    ret = _Engine.registryLocation()
    if ret and os.path.exists(ret):
      return ret

def create_engine(): return Engine()

location = Engine.location # return unicode

if __name__ == '__main__': # DEBUG
  print location()
  print len(create_engine().translate(u"こんにちは"))

# EOF
