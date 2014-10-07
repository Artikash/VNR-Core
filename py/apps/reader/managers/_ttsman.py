# coding: utf8
# _ttsman.py
# 4/7/2013 jichi

import os
from time import time
from functools import partial
from PySide.QtCore import QThread, Signal, Qt
from sakurakit import skfileio, skthreads, skwincom
from sakurakit.sktr import tr_
from zhszht.zhszht import zht2zhs
from mytr import my
import growl, rc

## Voice engines ##

class VoiceEngine(object):
  key = ''          # str
  language = 'ja'   # str
  name = tr_('Unknown') # unicode
  #gender = 'f'  # str
  online = False # bool  whether it is an online engine

  def isValid(self): return True
  def speak(self, text, language=None): pass
  def stop(self): pass

# Offline engines

OfflineEngine = VoiceEngine

class SapiEngine(OfflineEngine):

  def __init__(self, key, speed=0, pitch=0, volume=100):
    #super(SapiEngine, self).__init__()
    self.key = key # str
    self.speed = speed  # int
    self.pitch = pitch  # int
    self.volume = volume  # int
    self._speaking = False
    self._valid = False
    #self.mutex = QMutex() # speak mutex

    import sapi.engine, sapi.registry
    kw = sapi.registry.query(key=self.key)
    self.engine = sapi.engine.SapiEngine(speed=speed, pitch=pitch, volume=volume, **kw) if kw else None
    if self.engine:
      self.language = self.engine.language or 'ja' # override
      self.name = self.engine.name or tr_('Unknown')
      #self.gender = self.engine.gender or 'f'

  def setSpeed(self, v):
    """
    @param  v  int  [-10,10]
    """
    if self.speed != v:
      self.speed = v
      e = self.engine
      if e:
        e.speed = v

  def setPitch(self, v):
    """
    @param  v  int  [-10,10]
    """
    if self.pitch != v:
      self.pitch = v
      e = self.engine
      if e:
        e.pitch = v

  def setVolume(self, v):
    """
    @param  v  int  [0,100]
    """
    if self.volume != v:
      self.volume = v
      e = self.engine
      if e:
        e.volume = v

  def isValid(self):
    """"@reimp"""
    if not self._valid:
      self._valid = bool(self.engine) and self.engine.isValid()
    return self._valid

  #def speak(self, text, language=None):
  #  """@remip"""
  #  skthreads.runasync(partial(self._speakasync, text))

  #def _speakasync(self, text):
  #  e = self.engine
  #  if e:
  #    with SkCoInitializer(threading=True):
  #      if self._speaking:
  #        e.stop()
  #      else:
  #        self._speaking = True
  #      e.speak(text)
  #      #e.speak(text, async=False) # async=False, or it might crash?

  def speak(self, text, language=None):
    """@remip"""
    e = self.engine
    if e:
      if self._speaking:
        e.stop()
      else:
        self._speaking = True
      if language == 'zht':
        text = zht2zhs(text)
      e.speak(text)

  def stop(self):
    """@remip"""
    if self._speaking:
      self._speaking = False
      e = self.engine
      if e:
        e.stop()

# Vocalroids

class VocalroidEngine(OfflineEngine):

  def __init__(self, voiceroid, path=''):
    #super(VocalroidEngine, self).__init__()
    self.path = path # unicode
    self._engine = None # VocalroidController
    self._speaking = False
    self.voiceroid = voiceroid # Voiceroid
    self.key = voiceroid.key
    self.name = voiceroid.name

  def getPath(self):
    return self.path or self.voiceroid.getPath()

  def setPath(self, v):
    if v != self.path:
      self.path = v
      if self._engine:
        self._engine.setPath(path)

  @property
  def engine(self):
    if not self._engine:
      path = self.path or self.voiceroid.getPath()
      if path and os.path.exists(path):
        import voiceroid.bottle
        self._engine = voiceroid.bottle.VoiceroidController(self.voiceroid, path=path)
    return self._engine

  def isValid(self):
    """"@reimp"""
    return bool(self.engine)

  def run(self):
    e = self.engine
    if e:
      growl.msg(' '.join((
        my.tr("Activate Voiceroid+"),
        self.name,
      )))
      e.run()
    else:
      growl.warn(' '.join((
        my.tr("Cannot find Voiceroid+"),
        self.name,
      )))

  def speak(self, text, language=None):
    """@reimp@"""
    e = self.engine
    if e:
      if self._speaking:
        e.stop()
      else:
        self._speaking = True
      e.speak(text)

  def stop(self):
    """@reimp@"""
    if self._speaking:
      self._speaking = False
      e = self.engine
      #if e:
      if e:
        e.stop()

class YukariEngine(VocalroidEngine):
  def __init__(self, **kwargs):
    import voiceroid.apps
    v = voiceroid.apps.Yukari()
    super(YukariEngine, self).__init__(v, **kwargs)

