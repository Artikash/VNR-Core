# coding: utf8
# gameagent.py
# 5/2/2014 jichi

from PySide.QtCore import QObject, Signal
from sakurakit.skclass import memoized
from sakurakit.skdebug import dprint

@memoized
def global_(): return GameAgent()

class GameAgent(QObject):
  def __init__(self, parent=None):
    super(GameAgent, self).__init__(parent)
    self.__d = _GameAgent(self)

  processAttached = Signal(long) # pid
  processDetached = Signal(long) # pid

  # Not used
  #def clear(self): self.__d.clear()

  ## Inject ##

  def isAttached(self): return bool(self.__d.injectedPid)
  def attachedPid(self): return self.__d.injectedPid # -> long not None

  def isConnected(self): return bool(self.__d.connectedPid)
  def connectedPid(self): return self.__d.connectedPid # -> long not None

  def attachProcess(self, pid): # -> bool
    if pid == self.__d.injectedPid:
      return True
    else:
      if self.__d.connectedPid:
        self.detachProcess()
      self.__d.clear()
      import inject
      ok = inject.inject_agent(pid)
      if ok:
        self.__d.injectedPid = pid
      return ok

  def detachProcess(self):
    if self.__d.connectedPid:
      rpc = self.__d.rpc
      rpc.disableAgent()
      #rpc.detachAgent()
      rpc.closeAgent()
    self.__d.clear()

  ## States ##

  activeChanged = Signal(bool)

  def isActive(self): return bool(self.__d.connectedPid) and self.__d.active
  def setActive(self, t):
    d = self.__d
    if d.connectedPid:
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
    self.injectedPid = 0 # long
    self.active = False # bool
    #self.gameLanguage = 'ja' # str
    #self.gameEncoding = '' # str
    #self.userLanguage = 'en' # str
    self.userEncoding = '' # str

  @property # read only
  def connectedPid(self): return self.rpc.agentProcessId()

# EOF
