# coding: utf8
# winpicker.py
# 10/6/2012 jichi
# Windows only
# See: http://sourceforge.net/apps/mediawiki/pyhook/index.php?title=PyHook_Tutorial

# On mouse click warning:
# See: http://sourceforge.net/tracker/?func=detail&aid=2986042&group_id=235202&atid=1096323
# func = self.keyboard_funcs.get(msg)
# => func = self.keyboard_funcs.get( int(str(msg)) )

__all__ = ['WindowPicker']

from PySide.QtCore import QObject, Signal
from sakurakit.skclass import Q_Q
from sakurakit.skdebug import dprint
from sakurakit.skunicode import u

import os
if os.name == 'nt':

  @Q_Q
  class _WindowPicker(object):

    _instance = None

    def __init__(self):
      _WindowPicker._instance = self
      self.singleShot = True
      self._active = False
      self._hookManager = None # HookManager instance

    @property
    def active(self): return self._active

    @active.setter
    def active(self, t):
      if t != self._active:
        if t:
          self.hookManager.HookMouse()
        elif self._hookManager:
          self._hookManager.UnhookMouse()
        self._active = t

    @property
    def hookManager(self):
      if not self._hookManager:
        dprint("creating pyhook manager")
        import pyHook
        self._hookManager = pyHook.HookManager()
        self._hookManager.MouseLeftDown = \
        self._hookManager.MouseRightDown = \
        self._hookManager.MouseMiddleDown = \
          _WindowPicker._onMousePress
      return self._hookManager

    def _clickWindow(self, hwnd, title):
      if self.singleShot:
        self.active = False
      self.q.windowClicked.emit(hwnd, title)

    @staticmethod
    def _onMousePress(event):
      """
      @return  bool  Whether pass the event to other handlers
      """
      dprint("enter")
      d = _WindowPicker._instance
      if d._active:
        dprint("active")
        #print "---"
        #print "  message name:", event.MessageName
        #print "  message (MSG):", event.Message
        #print "  event time:", event.Time
        #print "  window (HWND):", event.Window
        #print "  window title:", event.WindowName
        #print "  position:", event.Position
        #print "  wheel:", event.Wheel
        #print "  injected:", event.Injected
        #print "---"

        hwnd = event.Window
        title = u(event.WindowName)
        if hwnd:
          dprint("found hwnd")
          d._clickWindow(hwnd, title)
          dprint("leave: ret = False")
          return False # eat the event
      dprint("leave: ret = True")
      return True # return True to pass the event to other handlers

  class WindowPicker(QObject):
    #instance = None

    def __init__(self, parent=None):
      super(WindowPicker, self).__init__(parent)
      self.__d = _WindowPicker(self)
      #WindowPicker.instance = self
      dprint("pass")

    windowClicked = Signal(long, unicode)

    def isSingleShot(self): return self.__d.singleShot
    def setSingleShot(self, t): self.__d.singleShot = t

    def isActive(self): return self.__d.active
    def start(self): self.__d.active = True
    def stop(self): self.__d.active = False

else: # dummy

  class WindowPicker(QObject):

    windowClicked = Signal(long, unicode) # hwnd, title

    def isSingleShot(self): return False
    def setSingleShot(self, t): pass

    def isActive(self): return False
    def start(self): pass
    def stop(self): pass

# Debug entry
if __name__ == '__main__' and os.name == 'nt':
  # Start event loop and block the main thread
  import pythoncom
  pythoncom.PumpMessages() # wait forever

# EOF
