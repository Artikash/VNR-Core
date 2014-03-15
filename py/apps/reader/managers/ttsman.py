# coding: utf8
# ttsman.py
# 4/7/2013 jichi

__all__ = ['TtsQmlProxy', 'TtsCoffeeProxy']

from functools import partial
from PySide.QtCore import QObject, Slot, QTimer
#from sakurakit import skevents, skthreads
from sakurakit.skdebug import dwarn, dprint
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
#from sakurakit.skqml import QmlObject
from mytr import my
import features, growl, settings, termman
import _ttsman

def _repairtext(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return text.replace(u'…', '.') # てんてんてん

@Q_Q
class _TtsManager(object):

  def __init__(self):
    self.defaultEngineKey = '' # str
    self._online = True     # bool
    self.speakTask = None   # partial function object

    self._googleEngine = None # _ttsman.GoogleEngine
    self._yukariEngine = None # _ttsman.YukariEngine
    self._zunkoEngine = None  # _ttsman.ZunkoEngine
    self._sapiEngines = {}    # {str key:_ttsman.SapiEngine}

    #self.defaultEngineKey = 'wrong engine'
    #self.defaultEngineKey = 'VW Misaki'
    #self.defaultEngineKey = 'VW Show'
    #self.defaultEngineKey = 'zunko'
    #self.defaultEngineKey = 'yukari'
    #self.defaultEngineKey = 'google'

  def iterActiveEngines(self):
    """
    @yield  engine
    """
    for it in self._googleEngine, self._yukariEngine, self._zunkoEngine:
      if it and it.isValid():
        yield it
    for it in self._sapiEngines.itervalues():
      if it.isValid():
        yield it

  def stop(self):
    for it in self.iterActiveEngines():
      it.stop()

  # Google

  @property
  def online(self): return self._online
  @online.setter
  def online(self, v):
    if v != self._online:
      self._online = v
      if self._googleEngine:
        self._googleEngine.setOnline(v)

  @property
  def googleEngine(self):
    if not self._googleEngine:
      ss = settings.global_()
      self._googleEngine = _ttsman.GoogleEngine(
          online=self._online,
          language=ss.googleTtsLanguage())
      ss.googleTtsLanguageChanged.connect(self._googleEngine.setLanguage)
    return self._googleEngine

  # Voiceroid

  @property
  def yukariEngine(self):
    if not self._yukariEngine:
      ss = settings.global_()
      eng = self._yukariEngine = _ttsman.YukariEngine(path=ss.yukariLocation())
      ss.yukariLocationChanged.connect(eng.setPath)
      growl.msg(' '.join((
        my.tr("Load TTS"),
        eng.name,
      )))
    return self._yukariEngine

  @property
  def zunkoEngine(self):
    if not self._zunkoEngine:
      ss = settings.global_()
      eng = self._zunkoEngine = _ttsman.ZunkoEngine(path=ss.zunkoLocation())
      ss.zunkoLocationChanged.connect(eng.setPath)
      growl.msg(' '.join((
        my.tr("Load TTS"),
        eng.name,
      )))
    return self._zunkoEngine

  # SAPI

  def getSapiEngine(self, key):
    ret = self._sapiEngines.get(key)
    if not ret:
      speed = self.getSapiSpeed(key)
      ret = _ttsman.SapiEngine(key=key, speed=speed)
      if ret.isValid():
        growl.msg(' '.join((
          my.tr("Load TTS"),
          ret.name,
        )))
        self._sapiEngines[key] = ret
      else:
        growl.warn(' '.join((
          my.tr("Failed to load TTS"),
          key,
        )))
        ret = None
    return ret

  def getSapiSpeed(self, key):
    """
    @param  key  str
    @return  int
    """
    try: return int(settings.global_().sapiSpeeds().get(key) or 0)
    except (ValueError, TypeError): return 0

  def setSapiSpeed(self, key, v):
    """
    @param  key  str
    @param  v  int
    """
    ss = settings.global_()
    m = ss.sapiSpeeds()
    if v != m.get(key):
      m[key] = v
      ss.setSapiSpeeds(m)
      eng = self._sapiEngines.get(key)
      if eng:
        eng.setSpeed(v)

  # Actions

  def getEngine(self, key):
    """
    @return  _ttsman.VoiceEngine or None
    """
    if key == 'zunko':
      return self.zunkoEngine
    if key == 'yukari':
      return self.yukariEngine
    if key == 'google':
      return self.googleEngine
    return self.getSapiEngine(key)

  @memoizedproperty
  def speakTimer(self):
    ret = QTimer(self.q)
    ret.setSingleShot(True)
    ret.timeout.connect(self._doSpeakTask)
    return ret

  def _doSpeakTask(self):
    try: apply(self.speakTask)
    except Exception, e: dwarn(e)

class TtsManager(QObject):

  def __init__(self, parent=None):
    super(TtsManager, self).__init__(parent)
    d = self.__d = _TtsManager(self)

  def isOnline(self): return self.__d.online
  def setOnline(self, v): self.__d.online = v

  def stop(self):
    if not features.TEXT_TO_SPEECH:
      return
    self.__d.stop()

  def warmup(self):
    d = self.__d
    if d.defaultEngineKey == 'google':
      dprint("enter")
      d.googleEngine.warmup()
      dprint("leave")

  def defaultEngine(self): return self.__d.defaultEngineKey
  def setDefaultEngine(self, key):
    """
    @param  key  unicode
    """
    d = self.__d
    if d.defaultEngineKey != key:
      d.defaultEngineKey = key
      settings.global_().setTtsEngine(key)

  def getSapiSpeed(self, v):
    return self.__d.getSapiSpeed(v)
  def setSapiSpeed(self, key, v):
    """
    @param  value  int in [-10,10]
    """
    self.__d.setSapiSpeed(key, v)

  def speak(self, text, interval=100, **kwargs):
    if not features.TEXT_TO_SPEECH:
      return
    d = self.__d
    d.speakTask = partial(self._speak, text, **kwargs)
    d.speakTimer.start(interval)

  def _speak(self, text, engine='', termEnabled=False, language='', verbose=True):
    """
    @param  text  unicode
    @param* engine  str
    @param* termEnabled  bool  whether apply game-specific terms
    @param* language  unicode
    @param* verbose  bool  whether warn on error
    """
    #if not features.TEXT_TO_SPEECH or not text:
    if not text:
      return

    d = self.__d

    eng = d.getEngine(engine) if engine else None
    if not eng and d.defaultEngineKey and d.defaultEngineKey != engine:
      eng = d.getEngine(d.defaultEngineKey)

    if not eng:
      if verbose:
        growl.warn(my.tr("TTS is not available in Preferences"))
      dprint("missing engine: %s" % (engine or d.defaultEngineKey))
      return
    if not eng.isValid():
      #if verbose:
      # Always warn
      growl.warn('<br/>'.join((
        my.tr("TTS is not available"),
        eng.name,
      )))
      dprint("invalid engine: %s" % (eng.key))
      return
    if language and language != eng.language:
      dprint("language mismatch: %s != %s" % (language, eng.language))
      return

    if termEnabled and (not language or language == 'ja'):
      text = termman.manager().applySpeechTerms(text)
    # Even if text is empty, trigger stop tts
    #if not text:
    #  return
    text = _repairtext(text)
    eng.speak(text)

    #skevents.runlater(partial(eng.speak, text))

  def yukariLocation(self): return self.__d.yukariEngine.getPath()
  def zunkoLocation(self): return self.__d.zunkoEngine.getPath()

  def runYukari(self): return self.__d.yukariEngine.run()
  def runZunko(self): return self.__d.zunkoEngine.run()

  def availableEngines(self):
    """
    @return  [unicode]
    """
    ret = ['google'] # warmup google take times, so, always available
    d = self.__d
    for it in d.yukariEngine, d.zunkoEngine:
      if it.isValid():
        ret.append(it.key)
    import sapiman
    ret.extend(it.key for it in sapiman.voices())
    return ret

@memoized
def manager(): return TtsManager()

def speak(*args, **kwargs): manager().speak(*args, **kwargs)
def warmup(): manager().warmup()
def stop(): manager().stop()

## Beans ##

#@QmlObject
class TtsQmlProxy(QObject):
  def __init__(self, parent=None):
    super(TtsQmlProxy, self).__init__(parent)

  @Slot(unicode, unicode)
  def speak(self, text, language):
    speak(text, language=language, termEnabled=False, verbose=False)
    dprint("pass")

  @Slot(result=unicode)
  def availableEngines(self):
    """
    @return  unicode  csv
    """
    return ',' + ','.join(manager().availableEngines())
    #return ',google,yukari,zunko,VW Misaki'

class TtsCoffeeProxy(QObject):
  def __init__(self, parent=None):
    super(TtsCoffeeProxy, self).__init__(parent)

  @Slot(unicode)
  def speak(self, text):
    speak(text, termEnabled=False, verbose=False)
    dprint("pass")

  #@Slot()
  #def stop(self): stop()

# EOF
