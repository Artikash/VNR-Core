# coding: utf8
# powertrans.py
# 1/19/2013 jichi

import os
from sakurakit import skpaths, skos
from sakurakit.skclass import memoizedproperty
from sakurakit.skdebug import dprint, dwarn

class _Engine(object):
  def __init__(self):
    self.dllLoaded = False
    self.pathLoaded = False

  @memoizedproperty
  def nova(self):
    import nova
    return nova.Loader()

  def loadDll(self):
    self.nova.init()
    self.dllLoaded = self.nova.isInitialized()
    dprint("ok = %s" % self.dllLoaded)

  def loadPath(self):
    rpath = self.registryLocation()
    epath = self.environLocation()
    if rpath:
      path = os.path.join(rpath, r"Nova\JaEn")
      if os.path.exists(path):
        skpaths.append_path(path)
    if epath and epath != rpath:
      path = os.path.join(epath, r"Nova\JaEn")
      if os.path.exists(path):
        skpaths.append_path(path)
    self.pathLoaded = True

  def destroy(self):
    if self.dllLoaded:
      self.nova.destroy()
      dprint("pass")

  @staticmethod
  def registryLocation():
    """
    @return  unicode or None
    """
    if not skos.WIN:
      return

    REG_PATH   = r"SOFTWARE\LogoMedia\LEC Power Translator 15\Configuration"
    REG_KEY    = r"ApplicationPath"
    import _winreg
    try:
      with _winreg.ConnectRegistry(None, _winreg.HKEY_LOCAL_MACHINE) as reg: # computer_name = None
        with _winreg.OpenKey(reg, REG_PATH) as key:
          path = _winreg.QueryValueEx(key, REG_KEY)[0]
          path = path.rstrip(os.path.sep)
          return os.path.dirname(path)
    except (WindowsError, TypeError, AttributeError): pass

  @staticmethod
  def environLocation():
    return os.path.join(skpaths.PROGRAMFILES, "Power Translator 15")

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

  def translate(self, text):
    """
    @param  text  unicode or str
    @return   unicode not None
    @throw  RuntimeError
    """
    if not self.isLoaded():
      self.load()
      if not self.isLoaded():
        raise RuntimeError("Failed to load Power Translator dll")
    return self.__d.nova.translate(text) #if text else ""

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
    ret = _Engine.environLocation()
    if ret and os.path.exists(ret):
      return ret

def create_engine():
  return Engine()

location = Engine.location

# EOF
