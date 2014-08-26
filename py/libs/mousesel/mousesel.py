# coding: utf8
# mousesel.py
# 8/21/2014 jichi
# Windows only

#__all__ = ['MouseSelector']

from sakurakit.skclass import memoized

from sakurakit import skos
if skos.WIN:
  from pymousesel import MouseSelector

else:
  from PySide.QtCore import Signal, QObject

  class MouseSelector(QObject): # dummy
    def __init__(self, parent=None):
      super(MouseSelector, self).__init__(parent)

    selected = Signal(int, int, int, int) # x, y, width, height

    def isEnabled(self): return False
    def setEnabled(self, t): pass

@memoized
def global_(): return MouseSelector()

# EOF
