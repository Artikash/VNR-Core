# coding: utf8
# srman.py
# 11/1/2014 jichi

from PySide.QtCore import Qt, QObject, QThread
from sakurakit.skclass import memoized, Q_Q

@memoized
def manager(): return SpeechRecognitionManager()

class SpeechRecognitionManager(QObject):

  def __init__(self, parent=None):
    super(SpeechRecognitionManager, self).__init__(parent)
    self.__d = _SpeechRecognitionManager(self)

  textReceived = Signal(unicode)

  def start(self): pass
  def stop(self): pass

  def language(self): return self.__d.language
  def setLanguage(self, v): self.__d.language = v

  def isSingleShot(self): return self.__d.singleShot
  def setSingleShot(self, t): self.__d.singleShot = t

  def isAutoStopEnabled(self): return self.__d.autoStop
  def setAutoStopEnabled(self, t): self.__d.autoStop = t

@Q_Q
class _SpeechRecognitionManager:
  def __init__(self):
    self.autoStop = True # bool
    self.singleShot = False # bool
    self.language = 'ja'
    self._thread = None # SpeechRecognitionThread

class SpeechRecognitionThread(QThread):

  def __init__(self, manager, parent=None):
    super(SpeechRecognitionThread, self).__init__(parent)
    self.__d = _SpeechRecognitionThread(self, manager)

  def requestStart(self):
    pass

  def requestStop(self):
    pass

  def run(self):
    """@reimp"""
    pass

@Q_Q
class _SpeechRecognitionThread:
  def __init__(self, q, manager):
    self.manager = manager # _SpeechRecognitionManager

# EOF
