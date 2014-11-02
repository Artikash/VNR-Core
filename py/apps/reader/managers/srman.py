# coding: utf8
# srman.py
# 11/1/2014 jichi

from functools import partial
from time import time
from PySide.QtCore import Qt, QObject, QThread, Signal, Property
from sakurakit import skthreads
from sakurakit.skclass import memoized, Q_Q
from sakurakit.skdebug import dprint, dwarn
from google import googlesr as sr

@memoized
def manager(): return SpeechRecognitionManager()

#QmlObject
class SpeechRecognitionBean(QObject):
  def __init__(self, parent=None):
    super(SpeechRecognitionBean, self).__init__(parent)

    m = manager()
    m.activeChanged.connect(self.activeChanged)
    m.singleShotChanged.connect(self.singleShotChanged)

  activeChanged = Signal(bool)
  active = Property(bool,
      lambda _: manager().isActive(),
      lambda _, t: manager().setActive(t),
      notify=activeChanged)

  singleShotChanged = Signal(bool)
  singleShot = Property(bool,
      lambda _: manager().isSingleShot(),
      lambda _, t: manager().setSingleShot(t),
      notify=singleShotChanged)

class SpeechRecognitionManager(QObject):

  def __init__(self, parent=None):
    super(SpeechRecognitionManager, self).__init__(parent)
    self.__d = _SpeechRecognitionManager(self)

  activeChanged = Signal(bool)
  singleShotChanged = Signal(bool)
  textRecognized = Signal(unicode)
  recognitionFinished = Signal()

  def isActive(self): return self.__d.active
  def setActive(self, t):
    if t:
      self.start()
    else:
      self.stop()

  def start(self):
    d = self.__d
    if not d.active:
      d.active = True
      self.activeChanged.emit(True)
      d.thread().requestListen()

  def stop(self):
    d = self.__d
    if d.active:
      d.active = False
      self.activeChanged.emit(False)
      if d._thread:
        d._thread.stop()

  def abort(self):
    d = self.__d
    if d.active:
      d.active = False
      if d._thread:
        d._thread.abort()
      self.activeChanged.emit(False)

  def isOnline(self): return self.__d.online
  def setOnline(self, t):
    if t != self.__d.online:
      self.__d.setOnline(t)

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
      self.singleShotChanged.emit(t)

@Q_Q
class _SpeechRecognitionManager:
  def __init__(self):
    self.active = False
    self.detectsQuiet = True # bool
    self.singleShot = True # bool
    self.online = True # bool
    self.language = 'ja' # str
    self._thread = None # SpeechRecognitionThread

  def thread(self): # -> QThread
    if not self._thread:
      t = self._thread = SpeechRecognitionThread()
      t.setOnline(self.online)
      t.setLanguage(self.language)
      t.setDetectsQuiet(self.detectsQuiet)

      q = self.q
      t.textRecognized.connect(q.textRecognized, Qt.QueuedConnection)
      t.recognitionFinished.connect(q.recognitionFinished, Qt.QueuedConnection)

      from PySide.QtCore import QCoreApplication
      qApp = QCoreApplication.instance()
      qApp.aboutToQuit.connect(t.destroy)

      t.start()
      dprint("create thread")
    return self._thread

  def setOnline(self, t):
    self.online = t
    if self._thread:
      self._thread.setOnline(t)

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
  textRecognized = Signal(unicode) # text
  recognitionFinished = Signal()

  def __init__(self, parent=None):
    super(SpeechRecognitionThread, self).__init__(parent)
    self.__d = _SpeechRecognitionThread(self)

  def run(self):
    """@reimp"""
    d = self.__d
    self.listenRequested.connect(d.listen, Qt.QueuedConnection)
    self.exec_()

  def requestListen(self):
    now = time()
    d = self.__d
    d.time = now
    d.aborted = False
    self.listenRequested.emit(now)

  def stop(self):
    d = self.__d
    #d.time = time()
    d.recognizer.stopped = True

  def abort(self):
    d = self.__d
    d.time = time()
    d.aborted = d.recognizer.stopped = True

  def destroy(self):
    self.abort()
    self.quit()

  def setOnline(self, t):
    self.__d.recognitionEnabled = t

  def setLanguage(self, v):
    self.__d.recognizer.language = v[:2] # trim language

  def setDetectsQuiet(self, t):
    self.__d.recognizer.detects_quiet = t

  def setSingleShot(self, t):
    self.__d.singleShot = t

@Q_Q
class _SpeechRecognitionThread:
  def __init__(self):
    self.time = 0 # float
    self.recognitionEnabled = True
    self.recognizer = sr.Recognizer()
    self.device = None # int or None  pyaudio device index
    self.singleShot = True
    self.aborted = False

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

      if time < self.time or self.aborted: # aborted
        return

      if audio and self.recognitionEnabled:
         skthreads.runasync(partial(self.recognize, audio))

      if time < self.time or self.aborted or self.singleShot:
        return

  def recognize(self, audio):
    """
    @param  audio  googlesr.AudioData
    """
    if time < self.time or self.aborted or not self.recognitionEnabled: # aborted
      return
    q = self.q
    dprint("recognize start")
    try:
      text = self.recognizer.recognize(audio)

      if time < self.time or self.aborted or not self.recognitionEnabled: # aborted
        return
      if text:
        q.textRecognized.emit(text)
    except Exception, e:
      dwarn("network error", e)
    dprint("recognize stop")

    q.recognitionFinished.emit()

# EOF
