# coding: utf8
# gameagent.py
# 5/2/2014 jichi

from functools import partial
from PySide.QtCore import QObject, Signal, QTimer
from sakurakit.skclass import memoized, Q_Q
from sakurakit.skdebug import dprint
from vnragent import vnragent
from mytr import my
import config, growl, settings

@memoized
def global_(): return GameAgent()

class GameAgent(QObject):
  def __init__(self, parent=None):
    super(GameAgent, self).__init__(parent)
    self.__d = _GameAgent(self)

  processAttached = Signal(long) # pid
  processDetached = Signal(long) # pid

  processAttachTimeout = Signal(long)
  engineChanged = Signal(str) # name

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
        self.__d.injectTimer.start()
      return ok

  def detachProcess(self):
    if self.__d.connectedPid:
      rpc = self.__d.rpc
      rpc.disableAgent()
      #rpc.detachAgent()
      rpc.closeAgent()
    self.__d.clear()

  def hasEngine(self): return bool(self.__d.engineName)

  ## Query ##

  @staticmethod
  def guessEngine(**kwargs):
    """
    @param* pid  long
    @param* path  unicode  game executable path
    @return  vnragent.Engine
    """
    return vnragent.match(**kwargs)

  ## States ##

  def disable(self):
    if self.__d.connectedPid:
      self.__d.rpc.disableAgent()

  #def setGameLanguage(self, v):
  #  self.__d.gameLanguage = v

  #def setUserEncoding(self, v): # str ->
  #  d = self.__d
  #  if d.userEncoding != v:
  #    d.userEncoding = v

  def sendSettings(self):
    if self.isConnected():
      self.__d.sendSettings()

  #def engine

_SETTINGS_DICT = {
  'windowTranslationEnabled': 'isWindowTranslationEnabled',
  'windowTranscodingEnabled': 'isWindowTranscodingEnabled',
  'windowTextVisible': 'isWindowTextVisible',

  'embeddedScenarioVisible': 'isEmbeddedScenarioVisible',
  'embeddedScenarioTranslationEnabled': 'isEmbeddedScenarioTranslationEnabled',
  'embeddedScenarioTranscodingEnabled': 'isEmbeddedScenarioTranscodingEnabled',
  'embeddedNameVisible': 'isEmbeddedNameVisible',
  'embeddedNameTranslationEnabled': 'isEmbeddedNameTranslationEnabled',
  'embeddedNameTranscodingEnabled': 'isEmbeddedNameTranscodingEnabled',
  'embeddedOtherVisible': 'isEmbeddedOtherVisible',
  'embeddedOtherTranslationEnabled': 'isEmbeddedOtherTranslationEnabled',
  'embeddedOtherTranscodingEnabled': 'isEmbeddedOtherTranscodingEnabled',
}

@Q_Q
class _GameAgent(object):
  def __init__(self, q):
    import rpcman
    self.rpc = rpcman.manager()

    self.rpc.agentConnected.connect(q.processAttached)
    self.rpc.agentDisconnected.connect(q.processDetached)
    self.rpc.engineReceived.connect(self._onEngineReceived)

    t = self.injectTimer = QTimer(q)
    t.setSingleShot(False)
    t.setInterval(5000)
    t.timeout.connect(self._onInjectTimeout)

    q.processAttached.connect(self._onAttached)

    self.clear()
    def f():pass

    ss = settings.global_()
    for k,v in _SETTINGS_DICT.iteritems():
      sig = getattr(ss, k + 'Changed')

      sig.connect(partial(lambda k, t:
        self.connectedPid and self.sendSetting(k, t)
      , k))

  def clear(self):
    self.injectedPid = 0 # long
    self.engineName = '' # str
    self.gameEncoding = 'shift-jis' # placeholder
    #self.gameLanguage = 'ja' # str
    #self.gameEncoding = '' # str
    #self.userLanguage = 'en' # str
    #self.userEncoding = '' # str

  @property # read only
  def connectedPid(self): return self.rpc.agentProcessId()

  def _onInjectTimeout(self):
    if self.injectedPid:
      self.q.processAttachTimeout.emit(self.injectedPid)
      self.injectedPid = 0

  def _onAttached(self):
    self.injectTimer.stop()
    self.sendSettings()
    #self.rpc.enableAgent()

  def _onEngineReceived(self, name): # str
    self.engineName = name
    self.q.engineChanged.emit(name)
    if name:
      growl.notify("%s: %s" % (my.tr("Detect game engine"), name))

  def sendSettings(self):
    ss = settings.global_()
    data = {k:apply(getattr(ss, v)) for k,v in _SETTINGS_DICT.iteritems()}
    data['gameEncoding'] = self.gameEncoding
    if config.APP_DEBUG:
      data['debug'] = True
    self.rpc.setAgentSettings(data)

  def sendSetting(self, k, v):
    data = {k:v}
    self.rpc.setAgentSettings(data)

# EOF
