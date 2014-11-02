# coding: utf8
# srman.py
# 11/1/2014 jichi

from functools import partial
from time import time
from PySide.QtCore import Qt, QObject, QThread, Signal
from sakurakit import skthreads
from sakurakit.skclass import memoized, Q_Q
from sakurakit.skdebug import dprint, dwarn
from google import googlesr as sr

@memoized
def manager(): return SpeechRecognitionManager()

class SpeechRecognitionManager(QObject):

  def __init__(self, parent=None):
    super(SpeechRecognitionManager, self).__init__(parent)
    self.__d = _SpeechRecognitionManager(self)

  textReceived = Signal(unicode)

  def start(self): self.__d.thread.requestListen()

  def stop(self):
    if self.__d._thread:
      self.__d._thread.stop()

  def restart(self):
    if self.__d._thread:
      self.__d._thread.restart()
    else:
      self.start()

  def language(self): return self.__d.language
  def setLanguage(self, v):
    if v != self.__d.language:
      self.__d.setLanguage(v)

  def detectsQuiet(self): return self.__d.detectsQuiet
  def setDetectsQuiet(self, t):
    if t != self.__d.detectsQuiet:
      self.__d.setDetectsQuiet(t)

  def isSingleShot(self): return self.__d.singleShot
  def setSingleShot(self, t):
    if t != self.__d.singleShot:
      self.__d.setSingleShot(t)

@Q_Q
class _SpeechRecognitionManager:
  def __init__(self):
    self.detectsQuiet = True # bool
    self.singleShot = True # bool
    self.language = 'ja'
    self._thread = None # SpeechRecognitionThread

  def thread(self): # -> QThread
    if not self._thread:
      t = self._thread = SpeechRecognitionThread()
      t.setLanguage(self.language)
      t.setDetectsQuiet(self.detectsQuiet)
      t.textReceived.connect(self.q, Qt.QueuedConnection)

      from PySide.QtCore import QCoreApplication
      qApp = QCoreApplication.instance()
      qApp.aboutToQuit.connect(t.destroy)

      t.start()
      dprint("create thread")
    return self._thread

  def setLanguage(self, v):
    self.language = v
    if self._thread:
      self._thread.setLanguage(v)

  def setDetectsQuiet(self, t):
    self.detectsQuiet = t
    if self._thread:
      self._thread.setDetectsQuiet(t)

  def setSingleShot(self, t):
    self.singleShot = t
    if self._thread:
      self._thread.setSingleShot(t)

class SpeechRecognitionThread(QThread):
  listenRequested = Signal(float) # time
  textReceived = Signal(unicode) # text

  def __init__(self, parent=None):
    super(SpeechRecognitionThread, self).__init__(parent)
    self.__d = _SpeechRecognitionThread()

  def run(self):
    """@reimp"""
    d = self.__d
    self.listenRequested.connect(d.listen, Qt.QueuedConnection)
    self.exec_()

  def requestListen(self):
    now = time()
    d = self.__d
    d.time = now
    self.listenRequested.emit(now)

  def stop(self):
    d = self.__d
    d.time = time()
    d.recognizer.stopped = True

  def restart(self):
    self.__d.recognizer.stopped = True

  def destroy(self):
    self.__d.recognizer.aborted = True
    self.quit()

  def setLanguage(self, v):
    self.__d.recognizer.language = v

  def setDetectsQuiet(self, t):
    self.__d.recognizer.detects_quiet = t

  def setSingleShot(self, t):
    self.__d.singleShot = t

@Q_Q
class _SpeechRecognitionThread:
  def __init__(self, q):
    self.time = 0 # float
    self.recognizer = sr.Recognizer
    self.device = None # int or None  pyaudio device index
    self.singleShot = True

  def listen(self, time):
    if time < self.time: # aborted
      return
    r = self.recognizer
    while True:
      try:
        with sr.Microphone(device_index=self.device) as source:
          dprint("listen start")
          r.stopped = False
          audio = r.listen(source)
          dprint("listen stop")
      except Exception, e:
        dwarn("audio device error", e)
        return

      if time < self.time: # aborted
        return

      if audio:
        try:
          dprint("recognize start")
          text = skthreads.runsync(partial(r.recognize, audio))
          dprint("recognize stop")

          if time < self.time: # aborted
            return
          if text:
            self.q.textReceived.emit(text)
        except Exception, e:
          dwarn("network error", e)

      if self.singleShot:
        return

# EOF
