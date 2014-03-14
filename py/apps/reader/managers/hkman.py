# coding: utf8
# hkman.py
# 1/29/2014 jichi

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

from functools import partial
from PySide.QtCore import QObject, Signal, Property
from sakurakit import skos
from sakurakit.skdebug import dprint, dwarn
from sakurakit.skclass import memoized
from sakurakit.skqml import QmlObject
import settings

if not skos.WIN:
  class dummy_pyhk:
    def addHotkey(self, *args, **kwargs): pass
    def removeHotkey(self, *args, **kwargs): pass

HOTKEY_DELIM = '\n'
def packhotkey(l): # list -> str
  return HOTKEY_DELIM.join(l)
def unpackhotkey(s): # str -> list
  return s.split(HOTKEY_DELIM)

@memoized
def manager(): return HotkeyManager()

class HotkeyManager(QObject):

  def __init__(self, parent=None):
    super(HotkeyManager, self).__init__(parent)
    self.__d = _HotkeyManager()

  enabledChanged = Signal(bool)
  def isEnabled(self): return self.__d.enabled
  def setEnabled(self, t):
    d = self.__d
    if d.enabled != t:
      dprint("t = %s" % t)
      d.enabled = t
      if t:
        d.start()
      else:
        d.stop()
      self.enabledChanged.emit(t)

  #def setTtsEnabled(self, t):
  #  self.__d.setMappingEnabled('tts', t)
  #  settings.global_().setTtsHotkeyEnabled(t)

  #def setTtsHotkey(self, k):
  #  self.__d.setMappingKey('tts', k)
  #  settings.global_().setTtsHotkey(k)

class _HotkeyManager(object):
  def __init__(self):
    self.enabled = False # bool

    ss = settings.global_()

    # key mapping
    self._mapping = {
      'tts': { # string task name
        'do': self._onTts, # function
        'on': ss.isTtsHotkeyEnabled(), # bool
        'key': ss.ttsHotkey(), # string
      }
    }

    ss.ttsHotkeyEnabledChanged.connect(partial(self.setMappingEnabled, 'tts'))
    ss.ttsHotkeyChanged.connect(partial(self.setMappingKey, 'tts'))

    #from PySide import QtCore
    #qApp = QtCore.QCoreApplication.instance()
    #qApp.aboutToQuit.connect(self.stop)

  def start(self):
    for hk in self._mapping.itervalues():
      if hk['on']:
        self._addHotkey(hk['key'])

  def stop(self):
    for hk in self._mapping.itervalues():
      if hk['on']:
        self._removeHotkey(hk['key'])

  def setMappingEnabled(self, name, t):
    m = self._mapping[name]
    if m['on'] != t:
      m['on'] = t
      k = m['key']
      if k:
        self._toggleHotkey(k, t)

  def setMappingKey(self, name, k):
    m = self._mapping[name]
    oldk = m['key']
    if oldk != k:
      m['key'] = k
      if oldk:
        self._removeHotkey(oldk)
      if k:
        self._addHotkey(k)

  def _addHotkey(self, k):
    l = unpackhotkey(k)
    self.pyhk.addHotkey(l, partial(self._onHotkey, k))

  def _removeHotkey(self, k): # str
    l = unpackhotkey(k)
    self.pyhk.removeHotkey(l)

  def _toggleHotkey(self, k, t):
    if t:
      self._addHotkey(k)
    else:
      self._removeHotkey(k)

  @memoized
  def pyhk(self): # pyhk instance
    if skos.WIN:
      from pyhk import pyhk
      return pyhk()
    else:
      return dummy_pyhk()

  def _onHotkey(self, key): # callback
    for name, hk in self._mapping.iteritems():
      if hk == key and hk['on']:
        apply(hk['do'])

  @staticmethod
  def _onTts():
    import textman
    dwarn("speak last text")

@QmlObject
class HotkeyManagerProxy(QObject):
  def __init__(self, parent=None):
    QtCore.QObject.__init__(self, parent)
    manager().enabledChanged.connect(self.enabledChanged)

  enabledChanged = Signal(bool)
  enabled = Property(bool,
      lambda self: manager().isEnabled(),
      lambda self, v: manager().setEnabled(t),
      notify=enabledChanged)

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
