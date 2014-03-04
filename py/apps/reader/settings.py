# coding: utf8
# settings.py
# 10/28/2012 jichi

from PySide.QtCore import Signal, Slot, Property, Qt, QObject, QSettings, QTimer, QSize
from sakurakit.skclass import memoized, memoizedproperty
from sakurakit.skqml import QmlObject
from sakurakit.sktypes import to_int, to_unicode #to_long
import defs, config

__all__ = ['SettingsProxy']

def to_bool(value):
  return value == True or value  == 'true'

#def to_str(value):
#  return value if isinstance(value, str) else '' if value is None else "%s" % value

def to_float(value, default=0.0):
  try: return float(value)
  except (ValueError, TypeError): return default

def to_size(value):
  """
  @param  value  QSize or None
  @return  (int w, int h)
  """
  try: return value.width(), value.height()
  except AttributeError: return 0,0

def to_set(value):
  """
  @param  value [list] or None
  @return  set
  """
  try: return value if isinstance(value, set) else set(value) if value is not None else set()
  except TypeError: return set()

def to_dict(value):
  """
  @param  dict or None
  @return  dict
  """
  return value if isinstance(value, dict) else {}

class _Settings:

  def __init__(self, q):
    t = self.syncTimer = QTimer(q)
    t.setInterval(config.APP_SAVE_SETTINGS_INTERVAL)
    t.timeout.connect(q.sync)

