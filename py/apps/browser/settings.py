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

  def setRubyEnabled(self, value): self.setValue("RubyEnabled", value)
  def isRubyEnabled(self): return to_bool(self.value("RubyEnabled"))

  ttsEnabledChanged = Signal(bool)
  def isTtsEnabled(self): return to_bool(self.value("TtsEnabled"))
  def setTtsEnabled(self, value):
    if value != self.isTtsEnabled():
      self.setValue("TtsEnabled", value)
      self.ttsEnabledChanged.emit(value)

  fullTranslationEnabledChanged = Signal(bool)
  def isFullTranslationEnabled(self): return to_bool(self.value("FullTranslationEnabled"))
  def setFullTranslationEnabled(self, value):
    if value != self.isFullTranslationEnabled():
      self.setValue("FullTranslationEnabled", value)
      self.fullTranslationEnabledChanged.emit(value)

  translationTipEnabledChanged = Signal(bool)
  def isTranslationTipEnabled(self): return to_bool(self.value("TranslationTipEnabled"))
  def setTranslationTipEnabled(self, value):
    if value != self.isTranslationTipEnabled():
      self.setValue("TranslationTipEnabled", value)
      self.translationTipEnabledChanged.emit(value)

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

  def wallpaperUrl(self): return to_unicode(self.value('SpringBoardWallpaperUrl'))

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

  ## Translators

  def jbeijingLocation(self): return to_unicode(self.value('JBeijingLocation'))
  def ezTransLocation(self): return to_unicode(self.value('ezTransLocation'))
  def dreyeLocation(self): return to_unicode(self.value('DreyeLocation'))
  def atlasLocation(self): return to_unicode(self.value('AtlasLocation'))
  def lecLocation(self): return to_unicode(self.value('LecLocation'))

  def isBaiduEnabled(self): return to_bool(self.value('BaiduEnabled'))
  def isBingEnabled(self): return to_bool(self.value('BingEnabled', True)) # only enabled
  def isExciteEnabled(self): return to_bool(self.value('ExciteEnabled'))
  def isGoogleEnabled(self): return to_bool(self.value('GoogleEnabled'))
  def isHanVietEnabled(self): return to_bool(self.value('HanVietEnabled'))
  def isInfoseekEnabled(self): return to_bool(self.value('InfoseekEnabled'))
  def isLecOnlineEnabled(self): return to_bool(self.value('LecOnlineEnabled'))
  def isTransruEnabled(self): return to_bool(self.value('TransruEnabled'))

  def isAtlasEnabled(self): return to_bool(self.value('AtlasEnabled'))
  def isEzTransEnabled(self): return to_bool(self.value('ezTransEnabled'))
  def isDreyeEnabled(self): return to_bool(self.value('DreyeEnabled'))
  def isJBeijingEnabled(self): return to_bool(self.value('JBeijingEnabled'))
  def isLecEnabled(self): return to_bool(self.value('LecEnabled'))

# EOF
