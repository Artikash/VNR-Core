# coding: utf8
# gameagent.py
# 5/2/2014 jichi

from PySide.QtCore import QObject, Signal
from sakurakit.skclass import memoized
from sakurakit.skdebug import dprint

@memoized
def manager(): return GameAgent()

class GameAgent(QObject):
  def __init__(self, parent=None):
    super(GameAgent, self).__init__(parent)

# EOF
