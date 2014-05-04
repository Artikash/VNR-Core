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
    self.__d = _GameAgent(self)

  processAttached = Signal(long) # pid
  processDetached = Signal(long) # pid

  # Not used
  #def clear(self): self.__d.clear()

  ## Inject ##

  def isAttached(self): return bool(self.__d.pid)
  def processId(self): return self.__d.pid # -> long not None

  def attachProcess(self, pid):
    if pid and pid != self.__d.pid:
      if self.__d.pid:
        self.detachProcess()
      self.clear()
      import inject
      inject.inject_agent(pid)

  def detachProcess(self):
    if self.__d.pid:
      rpc = self.__d.rpc
      rpc.disableAgent()
      #rpc.detachAgent()
      rpc.closeAgent()
    self.clear()

  ## States ##

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

  #def setGameLanguage(self, v):
  #  self.__d.gameLanguage = v

  def setUserEncoding(self, v): # str ->
    d = self.__d
    if d.userEncoding != v:
      d.userEncoding = v

class _GameAgent(object):
  def __init__(self, q):
    import rpcman
    self.rpc = rpcman.manager()

    self.rpc.agentConnected.connect(q.processAttached)
    self.rpc.agentDisconnected.connect(q.processDetached)

    self.clear()

  def clear(self):
    self.active = False # bool
    #self.gameLanguage = 'ja' # str
    #self.gameEncoding = '' # str
    #self.userLanguage = 'en' # str
    self.userEncoding = '' # str

  @property # read only
  def pid(self): return self.rpc.agentProcessId()

# EOF
