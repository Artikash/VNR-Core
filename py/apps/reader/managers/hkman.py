# coding: utf8
# hkman.py
# 1/29/2014 jichi

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

from sakurakit import skos
from sakurakit.skdebug import dprint, dwarn
from sakurakit.skclass import memoized

if not skos.WIN:
  class dummy_pyhk:
    def addHotkey(self, *args, **kwargs): pass
    def removeHotkey(self, *args, **kwargs): pass

@memoized
def manager(): return HotkeyManager()

class HotkeyManager:

  def __init__(self):
    self.__d = _HotkeyManager()

  def isEnabled(self): return self.__d.enabled
  def setEnabled(self, v): self.__d.enabled = v

class _HotkeyManager(object):
  def __init__(self):
    self.enabled = False # bool

  @memoized
  def pyhk(self): # pyhk instance
    if skos.WIN:
      from pyhk import pyhk
      return pyhk()
    else:
      return dummy_pyhk()

# Debug entry
if __name__ == '__main__': #and os.name == 'nt':
  # http://schurpf.com/python/python-hotkey-module/
  import pyhk
  def fun_k():
    print "key"
  def fun_m():
    print "mouse"

  #create pyhk class instance
  hot = pyhk.pyhk()
  #add hotkey
  hot.addHotkey(['Ctrl', 'A'],fun_k)
  hot.removeHotkey(['Ctrl', 'A'])

  hot.addHotkey(['mouse right'],fun_m)

  # Start event loop and block the main thread
  import pythoncom
  pythoncom.PumpMessages() # wait forever

# EOF
