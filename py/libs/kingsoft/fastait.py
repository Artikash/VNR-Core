# coding: utf8
# fastait.py
# 9/17/2014 jichi

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os
from sakurakit.skclass import memoizedproperty
from sakurakit.skdebug import dprint, dwarn

class _Engine(object):
  def __init__(self, dllLoader):
    self._dllLoader = dllLoader # gts.Loader
    self.dllLoaded = False

  @memoizedproperty
  def loader(self): return self._dllLoader()

  def loadDll(self):
    self.loader.init()
    self.dllLoaded = self.loader.isInitialized()
    dprint("ok = %s" % self.dllLoaded)

  def destroy(self):
    if self.dllLoaded:
      self.loader.destroy()
      dprint("pass")

class Engine(object):

  def __init__(self, loader):
    """
    @param  loader  transcom.JCLoader or transcom.ECLoader
    """
    self.__d = _Engine(loader)

  def __del__(self):
    self.destroy()

  def destroy(self): self.__d.destroy()
  def isLoaded(self): return self.__d.dllLoaded

  def load(self):
    """
    @return  bool
    """
    d = self.__d
    if not d.dllLoaded:
      d.loadDll()
      #if not d.userDicLoaded:
      #  self.loadDefaultUserDic()
    return self.isLoaded()

  def translate(self, text, to, fr):
    """
    @param  text  unicode
    @param  fr  unicode
    @param  to  unicode
    @return   unicode or None
    @throw  RuntimeError
    """
    if not self.isLoaded():
      self.load()
      if not self.isLoaded():
        raise RuntimeError("Failed to load FastAIT dll")
    return self.__d.loader.translate(text, to, fr)

  def warmup(self):
    #try: self.translate(u" ")
    try: self.translate(u"a")
    except Exception, e: dwarn(e)

def create_engine(fr='ja', to='zhs'):
  import gts
  dllpath = gts.DLLS(fr + to)
  if dllpath:
    dllname = os.path.basename(dllpath)
    bufsize = gts.EN_BUFFER_SIZE if fr == 'en' or to == 'en' else gts.ZH_BUFFER_SIZE
    return Engine(gts.Loader(dllname, bufsize))

location = Engine.location # return unicode

if __name__ == '__main__': # DEBUG
  print location()
  e = create_engine('ja')
  t = e.translate(u"こんにちは")
  print len(t)

# EOF