class Settings(QSettings):

  def __init__(self):
    super(Settings, self).__init__(
      QSettings.NativeFormat, QSettings.UserScope,
      config.VERSION_DOMAIN, config.VERSION_NAME)
    self.__d = _Settings(self)

  def autoSync(self): self.__d.syncTimer.start()

  def setVersion(self, value): self.setValue('Version', value)
  def version(self): return to_int(self.value('Version'))

  def setUpdateTime(self, value): self.setValue('UpdateTime', value)
  def updateTime(self): return to_int(self.value('UpdateTime'))

  def setGreetingTime(self, value): self.setValue('GreetingTime', value)
  def greetingTime(self): return to_int(self.value('GreetingTime'))

  def setGameFilesTime(self, value): self.setValue('GameFilesTime', value)
  def gameFilesTime(self): return to_int(self.value('GameFilesTime'))

  def setUserDigestsTime(self, value): self.setValue('UserDigestsTime', value)
  def userDigestsTime(self): return to_int(self.value('UserDigestsTime'))

  def setTermsTime(self, value): self.setValue('TermsTime', value)
  def termsTime(self): return to_int(self.value('TermsTime'))

  ## Window sizes ##

  def setSpringBoardSize(self, value): self.setValue('SpringBoardSize', value)
  def springBoardSize(self): return to_size(self.value('SpringBoardSize'))

  ## Spring board ##

  def setSpringBoardLaunchesGame(self, value): self.setValue('SpringBoardLaunch', value)
  def springBoardLaunchesGame(self): return to_bool(self.value('SpringBoardLaunch', False))

  ## Kagami ##

  kagamiFocusEnabledChanged = Signal(bool)
  def isKagamiFocusEnabled(self): return to_bool(self.value('KagamiFocusEnabled', False))
  def setKagamiFocusEnabled(self, value):
    if value != self.isKagamiFocusEnabled():
      self.setValue('KagamiFocusEnabled', value)
      self.kagamiFocusEnabledChanged.emit(value)

  hentaiChanged = Signal(bool)
  def isHentai(self): return to_bool(self.value('Hentai', False))
  #def setHentai(self, value):
  #  if value != self.isHentai():
  #    self.setValue('Hentai', value)
  #    self.hentaiChanged.emit(value)

  ## User profile ##

  userIdChanged = Signal(int)
  def userId(self): return to_int(self.value('UserId'))
  def setUserId(self, value):
    if value != self.userId():
      self.setValue('UserId', value)
      self.userIdChanged.emit(value)

  def setUserName(self, value):
    self.setValue('UserName', value)
  def userName(self): return self.value('UserName')

  # TODO: encrypt password
  def setUserPassword(self, value): self.setValue('UserPassword', value)
  def userPassword(self): return self.value('UserPassword')

  loginChanged = Signal(unicode, unicode) # username, password
  def invalidateLogin(self):
    self.loginChanged.emit(self.userName(), self.userPassword())

  userLanguageChanged = Signal(str)
  def userLanguage(self):
    return self.value('UserLanguage')
  def setUserLanguage(self, value):
    if value != self.userLanguage():
      self.setValue('UserLanguage', value)
      self.userLanguageChanged.emit(value)

  userGenderChanged = Signal(str)
  def userGender(self):
    return self.value('UserGender')
  def setUserGender(self, value):
    if value != self.userGender():
      self.setValue('UserGender', value)
      self.userGenderChanged.emit(value)

  userCommentLevelChanged = Signal(int)
  def userCommentLevel(self): return to_int(self.value('UserCommentLevel'))
  def setUserCommentLevel(self, value):
    if value != self.userCommentLevel():
      self.setValue('UserCommentLevel', value)
      self.userCommentLevelChanged.emit(value)

  userTermLevelChanged = Signal(int)
  def userTermLevel(self): return to_int(self.value('UserTermLevel'))
  def setUserTermLevel(self, value):
    if value != self.userTermLevel():
      self.setValue('UserTermLevel', value)
      self.userTermLevelChanged.emit(value)

  userColorChanged = Signal(unicode)
  def userColor(self): return self.value('UserColor')
  def setUserColor(self, value):
    if value != self.userColor():
      self.setValue('UserColor', value)
      self.userColorChanged.emit(value)

  userAvatarChanged = Signal(unicode)
  def userAvatar(self): return self.value('UserAvatar')
  def setUserAvatar(self, value):
    if value != self.userAvatar():
      self.setValue('UserAvatar', value)
      self.userAvatarChanged.emit(value)

  def setUserHomepage(self, value): self.setValue('UserHomepage', value)
  def userHomepage(self): return self.value('UserHomepage')

  ## PRC fixes, fxxk CCP ##

  mainlandChinaChanged = Signal(bool)
  def isMainlandChina(self): return to_bool(self.value('MainlandChina'))
  def setMainlandChina(self, value):
    if value != self.isMainlandChina():
      self.setValue('MainlandChina', value)
      self.mainlandChinaChanged.emit(value)

  ## i18n ##

  blockedLanguagesChanged = Signal(set)
  def blockedLanguages(self):
    return to_set(self.value('BlockedLanguages'))
  def setBlockedLanguages(self, value):
    if value != self.blockedLanguages():
      self.setValue('BlockedLanguages', value)
      self.blockedLanguagesChanged.emit(value)

  ## AppLocale ##

  #applocEnabledChanged = Signal(bool)
  #def isApplocEnabled(self):
  #  return to_bool(self.value('ApplocEnabled', True))
  #def setApplocEnabled(self, value):
  #  if value != self.isApplocEnabled():
  #    self.setValue('ApplocEnabled', value)
  #    self.applocEnabledChanged.emit(value)
  def isApplocEnabled(self):
    return to_bool(self.value('ApplocEnabled', True))
  def setApplocEnabled(self, value):
    self.setValue('ApplocEnabled', value)

  #timeZoneEnabledChanged = Signal(bool)
  def isTimeZoneEnabled(self):
    return to_bool(self.value('TimeZoneEnabled'))
  def setTimeZoneEnabled(self, value):
    self.setValue('TimeZoneEnabled', value)
  #  if value != self.isTimeZoneEnabled():
  #    self.setValue('TimeZoneEnabled', value)
  #    self.timeZoneEnabledChanged.emit(value)
  #def isTimeZoneEnabled(self): return True

  def isLocaleSwitchEnabled(self):
    return to_bool(self.value('LocaleSwitchEnabled'))
  def setLocaleSwitchEnabled(self, value):
    self.setValue('LocaleSwitchEnabled', value)

  def isNtleaEnabled(self):
    return to_bool(self.value('NtleaEnabled'))
  def setNtleaEnabled(self, value):
    self.setValue('NtleaEnabled', value)

  #def ntleaLocation(self):
  #  return to_unicode(self.value('NtleaLocation'))
  #def setNtleaLocation(self, value):
  #  self.setValue('NtleaLocation', value)

  def isLocaleEmulatorEnabled(self):
    return to_bool(self.value('LocaleEmulatorEnabled'))
  def setLocaleEmulatorEnabled(self, value):
    self.setValue('LocaleEmulatorEnabled', value)

  def localeEmulatorLocation(self):
    return to_unicode(self.value('LocaleEmulatorLocation'))
  def setLocaleEmulatorLocation(self, value):
    self.setValue('LocaleEmulatorLocation', value)

  ## QuickTime ##

  #def isQuickTimeEnabled(self): return to_bool(self.value('QuickTimeEnabled', True))
  #def setQuickTimeEnabled(self, value): self.setValue('QuickTimeEnabled', value)

  ## UI ##

  #def kagamiIgnoresFocus(self):
  #  return to_bool(self.value('KagamiIgnoresFocus', True))

  cursorThemeEnabledChanged = Signal(bool)
  def isCursorThemeEnabled(self):
    return to_bool(self.value('CursorThemeEnabled', True))
  def setCursorThemeEnabled(self, value):
    if value != self.isCursorThemeEnabled():
      self.setValue('CursorThemeEnabled', value)
      self.cursorThemeEnabledChanged.emit(value)

      import curtheme
      if value: curtheme.load()
      else: curtheme.unload()

  springBoardWallpaperUrlChanged = Signal(unicode)
  def springBoardWallpaperUrl(self):
    return to_unicode(self.value('SpringBoardWallpaperUrl'))
  def setSpringBoardWallpaperUrl(self, value):
    if value != self.springBoardWallpaperUrl():
      self.setValue('SpringBoardWallpaperUrl', value)
      self.springBoardWallpaperUrlChanged.emit(value)

  springBoardSlidesEnabledChanged = Signal(bool)
  def isSpringBoardSlidesEnabled(self):
    return to_bool(self.value('SpringBoardSlidesEnabled', True))
  def setSpringBoardSlidesEnabled(self, value):
    if value != self.isSpringBoardSlidesEnabled():
      self.setValue('SpringBoardSlidesEnabled', value)
      self.springBoardSlidesEnabledChanged.emit(value)

  grimoireFontColorChanged = Signal(str)
  def grimoireFontColor(self):
    return self.value('GrimoireFontColor', config.SETTINGS_FONT_COLOR)
  def setGrimoireFontColor(self, value):
    value = value or config.SETTINGS_SHADOW_COLOR
    if value != self.grimoireFontColor():
      self.setValue('GrimoireFontColor', value)
      self.grimoireFontColorChanged.emit(value)

  grimoireShadowColorChanged = Signal(str)
  def grimoireShadowColor(self):
    return self.value('GrimoireShadowColor', config.SETTINGS_SHADOW_COLOR)
  def setGrimoireShadowColor(self, value):
    value = value or config.SETTINGS_SHADOW_COLOR
    if value != self.grimoireShadowColor():
      self.setValue('GrimoireShadowColor', value)
      self.grimoireShadowColorChanged.emit(value)

  grimoireTextColorChanged = Signal(str)
  def grimoireTextColor(self):
    return self.value('GrimoireTextColor', config.SETTINGS_TEXT_COLOR)
  def setGrimoireTextColor(self, value):
    value = value or config.SETTINGS_TEXT_COLOR
    if value != self.grimoireTextColor():
      self.setValue('GrimoireTextColor', value)
      self.grimoireTextColorChanged.emit(value)

  #grimoireTranslationColorChanged = Signal(str)
  #def grimoireTranslationColor(self):
  #  return self.value('GrimoireTranslationColor', config.SETTINGS_TRANSLATION_COLOR)
  #def setGrimoireTranslationColor(self, value):
  #  value = value or config.SETTINGS_TRANSLATION_COLOR
  #  if value != self.grimoireTranslationColor():
  #    self.setValue('GrimoireTranslationColor', value)
  #    self.grimoireTranslationColorChanged.emit(value)

  grimoireSubtitleColorChanged = Signal(str)
  def grimoireSubtitleColor(self):
    return self.value('GrimoireSubtitleColor', config.SETTINGS_SUBTITLE_COLOR)
  def setGrimoireSubtitleColor(self, value):
    value = value or config.SETTINGS_SUBTITLE_COLOR
    if value != self.grimoireSubtitleColor():
      self.setValue('GrimoireSubtitleColor', value)
      self.grimoireSubtitleColorChanged.emit(value)

  grimoireCommentColorChanged = Signal(str)
  def grimoireCommentColor(self):
    return self.value('GrimoireCommentColor', config.SETTINGS_COMMENT_COLOR)
  def setGrimoireCommentColor(self, value):
    value = value or config.SETTINGS_COMMENT_COLOR
    if value != self.grimoireCommentColor():
      self.setValue('GrimoireCommentColor', value)
      self.grimoireCommentColorChanged.emit(value)

  grimoireDanmakuColorChanged = Signal(str)
  def grimoireDanmakuColor(self):
    return self.value('GrimoireDanmakuColor', config.SETTINGS_DANMAKU_COLOR)
  def setGrimoireDanmakuColor(self, value):
    value = value or config.SETTINGS_DANMAKU_COLOR
    if value != self.grimoireDanmakuColor():
      self.setValue('GrimoireDanmakuColor', value)
      self.grimoireDanmakuColorChanged.emit(value)

  infoseekColorChanged = Signal(str)
  def infoseekColor(self):
    return self.value('InfoseekColor', config.SETTINGS_INFOSEEK_COLOR)
  def setInfoseekColor(self, value):
    value = value or config.SETTINGS_INFOSEEK_COLOR
    if value != self.infoseekColor():
      self.setValue('InfoseekColor', value)
      self.infoseekColorChanged.emit(value)

  exciteColorChanged = Signal(str)
  def exciteColor(self):
    return self.value('ExciteColor', config.SETTINGS_EXCITE_COLOR)
  def setExciteColor(self, value):
    value = value or config.SETTINGS_EXCITE_COLOR
    if value != self.exciteColor():
      self.setValue('ExciteColor', value)
      self.exciteColorChanged.emit(value)

  googleColorChanged = Signal(str)
  def googleColor(self):
    return self.value('GoogleColor', config.SETTINGS_GOOGLE_COLOR)
  def setGoogleColor(self, value):
    value = value or config.SETTINGS_GOOGLE_COLOR
    if value != self.googleColor():
      self.setValue('GoogleColor', value)
      self.googleColorChanged.emit(value)

  bingColorChanged = Signal(str)
  def bingColor(self):
    return self.value('BingColor', config.SETTINGS_BING_COLOR)
  def setBingColor(self, value):
    value = value or config.SETTINGS_BING_COLOR
    if value != self.bingColor():
      self.setValue('BingColor', value)
      self.bingColorChanged.emit(value)

  baiduColorChanged = Signal(str)
  def baiduColor(self):
    return self.value('BaiduColor', config.SETTINGS_BAIDU_COLOR)
  def setBaiduColor(self, value):
    value = value or config.SETTINGS_BAIDU_COLOR
    if value != self.baiduColor():
      self.setValue('BaiduColor', value)
      self.baiduColorChanged.emit(value)

  lougoColorChanged = Signal(str)
  def lougoColor(self):
    return self.value('LougoColor', config.SETTINGS_LOUGO_COLOR)
  def setLougoColor(self, value):
    value = value or config.SETTINGS_LOUGO_COLOR
    if value != self.lougoColor():
      self.setValue('LougoColor', value)
      self.lougoColorChanged.emit(value)

  jbeijingColorChanged = Signal(str)
  def jbeijingColor(self):
    return self.value('JBeijingColor', config.SETTINGS_JBEIJING_COLOR)
  def setJBeijingColor(self, value):
    value = value or config.SETTINGS_JBEIJING_COLOR
    if value != self.jbeijingColor():
      self.setValue('JBeijingColor', value)
      self.jbeijingColorChanged.emit(value)

  dreyeColorChanged = Signal(str)
  def dreyeColor(self):
    return self.value('DreyeColor', config.SETTINGS_DREYE_COLOR)
  def setDreyeColor(self, value):
    value = value or config.SETTINGS_DREYE_COLOR
    if value != self.dreyeColor():
      self.setValue('DreyeColor', value)
      self.dreyeColorChanged.emit(value)

  ezTransColorChanged = Signal(str)
  def ezTransColor(self):
    return self.value('EzTransColor', config.SETTINGS_EZTRANS_COLOR)
  def setEzTransColor(self, value):
    value = value or config.SETTINGS_EZTRANS_COLOR
    if value != self.ezTransColor():
      self.setValue('EzTransColor', value)
      self.ezTransColorChanged.emit(value)

  atlasColorChanged = Signal(str)
  def atlasColor(self):
    return self.value('AtlasColor', config.SETTINGS_ATLAS_COLOR)
  def setAtlasColor(self, value):
    value = value or config.SETTINGS_ATLAS_COLOR
    if value != self.atlasColor():
      self.setValue('AtlasColor', value)
      self.atlasColorChanged.emit(value)

  lecColorChanged = Signal(str)
  def lecColor(self):
    return self.value('LecColor', config.SETTINGS_LEC_COLOR)
  def setLecColor(self, value):
    value = value or config.SETTINGS_LEC_COLOR
    if value != self.lecColor():
      self.setValue('LecColor', value)
      self.lecColorChanged.emit(value)

  ## Plugin ##

  def isPyPluginEnabled(self): return to_bool(self.value('PyPluginEnabled'))
  def setPyPluginEnabled(self, value): self.setValue('PyPluginEnabled', value)

  def isJsPluginEnabled(self): return to_bool(self.value('JsPluginEnabled'))
  def setJsPluginEnabled(self, value): self.setValue('JsPluginEnabled', value)

  ## Features ##

  internetConnectionChanged = Signal(str)
  def internetConnection(self): return self.value('InternetConnection', defs.INTERNET_CONNECTION_AUTO)
  def setInternetConnection(self, value):
    if value != self.internetConnection():
      self.setValue('InternetConnection', value)
      self.internetConnectionChanged.emit(value)

  #allowsInternetAccessChanged = Signal(bool)
  #def allowsInternetAccess(self): return to_bool(self.value('InternetEnabled', True))
  #def setAllowsInternetAccess(self, value):
  #  if value != self.allowsInternetAccess():
  #    self.setValue('InternetEnabled', value)
  #    self.allowsInternetAccessChanged.emit(value)

  allowsMachineTranslationChanged = Signal(bool)
  def allowsMachineTranslation(self): return to_bool(self.value('MachineTranslationEnabled', True))
  def setAllowsMachineTranslation(self, value):
    if value != self.allowsMachineTranslation():
      self.setValue('MachineTranslationEnabled', value)
      self.allowsMachineTranslationChanged.emit(value)

  allowsUserCommentChanged = Signal(bool)
  def allowsUserComment(self): return to_bool(self.value('UserCommentEnabled', True))
  def setAllowsUserComment(self, value):
    if value != self.allowsUserComment():
      self.setValue('UserCommentEnabled', value)
      self.allowsUserCommentChanged.emit(value)

  allowsTextToSpeechChanged = Signal(bool)
  def allowsTextToSpeech(self): return to_bool(self.value('TextToSpeechEnabled', True))
  def setAllowsTextToSpeech(self, value):
    if value != self.allowsTextToSpeech():
      self.setValue('TextToSpeechEnabled', value)
      self.allowsTextToSpeechChanged.emit(value)

  ## TTS ##

  speaksGameTextChanged = Signal(bool)
  def speaksGameText(self):
    return to_bool(self.value('SpeakGameText'))
  def setSpeaksGameText(self, value):
    if value != self.speaksGameText():
      self.setValue('SpeakGameText', value)
      self.speaksGameTextChanged.emit(value)

  voiceCharacterEnabledChanged = Signal(bool)
  def isVoiceCharacterEnabled(self):
    return to_bool(self.value('VoiceCharacter', True))
  def setVoiceCharacterEnabled(self, value):
    if value != self.isVoiceCharacterEnabled():
      self.setValue('VoiceCharacter', value)
      self.voiceCharacterEnabledChanged.emit(value)

  #def isSubtitleVoiceEnabled(self): return to_bool(self.value('SubtitleVoice'))

  #googleTtsEnabledChanged = Signal(bool)
  #def isGoogleTtsEnabled(self):
  #  return to_bool(self.value('GoogleTTSEnabled'))
  #def setGoogleTtsEnabled(self, value):
  #  if value != self.isGoogleTtsEnabled():
  #    self.setValue('GoogleTTSEnabled', value)
  #    self.googleTtsEnabledChanged.emit(value)

  #windowsTtsSpeedChanged = Signal(int)
  #def windowsTtsSpeed(self):
  #  return to_int(self.value('WindowsTTSSpeed', 0))
  #def setWindowsTtsSpeed(self, value):
  #  if value != self.windowsTtsSpeed():
  #    self.setValue('WindowsTTSSpeed', value)
  #    self.windowsTtsSpeedChanged.emit(value)

  #windowsTtsEnabledChanged = Signal(bool)
  #def isWindowsTtsEnabled(self):
  #  return to_bool(self.value('WindowsTTSEnabled'))
  #def setWindowsTtsEnabled(self, value):
  #  if value != self.isWindowsTtsEnabled():
  #    self.setValue('WindowsTTSEnabled', value)
  #    self.windowsTtsEnabledChanged.emit(value)

  #windowsTtsEngineChanged = Signal(unicode)
  #def windowsTtsEngine(self):
  #  return to_unicode(self.value('WindowsTTS'))
  #def setWindowsTtsEngine(self, value):
  #  if value != self.windowsTtsEngine():
  #    self.setValue('WindowsTTS', value)
  #    self.windowsTtsEngineChanged.emit(value)

  #maleVoiceJaChanged = Signal(unicode)
  #def maleVoiceJa(self):
  #  return to_unicode(self.value('MaleVoiceJa'))
  #def setMaleVoiceJa(self, value):
  #  if value != self.maleVoiceJa():
  #    self.setValue('MaleVoiceJa', value)
  #    self.maleVoiceJaChanged.emit(value)

  #femaleVoiceJaChanged = Signal(unicode)
  #def femaleVoiceJa(self):
  #  return to_unicode(self.value('FemaleVoiceJa'))
  #def setFemaleVoiceJa(self, value):
  #  if value != self.femaleVoiceJa():
  #    self.setValue('FemaleVoiceJa', value)
  #    self.femaleVoiceJaChanged.emit(value)

  #maleVoiceChanged = Signal(unicode)
  #def maleVoice(self):
  #  return to_unicode(self.value('MaleVoice'))
  #def setMaleVoice(self, value):
  #  if value != self.maleVoice():
  #    self.setValue('MaleVoice', value)
  #    self.maleVoiceChanged.emit(value)

  #femaleVoiceChanged = Signal(unicode)
  #def femaleVoice(self):
  #  return to_unicode(self.value('FemaleVoice'))
  #def setFemaleVoice(self, value):
  #  if value != self.femaleVoice():
  #    self.setValue('FemaleVoice', value)
  #    self.femaleVoiceChanged.emit(value)

  ttsEngineChanged = Signal(unicode)
  def ttsEngine(self):
    return to_unicode(self.value('TTSEngine'))
  def setTtsEngine(self, value):
    if value != self.ttsEngine():
      self.setValue('TTSEngine', value)
      self.ttsEngineChanged.emit(value)

  googleTtsLanguageChanged = Signal(str)
  def googleTtsLanguage(self):
    return self.value('GoogleTTSLanguage', 'ja')
  def setGoogleTtsLanguage(self, value):
    if value != self.googleTtsLanguage():
      self.setValue('GoogleTTSLanguage', value)
      self.googleTtsLanguageChanged.emit(value)

  zunkoLocationChanged = Signal(unicode)
  def zunkoLocation(self):
    return to_unicode(self.value('ZunkoLocation'))
  def setZunkoLocation(self, value):
    if value != self.zunkoLocation():
      self.setValue('ZunkoLocation', value)
      self.zunkoLocationChanged.emit(value)

  yukariLocationChanged = Signal(unicode)
  def yukariLocation(self):
    return to_unicode(self.value('YukariLocation'))
  def setYukariLocation(self, value):
    if value != self.yukariLocation():
      self.setValue('YukariLocation', value)
      self.yukariLocationChanged.emit(value)

  def sapiSpeeds(self):
    """
    @return  {str ttskey:int speed}
    """
    return to_dict(self.value('SAPISpeeds'))
  def setSapiSpeeds(self, value): self.setValue('SAPISpeeds', value)

  ## Game launcher ##

  def isGameDetectionEnabled(self): return to_bool(self.value('GameDetectionEnabled', True))
  def setGameDetectionEnabled(self, value): self.setValue('GameDetectionEnabled', value)

  def isHookCodeEnabled(self):
    #return to_bool(self.value('HookCodeEnabled', True))
    return True # Always enable it
  def setHookCodeEnabled(self, value): self.setValue('HookCodeEnabled', value)

  # Max length of the hooked game text
  gameTextCapacityChanged = Signal(int)
  def gameTextCapacity(self):
    return max(20,
        to_int(self.value('GameTextCapacity', config.SETTINGS_TEXT_CAPACITY)))

  #  - Dictionaries -

  meCabEnabledChanged = Signal(bool)
  def isMeCabEnabled(self): return bool(self.meCabDictionary())

  # {ipadic, unidic, unidic-mlj}
  meCabDictionaryChanged = Signal(str)
  def meCabDictionary(self): return self.value('MeCabDictionary', '')
  def setMeCabDictionary(self, v):
    if v != self.meCabDictionary():
      self.setValue('MeCabDictionary', v)
      self.meCabEnabledChanged.emit(bool(v)) # This must before mecab dictionary is set
      self.meCabDictionaryChanged.emit(v)

  def isEdictEnabled(self): return to_bool(self.value('EdictEnabled'))
  def setEdictEnabled(self, v):
    if v != self.isEdictEnabled():
      self.setValue('EdictEnabled', v)
      self._updateDictionaryEnabled()

  def isKojienEnabled(self): return to_bool(self.value('KojienEnabled'))
  def setKojienEnabled(self, v):
    if v != self.isKojienEnabled():
      self.setValue('KojienEnabled', v)
      self._updateDictionaryEnabled()

  def isWadokuEnabled(self): return to_bool(self.value('WadokuEnabled'))
  def setWadokuEnabled(self, v):
    if v != self.isWadokuEnabled():
      self.setValue('WadokuEnabled', v)
      self._updateDictionaryEnabled()

  def isZhongriEnabled(self): return to_bool(self.value('ZhongriEnabled'))
  def setZhongriEnabled(self, v):
    if v != self.isZhongriEnabled():
      self.setValue('ZhongriEnabled', v)
      self._updateDictionaryEnabled()

  dictionaryEnabledChanged = Signal(bool)
  def isDictionaryEnabled(self):
    return (
        self.isEdictEnabled() or
        self.isZhongriEnabled() or
        self.isKojienEnabled() or
        self.isWadokuEnabled() or
        self.isJMDictFrEnabled() or
        self.isJMDictRuEnabled() or
        self.isJMDictNlEnabled() or
        self.isLingoesJaZhEnabled() or
        self.isLingoesJaKoEnabled() or
        self.isLingoesJaViEnabled() or
        self.isLingoesJaEnEnabled())
  def _updateDictionaryEnabled(self):
    self.dictionaryEnabledChanged.emit(self.isDictionaryEnabled())

  def kojienLocation(self): return to_unicode(self.value('KojienLocation'))
  def setKojienLocation(self, v): self.setValue('KojienLocation', v)

  #def wadokuLocation(self): return to_unicode(self.value('WadokuLocation'))
  #def setWadokuLocation(self, v): self.setValue('WadokuLocation', v)

  def zhongriLocation(self): return to_unicode(self.value('ZhongriLocation'))
  def setZhongriLocation(self, v): self.setValue('ZhongriLocation', v)

  def isKojienEnabled(self): return to_bool(self.value('KojienEnabled'))
  def setKojienEnabled(self, v):
    if v != self.isKojienEnabled():
      self.setValue('KojienEnabled', v)
      self._updateDictionaryEnabled()

  def isWadokuEnabled(self): return to_bool(self.value('WadokuEnabled'))
  def setWadokuEnabled(self, v):
    if v != self.isWadokuEnabled():
      self.setValue('WadokuEnabled', v)
      self._updateDictionaryEnabled()

  def isLingoesJaZhEnabled(self): return to_bool(self.value('LingoesJaZh'))
  def setLingoesJaZhEnabled(self, v):
    if v != self.isLingoesJaZhEnabled():
      self.setValue('LingoesJaZh', v)
      self._updateDictionaryEnabled()

  def isLingoesJaKoEnabled(self): return to_bool(self.value('LingoesJaKo'))
  def setLingoesJaKoEnabled(self, v):
    if v != self.isLingoesJaKoEnabled():
      self.setValue('LingoesJaKo', v)
      self._updateDictionaryEnabled()

  def isLingoesJaViEnabled(self): return to_bool(self.value('LingoesJaVi'))
  def setLingoesJaViEnabled(self, v):
    if v != self.isLingoesJaViEnabled():
      self.setValue('LingoesJaVi', v)
      self._updateDictionaryEnabled()

  def isLingoesJaEnEnabled(self): return to_bool(self.value('LingoesJaEn'))
  def setLingoesJaEnEnabled(self, v):
    if v != self.isLingoesJaEnEnabled():
      self.setValue('LingoesJaEn', v)
      self._updateDictionaryEnabled()

  def isLingoesDictionaryEnabled(self, name):
    if name == 'ja-zh':
      return self.isLingoesJaZhEnabled()
    elif name == 'ja-ko':
      return self.isLingoesJaKoEnabled()
    elif name == 'ja-vi':
      return self.isLingoesJaViEnabled()
    elif name == 'ja-en':
      return self.isLingoesJaEnEnabled()

  def setLingoesDictionaryEnabled(self, name, v):
    if name == 'ja-zh':
      self.setLingoesJaZhEnabled(v)
    elif name == 'ja-ko':
      self.setLingoesJaKoEnabled(v)
    elif name == 'ja-vi':
      self.setLingoesJaViEnabled(v)
    elif name == 'ja-en':
      self.setLingoesJaEnEnabled(v)

  # JMDict

  def isJMDictFrEnabled(self): return to_bool(self.value('JMDictFr'))
  def setJMDictFrEnabled(self, v):
    if v != self.isJMDictFrEnabled():
      self.setValue('JMDictFr', v)
      self._updateDictionaryEnabled()

  def isJMDictRuEnabled(self): return to_bool(self.value('JMDictRu'))
  def setJMDictRuEnabled(self, v):
    if v != self.isJMDictRuEnabled():
      self.setValue('JMDictRu', v)
      self._updateDictionaryEnabled()

  def isJMDictNlEnabled(self): return to_bool(self.value('JMDictNl'))
  def setJMDictNlEnabled(self, v):
    if v != self.isJMDictNlEnabled():
      self.setValue('JMDictNl', v)
      self._updateDictionaryEnabled()

  def isJMDictEnabled(self, lang):
    if lang == 'fr':
      return self.isJMDictFrEnabled()
    elif lang == 'ru':
      return self.isJMDictRuEnabled()
    elif lang == 'nl':
      return self.isJMDictNlEnabled()

  def setJMDictEnabled(self, lang, v):
    if name == 'fr':
      self.setJMDictFrEnabled(v)
    elif name == 'ru':
      self.setJMDictRuEnabled(v)
    elif name == 'nl':
      self.setJMDictNlEnabled(v)

  ## Machine translation ##

  # "atlas", or "infoseek", or "jbeijing"
  #machineTranslatorChanged = Signal(unicode)
  #def machineTranslator(self):
  #  return self.value('MachineTranslator', 'infoseek')
  #def setMachineTranslator(self, value):
  #  if value != self.machineTranslator():
  #    self.setValue('MachineTranslator', value)
  #    self.machineTranslatorChanged.emit(value)

  #def isMsimeCorrectionEnabled(self): return to_bool(self.value('MsimeCorrectionEnabled'))
  #def setMsimeCorrectionEnabled(self, value): self.setValue('MsimeCorrectionEnabled', value)

  #msimeParserEnabledChanged = Signal(bool)
  #def isMsimeParserEnabled(self):
  #  return to_bool(self.value('MsimeParserEnabled'))
  #def setMsimeParserEnabled(self, value):
  #  if value != self.isMsimeParserEnabled():
  #    self.setValue('MsimeParserEnabled', value)
  #    self.msimeParserEnabledChanged.emit(value)

  rubyTypeChanged = Signal(str)
  def rubyType(self):
    return self.value('FuriganaType', 'hiragana')
  def setRubyType(self, value):
    if value != self.rubyType():
      self.setValue('FuriganaType', value)
      self.rubyTypeChanged.emit(value)

  def jbeijingLocation(self):
    return to_unicode(self.value('JBeijingLocation'))
  def setJBeijingLocation(self, path):
    self.setValue('JBeijingLocation', path)

  def ezTransLocation(self):
    return to_unicode(self.value('ezTransLocation'))
  def setEzTransLocation(self, path):
    self.setValue('ezTransLocation', path)

  def dreyeLocation(self):
    return to_unicode(self.value('DreyeLocation'))
  def setDreyeLocation(self, path):
    self.setValue('DreyeLocation', path)

  def atlasLocation(self):
    return to_unicode(self.value('AtlasLocation'))
  def setAtlasLocation(self, path):
    self.setValue('AtlasLocation', path)

  def lecLocation(self):
    return to_unicode(self.value('LecLocation'))
  def setLecLocation(self, path):
    self.setValue('LecLocation', path)

  windowHookEnabledChanged = Signal(bool)
  def isWindowHookEnabled(self):
    return to_bool(self.value('WindowHookEnabled')) # disabled by default
  #def setWindowHookEnabled(self, value):
  #  if value != self.isWindowHookEnabled():
  #    self.setValue('WindowHookEnabled', value)
  #    self.windowHookEnabledChanged.emit(value)

  windowTextVisibleChanged = Signal(bool)
  def isWindowTextVisible(self):
    return to_bool(self.value('WindowTextVisible', True))
  #def setWindowTextVisible(self, value):
  #  if value != self.isWindowTextVisible():
  #    self.setValue('WindowTextVisible', value)
  #    self.windowTextVisibleChanged.emit(value)

  infoseekEnabledChanged = Signal(bool)
  def isInfoseekEnabled(self):
    return to_bool(self.value('InfoseekEnabled', False))
  def setInfoseekEnabled(self, value):
    if value != self.isInfoseekEnabled():
      self.setValue('InfoseekEnabled', value)
      self.infoseekEnabledChanged.emit(value)

  exciteEnabledChanged = Signal(bool)
  def isExciteEnabled(self):
    return to_bool(self.value('ExciteEnabled'))
  def setExciteEnabled(self, value):
    if value != self.isExciteEnabled():
      self.setValue('ExciteEnabled', value)
      self.exciteEnabledChanged.emit(value)

  googleEnabledChanged = Signal(bool)
  def isGoogleEnabled(self):
    return to_bool(self.value('GoogleEnabled', False))
  def setGoogleEnabled(self, value):
    if value != self.isGoogleEnabled():
      self.setValue('GoogleEnabled', value)
      self.googleEnabledChanged.emit(value)

  bingEnabledChanged = Signal(bool)
  def isBingEnabled(self):
    return to_bool(self.value('BingEnabled', True))
  def setBingEnabled(self, value):
    if value != self.isBingEnabled():
      self.setValue('BingEnabled', value)
      self.bingEnabledChanged.emit(value)

  baiduEnabledChanged = Signal(bool)
  def isBaiduEnabled(self):
    return to_bool(self.value('BaiduEnabled'))
  def setBaiduEnabled(self, value):
    if value != self.isBaiduEnabled():
      self.setValue('BaiduEnabled', value)
      self.baiduEnabledChanged.emit(value)

  lougoEnabledChanged = Signal(bool)
  def isLougoEnabled(self):
    #return to_bool(self.value('LougoEnabled'))
    return False # always disable
  def setLougoEnabled(self, value):
    if value != self.isLougoEnabled():
      self.setValue('LougoEnabled', value)
      self.lougoEnabledChanged.emit(value)

  machineTranslatorChanged = Signal()

  atlasEnabledChanged = Signal(bool)
  def isAtlasEnabled(self):
    return to_bool(self.value('AtlasEnabled'))
  def setAtlasEnabled(self, value):
    if value != self.isAtlasEnabled():
      self.setValue('AtlasEnabled', value)
      self.atlasEnabledChanged.emit(value)
      self.machineTranslatorChanged.emit()

  lecEnabledChanged = Signal(bool)
  def isLecEnabled(self):
    return to_bool(self.value('LecEnabled'))
  def setLecEnabled(self, value):
    if value != self.isLecEnabled():
      self.setValue('LecEnabled', value)
      self.lecEnabledChanged.emit(value)
      self.machineTranslatorChanged.emit()

  jbeijingEnabledChanged = Signal(bool)
  def isJBeijingEnabled(self):
    return to_bool(self.value('JBeijingEnabled'))
  def setJBeijingEnabled(self, value):
    if value != self.isJBeijingEnabled():
      self.setValue('JBeijingEnabled', value)
      self.jbeijingEnabledChanged.emit(value)
      self.machineTranslatorChanged.emit()

  dreyeEnabledChanged =Signal(bool)
  def isDreyeEnabled(self):
    return to_bool(self.value('DreyeEnabled'))
  def setDreyeEnabled(self, value):
    if value != self.isDreyeEnabled():
      self.setValue('DreyeEnabled', value)
      self.dreyeEnabledChanged.emit(value)
      self.machineTranslatorChanged.emit()

  ezTransEnabledChanged = Signal(bool)
  def isEzTransEnabled(self):
    return to_bool(self.value('ezTransEnabled'))
  def setEzTransEnabled(self, value):
    if value != self.isEzTransEnabled():
      self.setValue('ezTransEnabled', value)
      self.ezTransEnabledChanged.emit(value)
      self.machineTranslatorChanged.emit()

  ## Translation options ##

  termEnabledChanged = Signal(bool)
  def isTermEnabled(self): return to_bool(self.value('TermEnabled', True))
  #def setTermEnabled(self, value):
  #  if value != self.isTermEnabled():
  #    self.setValue('TermEnabled', value)
  #    self.termEnabledChanged.emit(value)

  copiesGameTextChanged = Signal(bool)
  def copiesGameText(self): return to_bool(self.value('CopyGameText'))

  convertsChineseChanged = Signal(bool)
  def convertsChinese(self):
    return to_bool(self.value('ConvertsChinese'))
  def setConvertsChinese(self, value):
    if value != self.convertsChinese():
      self.setValue('ConvertsChinese', value)
      self.convertsChineseChanged.emit(value)

  ## Fonts ##

  japaneseFontChanged = Signal(unicode)
  def japaneseFont(self):
    return self.value('JapaneseFont', config.FONT_JA)
  def setJapaneseFont(self, value):
    if value != self.japaneseFont():
      self.setValue('JapaneseFont', value)
      self.japaneseFontChanged.emit(value)

  englishFontChanged = Signal(unicode)
  def englishFont(self):
    return self.value('EnglishFont', config.FONT_EN)
  def setEnglishFont(self, value):
    if value != self.englishFont():
      self.setValue('EnglishFont', value)
      self.englishFontChanged.emit(value)

  chineseFontChanged = Signal(unicode)
  def chineseFont(self):
    return self.value('ChineseFont', config.FONT_ZHT)
  def setChineseFont(self, value):
    if value != self.chineseFont():
      self.setValue('ChineseFont', value)
      self.chineseFontChanged.emit(value)

  #simplifiedChineseFontChanged = Signal(unicode)
  #def simplifiedChineseFont(self):
  #  return self.value('SimplifiedChineseFont', config.FONT_ZHS)
  #def setSimplifiedChineseFont(self, value):
  #  if value != self.simplifiedChineseFont():
  #    self.setValue('SimplifiedChineseFont', value)
  #    self.simplifiedChineseFontChanged.emit(value)

  koreanFontChanged = Signal(unicode)
  def koreanFont(self):
    return self.value('KoreanFont', config.FONT_KO)
  def setKoreanFont(self, value):
    if value != self.koreanFont():
      self.setValue('KoreanFont', value)
      self.koreanFontChanged.emit(value)

  thaiFontChanged = Signal(unicode)
  def thaiFont(self):
    return self.value('ThaiFont', config.FONT_TH)
  def setThaiFont(self, value):
    if value != self.thaiFont():
      self.setValue('ThaiFont', value)
      self.thaiFontChanged.emit(value)

  vietnameseFontChanged = Signal(unicode)
  def vietnameseFont(self):
    return self.value('VietnameseFont', config.FONT_VI)
  def setVietnameseFont(self, value):
    if value != self.vietnameseFont():
      self.setValue('VietnameseFont', value)
      self.vietnameseFontChanged.emit(value)

  malaysianFontChanged = Signal(unicode)
  def malaysianFont(self):
    return self.value('MalaysianFont', config.FONT_MS)
  def setMalaysianFont(self, value):
    if value != self.malaysianFont():
      self.setValue('MalaysianFont', value)
      self.malaysianFontChanged.emit(value)

  indonesianFontChanged = Signal(unicode)
  def indonesianFont(self):
    return self.value('IndonesianFont', config.FONT_ID)
  def setIndonesianFont(self, value):
    if value != self.indonesianFont():
      self.setValue('IndonesianFont', value)
      self.indonesianFontChanged.emit(value)

  germanFontChanged = Signal(unicode)
  def germanFont(self):
    return self.value('GermanFont', config.FONT_DE)
  def setGermanFont(self, value):
    if value != self.germanFont():
      self.setValue('GermanFont', value)
      self.germanFontChanged.emit(value)

  frenchFontChanged = Signal(unicode)
  def frenchFont(self):
    return self.value('FrenchFont', config.FONT_FR)
  def setFrenchFont(self, value):
    if value != self.frenchFont():
      self.setValue('FrenchFont', value)
      self.frenchFontChanged.emit(value)

  italianFontChanged = Signal(unicode)
  def italianFont(self):
    return self.value('ItalianFont', config.FONT_IT)
  def setItalianFont(self, value):
    if value != self.italianFont():
      self.setValue('ItalianFont', value)
      self.italianFontChanged.emit(value)

  spanishFontChanged = Signal(unicode)
  def spanishFont(self):
    return self.value('SpanishFont', config.FONT_ES)
  def setSpanishFont(self, value):
    if value != self.spanishFont():
      self.setValue('SpanishFont', value)
      self.spanishFontChanged.emit(value)

  portugueseFontChanged = Signal(unicode)
  def portugueseFont(self):
    return self.value('PortugueseFont', config.FONT_PT)
  def setPortugueseFont(self, value):
    if value != self.portugueseFont():
      self.setValue('PortugueseFont', value)
      self.portugueseFontChanged.emit(value)

  russianFontChanged = Signal(unicode)
  def russianFont(self):
    return self.value('RussianFont', config.FONT_RU)
  def setRussianFont(self, value):
    if value != self.russianFont():
      self.setValue('RussianFont', value)
      self.russianFontChanged.emit(value)

  polishFontChanged = Signal(unicode)
  def polishFont(self):
    return self.value('PolishFont', config.FONT_PL)
  def setPolishFont(self, value):
    if value != self.polishFont():
      self.setValue('PolishFont', value)
      self.polishFontChanged.emit(value)

  dutchFontChanged = Signal(unicode)
  def dutchFont(self):
    return self.value('DutchFont', config.FONT_NL)
  def setDutchFont(self, value):
    if value != self.dutchFont():
      self.setValue('DutchFont', value)
      self.dutchFontChanged.emit(value)

