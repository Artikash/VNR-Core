# coding: utf8
# winman.py
# 9/6/2014 jichi

from sakurakit.skclass import memoized
from sakurakit.skwinobj import SkWindowObject

@memoized
def manager(): return WindowManager()

class _WindowManager:
  def __init__(self):
    self.parent = None # QObject
    self.windows = {} # {long wid:SkWindowObject}

  def createWindow(self, wid): # long wid -> SkWindowObject
    ret = SkWindowObject(winId=wid, parent=self.parent)
    ret.referenceCount = 0 # int
    return ret

  def destroyWindow(self, w): # SkWindowObject ->
    w.setWinId(0)
    w.setParent(None)

class WindowManager:
  def __init__(self):
    self.__d = _WindowManager()

  def setParent(self, parent): self.__d.parent = parent

  def createWindow(self, wid): # long wid -> SkWindowObject
    d = self.__d
    w = d.windows.get(wid)
    if w:
      w.referenceCount += 1
    else:
      w = d.windows[wid] = d.createWindow(wid)
    return w

  def releaseWindow(self, wid): # long wid ->
    d = self.__d
    w = d.windows.get(wid)
    if w:
      w.referenceCount -= 1
      if w.referenceCount <= 0:
        del d.windows[wid]
        d.destroyWindow(w)

# EOF
