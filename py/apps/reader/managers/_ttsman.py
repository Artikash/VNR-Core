# coding: utf8
# _ttsman.py
# 4/7/2013 jichi

import os
from functools import partial
from PySide.QtCore import QThread, Signal, Qt
from sakurakit import skthreads, skwincom
from sakurakit.sktr import tr_
from zhszht.zhszht import zht2zhs
from mytr import my
import growl

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

  def __init__(self, key, speed=0):
    self.key = key      # str
    self.speed = speed  # int
    self._speaking = False
    self._valid = False
    #self.mutex = QMutex() # speak mutex

    import sapi.engine, sapi.registry
    kw = sapi.registry.query(key=self.key)
    self.engine = sapi.engine.SapiEngine(speed=speed, **kw) if kw else None
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

class OnlineThread(QThread):
  playRequested = Signal(unicode) # url
  stopRequested = Signal()

  def __init__(self, parent=None):
    super(OnlineThread, self).__init__(parent)

    from pywmp import WindowsMediaPlayer
    self._wmp = WindowsMediaPlayer()

    self._playing = False

  def run(self):
    """@reimp"""
    skwincom.coinit(threading=True) # critical to use WMP

    self.playRequested.connect(self.play, Qt.QueuedConnection)
    self.stopRequested.connect(self.stop, Qt.QueuedConnection)

    self.exec_()

  def destroy(self):
    self.quit()
    skwincom.couninit()

  # Actions

  def play(self, url): # unicode ->
    if url:
      self._playing = self._wmp.play(url)
    else:
      self.stop()

  def stop(self):
    if self._playing:
      self._wmp.stop()
      self._playing = False

  def requestPlay(self, url): # unicode ->
    self.playRequested.emit(url)

  def requestStop(self, url): # unicode ->
    if self._playing:
      self.stopRequested.emit()

class OnlineEngine(VoiceEngine):
  language = '' # override

  online = True # override

  _thread = None
  @classmethod
  def thread(cls): # -> OnlineThread not None
    if not cls._thread:
      t = cls._thread = OnlineThread()
      t.start()

      from PySide.QtCore import QCoreApplication
      qApp = QCoreApplication.instance()
      qApp.aboutToQuit.connect(t.quit)

      growl.msg(my.tr("Load {0}").format("Windows Media Player"))
    return cls._thread

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
      self.thread().requestPlay(url)

  def stop(self):
    """@reimp@"""
    if self._thread:
      self._thread.requestStop()

  def createUrl(text, language=None): pass # str -> str

class GoogleEngine(OnlineEngine):
  key = 'google' # override
  name = u'Google TTS' # override

  def createUrl(self, text, language=None):
    """@reimp@"""
    from google import googletts
    return googletts.url(text, language)

# EOF
