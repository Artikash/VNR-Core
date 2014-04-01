# coding: utf8
# settings.py
# 12/13/2012 jichi

from PySide.QtCore import QSettings, Signal
from sakurakit.skclass import memoized
from sakurakit.sktypes import to_int, to_unicode
import config

@memoized
def global_(): return Settings()

@memoized
def reader(): return ReaderSettings()

def to_bool(value): return value == True or value  == 'true'

def to_dict(value):
  """
  @param  dict or None
  @return  dict
  """
  return value if isinstance(value, dict) else {}

class Settings(QSettings):

  def __init__(self):
    super(Settings, self).__init__(
      QSettings.NativeFormat, QSettings.UserScope,
      config.VERSION_DOMAIN, config.VERSION_NAME)

  def setVersion(self, value): self.setValue('Version', value)
  def version(self): return to_int(self.value('Version'))

  def setLanguage(self, value): self.setValue('UserLanguage', value)
  def language(self): return to_unicode(self.value('UserLanguage'))

  def setPlayerActivated(self, value): self.setValue("PlayerActivated", value)
  def isPlayerActivated(self): return to_bool(self.value("PlayerActivated", True))

  def setReaderActivated(self, value): self.setValue("ReaderActivated", value)
  def isReaderActivated(self): return to_bool(self.value("ReaderActivated", True))

  def setMeCabEnabled(self, value): self.setValue("MeCabEnabled", value)
  def isMeCabEnabled(self): return to_bool(self.value("MeCabEnabled"))

  def setTtsEnabled(self, value): self.setValue("TtsEnabled", value)
  def isTtsEnabled(self): return to_bool(self.value("TtsEnabled"))

class ReaderSettings(QSettings):

  def __init__(self):
    super(ReaderSettings, self).__init__(
      QSettings.NativeFormat, QSettings.UserScope,
      config.VERSION_DOMAIN, "reader")

  def version(self): return to_int(self.value('Version'))
  def updateTime(self): return to_int(self.value('UpdateTime'))

  def userLanguage(self): return self.value('UserLanguage')

  def isMainlandChina(self): return to_bool(self.value('MainlandChina'))
  def isCursorThemeEnabled(self): return to_bool(self.value('CursorThemeEnabled', True))

  ## MeCab

  def rubyType(self):
    ret = self.value('FuriganaType', 'hira')
    if ret == 'hiragana':
      ret = 'hira'
    elif ret == 'katagana':
      ret = 'kata'
    return ret

  def meCabDictionary(self): return self.value('MeCabDictionary', '') # str

  ## TTS

  ttsEngineChanged = Signal(unicode)
  def ttsEngine(self): return to_unicode(self.value('TTSEngine'))

  zunkoLocationChanged = Signal(unicode)
  def zunkoLocation(self): return to_unicode(self.value('ZunkoLocation'))

  yukariLocationChanged = Signal(unicode)
  def yukariLocation(self): return to_unicode(self.value('YukariLocation'))

  def sapiSpeeds(self):
    """
    @return  {str ttskey:int speed}
    """
    return to_dict(self.value('SAPISpeeds'))

# EOF