@memoized
def global_(): return Settings()

## Kagami settings proxy ##

#def int_property(name, default, notify=None):
#  return Property(int,
#      lambda _: to_int(global_().value(name, default)),
#      lambda _, value: global_().setValue(name, value),
#      notify=notify)

def int_property(name, default, notify=None):
  return Property(int,
      lambda _: to_int(global_().value(name, default)),
      lambda _, value: global_().setValue(name, value),
      notify=notify)

def float_property(name, default, notify=None):
  return Property(float,
      lambda _: to_float(global_().value(name, default), default),
      lambda _, value: global_().setValue(name, value),
      notify=notify)

def bool_property(name, default, notify=None):
  return Property(bool,
      lambda _: to_bool(global_().value(name, default)),
      lambda _, value: global_().setValue(name, value),
      notify=notify)

def unicode_property(name, default="", notify=None):
  return Property(unicode,
      lambda _: to_unicode(global_().value(name, default)),
      lambda _, value: global_().setValue(name, value),
      notify=notify)

@QmlObject
class SettingsProxy(QObject):

  @Slot()
  def sync(self): global_().sync()

  def __init__(self, parent=None):
    super(SettingsProxy, self).__init__(parent)
    g = global_()
    #g.applocEnabledChanged.connect(self.applocEnabledChanged)
    #g.timeZoneEnabledChanged.connect(self.timeZoneEnabledChanged)
    g.springBoardWallpaperUrlChanged.connect(self.springBoardWallpaperUrlChanged)
    g.springBoardSlidesEnabledChanged.connect(self.springBoardSlidesEnabledChanged)

    g.grimoireFontColorChanged.connect(self.grimoireFontColorChanged)
    g.grimoireShadowColorChanged.connect(self.grimoireShadowColorChanged)
    g.grimoireTextColorChanged.connect(self.grimoireTextColorChanged)
    #g.grimoireTranslationColorChanged.connect(self.grimoireTranslationColorChanged)
    g.grimoireSubtitleColorChanged.connect(self.grimoireSubtitleColorChanged)
    g.grimoireCommentColorChanged.connect(self.grimoireCommentColorChanged)
    g.grimoireDanmakuColorChanged.connect(self.grimoireDanmakuColorChanged)

    g.infoseekColorChanged.connect(self.infoseekColorChanged)
    g.exciteColorChanged.connect(self.exciteColorChanged)
    g.bingColorChanged.connect(self.bingColorChanged)
    g.googleColorChanged.connect(self.googleColorChanged)
    g.baiduColorChanged.connect(self.baiduColorChanged)
    g.lougoColorChanged.connect(self.lougoColorChanged)
    g.jbeijingColorChanged.connect(self.jbeijingColorChanged)
    g.dreyeColorChanged.connect(self.dreyeColorChanged)
    g.ezTransColorChanged.connect(self.ezTransColorChanged)
    g.atlasColorChanged.connect(self.atlasColorChanged)
    g.lecColorChanged.connect(self.lecColorChanged)

    g.rubyTypeChanged.connect(self.rubyTypeChanged)
    g.convertsChineseChanged.connect(self.convertsChineseChanged)

    #g.msimeParserEnabledChanged.connect(self.msimeParserEnabledChanged)
    #g.meCabEnabledChanged.connect(self.meCabEnabledChanged)
    g.meCabDictionaryChanged.connect(self.meCabDictionaryChanged)

    self.hentaiChanged.connect(g.hentaiChanged)

    self.termEnabledChanged.connect(g.termEnabledChanged)
    self.copiesGameTextChanged.connect(g.copiesGameTextChanged)

    self.windowHookEnabledChanged.connect(g.windowHookEnabledChanged)
    self.windowTextVisibleChanged.connect(g.windowTextVisibleChanged)

    self.gameTextCapacityChanged.connect(g.gameTextCapacityChanged)

    #for sig in 'japaneseFontChanged':
    #  getattr(g, sig).connect(
    #      getattr(self, sig))
    g.japaneseFontChanged.connect(self.japaneseFontChanged)
    g.englishFontChanged.connect(self.englishFontChanged)
    g.chineseFontChanged.connect(self.chineseFontChanged)
    #g.simplifiedChineseFontChanged.connect(self.simplifiedChineseFontChanged)
    g.koreanFontChanged.connect(self.koreanFontChanged)
    g.thaiFontChanged.connect(self.thaiFontChanged)
    g.vietnameseFontChanged.connect(self.vietnameseFontChanged)
    g.malaysianFontChanged.connect(self.malaysianFontChanged)
    g.indonesianFontChanged.connect(self.indonesianFontChanged)
    g.germanFontChanged.connect(self.germanFontChanged)
    g.frenchFontChanged.connect(self.frenchFontChanged)
    g.italianFontChanged.connect(self.italianFontChanged)
    g.spanishFontChanged.connect(self.spanishFontChanged)
    g.portugueseFontChanged.connect(self.portugueseFontChanged)
    g.russianFontChanged.connect(self.russianFontChanged)
    g.polishFontChanged.connect(self.polishFontChanged)
    g.dutchFontChanged.connect(self.dutchFontChanged)

    g.voiceCharacterEnabledChanged.connect(self.voiceCharacterEnabledChanged)
    g.speaksGameTextChanged.connect(self.speaksGameTextChanged)

  def setHentai(self, value):
    if value != self.hentai:
      global_().setValue('Hentai', value)
      self.hentaiChanged.emit(value)
  hentaiChanged = Signal(bool)
  hentai = Property(bool,
      lambda _: global_().isHentai(),
      setHentai,
      notify=hentaiChanged)

  def setGameTextCapacity(self, value):
    if value != self.gameTextCapacity:
      global_().setValue('GameTextCapacity', value)
      self.gameTextCapacityChanged.emit(value)
  gameTextCapacityChanged = Signal(int)
  gameTextCapacity = Property(int,
      lambda _: global_().gameTextCapacity(),
      setGameTextCapacity,
      notify=gameTextCapacityChanged)

  def setTermEnabled(self, value):
    if value != self.termEnabled:
      global_().setValue('TermEnabled', value)
      self.termEnabledChanged.emit(value)
  termEnabledChanged = Signal(bool)
  termEnabled = Property(bool,
      lambda _: global_().isTermEnabled(),
      setTermEnabled,
      notify=termEnabledChanged)

  def setWindowHookEnabled(self, value):
    if value != self.windowHookEnabled:
      global_().setValue('WindowHookEnabled', value)
      self.windowHookEnabledChanged.emit(value)
  windowHookEnabledChanged = Signal(bool)
  windowHookEnabled = Property(bool,
      lambda _: global_().isWindowHookEnabled(),
      setWindowHookEnabled,
      notify=windowHookEnabledChanged)

  def setWindowTextVisible(self, value):
    if value != self.windowTextVisible:
      global_().setValue('WindowTextVisible', value)
      self.windowTextVisibleChanged.emit(value)
  windowTextVisibleChanged = Signal(bool)
  windowTextVisible = Property(bool,
      lambda _: global_().isWindowTextVisible(),
      setWindowTextVisible,
      notify=windowTextVisibleChanged)

  speaksGameTextChanged = Signal(bool)
  speaksGameText = Property(bool,
      lambda _: global_().speaksGameText(),
      lambda _, v: global_().setSpeaksGameText(v),
      notify=speaksGameTextChanged)

  voiceCharacterEnabledChanged = Signal(bool)
  voiceCharacterEnabled = Property(bool,
      lambda _: global_().isVoiceCharacterEnabled(),
      lambda _, v: global_().setVoiceCharacterEnabled(v),
      notify=voiceCharacterEnabledChanged)

  #subtitleVoiceEnabled = bool_property('SubtitleVoice', False)

  japaneseFontChanged = Signal(unicode)
  japaneseFont = unicode_property('JapaneseFont', config.FONT_JA, notify=japaneseFontChanged)
  englishFontChanged = Signal(unicode)
  englishFont = unicode_property('EnglishFont', config.FONT_EN, notify=englishFontChanged)
  chineseFontChanged = Signal(unicode)
  chineseFont = unicode_property('ChineseFont', config.FONT_ZHT, notify=chineseFontChanged)
  #simplifiedChineseFontChanged = Signal(unicode)
  #simplifiedChineseFont = unicode_property('SimplifiedChineseFont', config.FONT_ZHS, notify=simplifiedChineseFontChanged)
  koreanFontChanged = Signal(unicode)
  koreanFont = unicode_property('KoreanFont', config.FONT_KO, notify=koreanFontChanged)
  thaiFontChanged = Signal(unicode)
  thaiFont = unicode_property('ThaiFont', config.FONT_TH, notify=thaiFontChanged)
  vietnameseFontChanged = Signal(unicode)
  vietnameseFont = unicode_property('VietnameseFont', config.FONT_VI, notify=vietnameseFontChanged)
  malaysianFontChanged = Signal(unicode)
  malaysianFont = unicode_property('MalaysianFont', config.FONT_ID, notify=malaysianFontChanged)
  indonesianFontChanged = Signal(unicode)
  indonesianFont = unicode_property('IndonesianFont', config.FONT_ID, notify=indonesianFontChanged)
  germanFontChanged = Signal(unicode)
  germanFont = unicode_property('GermanFont', config.FONT_DE, notify=germanFontChanged)
  frenchFontChanged = Signal(unicode)
  frenchFont = unicode_property('FrenchFont', config.FONT_FR, notify=frenchFontChanged)
  italianFontChanged = Signal(unicode)
  italianFont = unicode_property('ItalianFont', config.FONT_IT, notify=italianFontChanged)
  spanishFontChanged = Signal(unicode)
  spanishFont = unicode_property('SpanishFont', config.FONT_ES, notify=spanishFontChanged)
  portugueseFontChanged = Signal(unicode)
  portugueseFont = unicode_property('PortugueseFont', config.FONT_PT, notify=portugueseFontChanged)
  russianFontChanged = Signal(unicode)
  russianFont = unicode_property('RussianFont', config.FONT_RU, notify=russianFontChanged)
  polishFontChanged = Signal(unicode)
  polishFont = unicode_property('PolishFont', config.FONT_RU, notify=polishFontChanged)
  dutchFontChanged = Signal(unicode)
  dutchFont = unicode_property('DutchFont', config.FONT_NL, notify=dutchFontChanged)

  #msimeParserEnabledChanged = Signal(bool)
  #msimeParserEnabled = bool_property('MsimeParserEnabled', False, notify=msimeParserEnabledChanged)

  kagamiClockEnabledChanged = Signal(bool)
  kagamiClockEnabled = bool_property('KagamiClock', False, notify=kagamiClockEnabledChanged)
  kagamiClockXChanged = Signal(int)
  kagamiClockX = int_property('KagamiClockX', 0, notify=kagamiClockXChanged)
  kagamiClockYChanged = Signal(int)
  kagamiClockY = int_property('KagamiClockY', 0, notify=kagamiClockYChanged)

  convertsChineseChanged = Signal(bool)
  convertsChinese = bool_property('ConvertsChinese', False, notify=convertsChineseChanged)

  #meCabEnabledChanged = Signal(bool)
  #meCabEnabled = bool_property('MeCabEnabled', False, notify=meCabEnabledChanged)
  meCabDictionaryChanged = Signal(unicode)
  meCabDictionary = unicode_property('MeCabDictionary', '', notify=meCabDictionaryChanged)

  rubyTypeChanged = Signal(unicode)
  rubyType = unicode_property('FuriganaType', 'hiragana', notify=rubyTypeChanged)

  springBoardWallpaperUrlChanged = Signal(unicode)
  springBoardWallpaperUrl = unicode_property('SpringBoardWallpaperUrl', notify=springBoardWallpaperUrlChanged)

  springBoardSlidesEnabledChanged = Signal(bool)
  springBoardSlidesEnabled = bool_property('SpringBoardSlidesEnabled', True, notify=springBoardSlidesEnabledChanged)

  #applocEnabledChanged = Signal(bool)
  #applocEnabled = bool_property('ApplocEnabled', True, notify=applocEnabledChanged)

  #timeZoneEnabledChanged = Signal(bool)
  #timeZoneEnabled = bool_property('TimeZoneEnabled', True, notify=timeZoneEnabledChanged)

  grimoireNormalizedXChanged = Signal(float)
  grimoireNormalizedX = float_property('GrimoireNormalizedX', 0.0, notify=grimoireNormalizedXChanged)
  grimoireNormalizedYChanged = Signal(float)
  grimoireNormalizedY = float_property('GrimoireNormalizedY', 0.0, notify=grimoireNormalizedYChanged)

  grimoireFontColorChanged = Signal(unicode)
  grimoireFontColor = unicode_property('GrimoireFontColor',
      config.SETTINGS_FONT_COLOR,
      notify=grimoireFontColorChanged)
  grimoireShadowColorChanged = Signal(unicode)
  grimoireShadowColor = unicode_property('GrimoireShadowColor',
      config.SETTINGS_SHADOW_COLOR,
      notify=grimoireShadowColorChanged)
  grimoireTextColorChanged = Signal(unicode)
  grimoireTextColor = unicode_property('GrimoireTextColor',
      config.SETTINGS_TEXT_COLOR,
      notify=grimoireTextColorChanged)
  grimoireSubtitleColorChanged = Signal(unicode)
  grimoireSubtitleColor = unicode_property('GrimoireSubtitleColor',
      config.SETTINGS_SUBTITLE_COLOR,
      notify=grimoireSubtitleColorChanged)
  #grimoireTranslationColorChanged = Signal(unicode)
  #grimoireTranslationColor = unicode_property('GrimoireTranslationColor',
  #    config.SETTINGS_TRANSLATION_COLOR,
  #    notify=grimoireTranslationColorChanged)
  grimoireCommentColorChanged = Signal(unicode)
  grimoireCommentColor = unicode_property('GrimoireCommentColor',
      config.SETTINGS_COMMENT_COLOR,
      notify=grimoireCommentColorChanged)
  grimoireDanmakuColorChanged = Signal(unicode)
  grimoireDanmakuColor = unicode_property('GrimoireDanmakuColor',
      config.SETTINGS_DANMAKU_COLOR,
      notify=grimoireDanmakuColorChanged)

  infoseekColorChanged = Signal(unicode)
  infoseekColor = unicode_property('InfoseekColor', config.SETTINGS_INFOSEEK_COLOR, notify=infoseekColorChanged)
  exciteColorChanged = Signal(unicode)
  exciteColor = unicode_property('ExciteColor', config.SETTINGS_EXCITE_COLOR, notify=exciteColorChanged)
  googleColorChanged = Signal(unicode)
  googleColor = unicode_property('GoogleColor', config.SETTINGS_GOOGLE_COLOR, notify=googleColorChanged)
  bingColorChanged = Signal(unicode)
  bingColor = unicode_property('BingColor', config.SETTINGS_BING_COLOR, notify=bingColorChanged)
  baiduColorChanged = Signal(unicode)
  baiduColor = unicode_property('BaiduColor', config.SETTINGS_BAIDU_COLOR, notify=baiduColorChanged)
  lougoColorChanged = Signal(unicode)
  lougoColor = unicode_property('LougoColor', config.SETTINGS_LOUGO_COLOR, notify=lougoColorChanged)
  jbeijingColorChanged = Signal(unicode)
  jbeijingColor = unicode_property('JBeijingColor', config.SETTINGS_JBEIJING_COLOR, notify=jbeijingColorChanged)
  dreyeColorChanged = Signal(unicode)
  dreyeColor = unicode_property('DreyeColor', config.SETTINGS_DREYE_COLOR, notify=dreyeColorChanged)
  ezTransColorChanged = Signal(unicode)
  ezTransColor = unicode_property('EzTransColor', config.SETTINGS_EZTRANS_COLOR, notify=ezTransColorChanged)
  atlasColorChanged = Signal(unicode)
  atlasColor = unicode_property('AtlasColor', config.SETTINGS_ATLAS_COLOR, notify=atlasColorChanged)
  lecColorChanged = Signal(unicode)
  lecColor = unicode_property('LecColor', config.SETTINGS_LEC_COLOR, notify=lecColorChanged)

  #grimoireRevertsColor = bool_property('GrimoireRevertsColor', False)
  grimoireShadowEnabled = bool_property('GrimoireShadow', True)
  grimoireHoverEnabled = bool_property('GrimoireHover', False)
  #grimoireReadEnabled = bool_property('GrimoireRead', True)
  #grimoirePopupEnabled = bool_property('GrimoirePopup', True)
  #grimoireCopyEnabled = bool_property('GrimoireCopy', False)
  #grimoireFuriganaEnabled = bool_property('GrimoireFurigana', True)
  grimoireAlignCenter = bool_property('GrimoireAlignCenter', False)
  #gameBorderVisible = bool_property('GameBorderVisible', False)

  grimoireSlimDock = bool_property('GrimoireSlimDock', False)

  grimoireTextVisible = bool_property('GrimoireText', True)
  grimoireTranslationVisible = bool_property('GrimoireTranslation', True)
  grimoireSubtitleVisible = bool_property('GrimoireSubtitle', True)
  grimoireCommentVisible = bool_property('GrimoireComment', True)
  grimoireDanmakuVisible = bool_property('GrimoireDanmaku', True)
  grimoireNameVisible = bool_property('GrimoireName', True)

  shioriZoomFactor = float_property('ShioriZoomFactor', config.SETTINGS_ZOOM_FACTOR)

  grimoireZoomFactor = float_property('GrimoireZoomFactor', config.SETTINGS_ZOOM_FACTOR)
  grimoireWidthFactor = float_property('GrimoireWidthFactor', config.SETTINGS_WIDTH_FACTOR)
  grimoireShadowOpacity = float_property('GrimoireShadowOpacity', config.SETTINGS_SHADOW_OPACITY)

  mirageTextVisible = bool_property('MirageText', True)
  mirageTranslationVisible = bool_property('MirageTranslation', True)
  mirageSpeaksText = bool_property('MirageSpeaksText', False)
  mirageSpeaksTranslation = bool_property('MirageSpeaksTranslation', False)
  mirageMouseEnabled = bool_property('MirageMouse', True)
  mirageClipboardEnabled = bool_property('MirageClipboard', True)

  glowIntensity = float_property('GlowIntensity', 1)
  glowRadius = float_property('GlowRadius', 4)

  # Only applied to grimoire, though
  splitsGameText = bool_property('SplitGameText', True)
  splitsTranslation = bool_property('SplitTranslation', False)

  copiesGameTextChanged = Signal(bool)
  copiesGameText = bool_property('CopyGameText', False, notify=copiesGameTextChanged)

  kagamiFocusEnabledChanged = Signal(bool)
  kagamiFocusEnabled = bool_property('KagamiFocusEnabled', False, notify=kagamiFocusEnabledChanged)

  graffitiCopyEnabled = bool_property('GraffitiCopy', True)
  graffitiHeaderVisible = bool_property('GraffitiHeaderVisible', True)
  #graffitiSpellCheck = bool_property('GraffitiSpellCheck', True)
  #graffitiGroupEnabled = bool_property('GraffitiGroupEnabled', True)

  #graffitiTextVisible = bool_property('GraffitiText', True)
  #graffitiTranslationVisible = bool_property('GraffitiTranslation', True)
  #graffitiSubtitleVisible = bool_property('GraffitiSubtitle', True)
  #graffitiCommentVisible = bool_property('GraffitiComment', True)
  #graffitiDanmakuVisible = bool_property('GraffitiDanmaku', True)

  inputSpellCheck = bool_property('InputSpellCheck', True)
  inputAcceptsReturn = bool_property('InputAcceptsReturn', False)
  inputAcceptsShiftReturn = bool_property('InputAcceptsShiftReturn', False)
  inputLocked = bool_property('InputLocked', False)

# EOF

  #kagamiIgnoresFocusChanged = Signal(bool)
  #kagamiIgnoresFocus = bool_property('KagamiIgnoresFocus', True, notify=kagamiIgnoresFocusChanged)

  #gameDetectionEnabledChanged = Signal(bool)
  #gameDetectionEnabled = bool_property('GameDetectionEnabled', True, notify=gameDetectionEnabledChanged)

  #hookCodeEnabledChanged = Signal(bool)
  #hookCodeEnabled = bool_property('HookCodeEnabled', True, notify=hookCodeEnabledChanged)

  #def setKagamiIgnoresFocus(self, value):
  #  if value != self.kagamiIgnoresFocus:
  #    global_().setValue('KagamiIgnoresFocus', value)
  #    self.kagamiIgnoresFocusChanged.emit(value)
  #kagamiIgnoresFocusChanged = Signal(bool)
  #kagamiIgnoresFocus = Property(bool,
  #    lambda _: global_().kagamiIgnoresFocus(),
  #    notify=kagamiIgnoresFocusChanged)
