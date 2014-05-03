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
    self.__ = _GameAgent(self)

  processAttached = Signal(long) # pid
  processDetached = Signal(long) # pid

  def isAttached(self): return bool(self.__d.pid)
  def processId(self): return self.__d.pid # -> long not None

  def attachProcess(self, pid):
    if pid and pid != self.__d.pid:
      if self.__d.pid:
        self.detachProcess()
      import inject
      inject.inject_agent(pid)

  def detachProcess(self):
    if self.__d.pid:
      rpc = self.__d.rpc
      rpc.disableAgent()
      rpc.detachAgent()
      rpc.closeAgent()

  activeChanged = Signal(bool)

  def isActive(self): return bool(self.__d.pid) and self.__d.active
  def setActive(self, t):
    d = self.__d
    if d.pid:
      d.rpc.enableAgent() if t else d.rpc.disableAgent()
    else:
      t = False
    if d.active != t:
      d.active = t
      self.activeChanged.emit(t)

class _GameAgent(object):
  def __init__(self, q):
    import rpcman
    self.rpc = rpcman.manager()

    self.rpc.agentConnected.connect(q.processAttached)
    self.rpc.agentDisconnected.connect(q.processDetached)

    self.active = True # bool

  @property # read only
  def pid(self): return self.rpc.agentProcessId()

# EOF