class ZunkoEngine(VocalroidEngine):
  def __init__(self, **kwargs):
    import voiceroid.apps
    v = voiceroid.apps.Zunko()
    super(ZunkoEngine, self).__init__(v, **kwargs)

# Online engines

class _OnlineThread:
  def __init__(self):
    self.playing = False
    self.downloadCount = 0 # int
    self.time = 0 # float

  def init(self, q):
    from pywmp import WindowsMediaPlayer
    self.wmp = WindowsMediaPlayer()

    from qtrequests import qtrequests
    from PySide.QtNetwork import QNetworkAccessManager
    nam = QNetworkAccessManager()
    self.session = qtrequests.Session(nam, q.abortSignal)

    q.abortSignalRequested.connect(q.abortSignal, Qt.QueuedConnection)

    q.playRequested.connect(self.play, Qt.QueuedConnection)
    q.stopRequested.connect(self.stop, Qt.QueuedConnection)

  def play(self, time, url, volume, speed): # float, unicode, int, int ->
    if time < self.time: # outdate
      return
    if not url:
      self.stop()
      return

    path = rc.tts_path(url)
    if os.path.exists(path):
      self._playFile(path, volume, speed)
    else:
      self.downloadCount += 1
      r = self.session.get(url)
      if self.downloadCount > 0:
        self.downloadCount -= 1
      if r and r.content and len(r.content) > 500: # Minimum TTS file size is around 1k for MP3
        if time < self.time: # outdate
          return
        path = rc.tts_path(url)
        if skfileio.writefile(path, r.content, mode='wb'):
          if time < self.time: # outdate
            return
          self._playFile(path, volume, speed)

  def _playFile(self, path, volume, speed): # unicode, int, int
    if volume != 100:
      self.wmp.setVolume(volume)
    if speed:
      speed = pow(1.1, speed)
      self.wmp.setSpeed(speed)
    self.playing = self.wmp.play(path)

  def stop(self, time): # float ->
    if time < self.time: # outdate
      return
    if self.playing:
      self.wmp.stop()
      self.playing = False

class OnlineThread(QThread):
  playRequested = Signal(float, unicode, int, int) # time, url, volume, speed
  stopRequested = Signal(float) # time
  abortSignal = Signal()
  abortSignalRequested = Signal()

  def __init__(self, parent=None):
    super(OnlineThread, self).__init__(parent)
    self.__d = _OnlineThread()

  def run(self):
    """@reimp"""
    skwincom.coinit(threading=True) # critical to use WMP
    self.__d.init(self)
    self.exec_()

  def destroy(self):
    self.abortSignalRequested.emit()
    self.quit()
    #skwincom.couninit() # never invoked

  # Actions

  def requestPlay(self, url, volume=100, speed=0): # unicode ->
    d = self.__d
    now = time()
    d.time = now
    if d.downloadCount > 0:
      self.abortSignalRequested.emit()
    self.playRequested.emit(now, url, volume, speed)

  def requestStop(self):
    d = self.__d
    now = time()
    d.time = now
    if d.downloadCount > 0:
      self.abortSignalRequested.emit()
    if d.playing:
      self.stopRequested.emit(now)

ONLINE_ENGINES = 'google',
class OnlineEngine(VoiceEngine):
  language = '*' # override

  online = True # override

  _thread = None
  @classmethod
  def thread(cls): # -> OnlineThread not None
    if not cls._thread:
      t = cls._thread = OnlineThread()
      t.start()

      from PySide.QtCore import QCoreApplication
      qApp = QCoreApplication.instance()
      qApp.aboutToQuit.connect(t.destroy)

      growl.msg(my.tr("Load {0} for TTS").format("Windows Media Player"))
    return cls._thread

  def __init__(self, volume=100, speed=0):
    #super(OnlineEngine, self).__init__()
    self.speed = speed # int
    self.volume = volume # int

  _valid = None
  def isValid(self):
    """@reimp"""
    if not OnlineEngine._valid:
      import libman
      OnlineEngine._valid = libman.wmp().exists()
    return OnlineEngine._valid

  def speak(self, text, language=None):
    """@reimp@"""
    if not self.isValid():
      growl.warn(my.tr("Missing Windows Media Player needed by text-to-speech"))
    else:
      language = language[:2] if language else 'ja'
      url = self.createUrl(text, language)
      self.thread().requestPlay(url, volume=self.volume, speed=self.speed)

  def stop(self):
    """@reimp@"""
    if self._thread:
      self._thread.requestStop()

  def createUrl(text, language=None): pass # str -> str

class GoogleEngine(OnlineEngine):
  key = 'google' # override
  name = u'Google TTS' # override

  def __init__(self, *args, **kwargs):
    super(GoogleEngine, self).__init__(*args, **kwargs)

  def createUrl(self, text, language=None):
    """@reimp@"""
    from google import googletts
    return googletts.url(text, language, encoding=None) # encoding is not needed

# EOF
