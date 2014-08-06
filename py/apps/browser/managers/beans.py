# coding: utf8
# beans.py
# 3/29/2014 jichi

import json
from PySide.QtCore import QObject, Slot
from sakurakit.skclass import memoized, memoizedproperty
import rc

@memoized
def manager(): return BeanManager()

class BeanManager(object):

  def __init__(self, parent=None): # QObject
    self.parent = parent

  def setParent(self, parent):
    self.parent = parent

  @memoizedproperty
  def cdnBean(self):
    return CdnBean(self.parent)

  @memoizedproperty
  def settingsBean(self):
    return SettingsBean(self.parent)

  @memoizedproperty
  def jlpBean(self):
    return JlpBean(self.parent)

  @memoizedproperty
  def trBean(self):
    return TranslatorBean(self.parent)

  @memoizedproperty
  def ttsBean(self):
    return TtsBean(self.parent)

  @memoizedproperty
  def clipBean(self):
    from sakurakit import skwebkit
    return skwebkit.SkClipboardProxy(self.parent)

class CdnBean(QObject):
  def __init__(self, parent):
    super(CdnBean, self).__init__(parent)

  @Slot(unicode, result=unicode)
  def url(self, key):
    return rc.cdn_url(key)

class JlpBean(QObject):
  def __init__(self, parent):
    super(JlpBean, self).__init__(parent)

  # Not used
  #@Slot(result=bool)
  #def isEnabled(self):
  #  import jlpman
  #  return jlpman.manager().isAvailable()

  @Slot(unicode, result=unicode)
  def parse(self, text):
    import json
    import jlpman
    ret = jlpman.manager().parseToRuby(text)
    return json.dumps(ret) if ret else ''

class TtsBean(QObject):
  def __init__(self, parent):
    super(TtsBean, self).__init__(parent)

  #@Slot(result=bool)
  #def isEnabled(self):
  #  import ttsman
  #  return ttsman.manager().isEnabled()

  @Slot(unicode)
  def speak(self, text):
    import ttsman
    ttsman.speak(text)

class TranslatorBean(QObject):
  def __init__(self, parent):
    super(TranslatorBean, self).__init__(parent)

  #@Slot(result=bool)
  #def isEnabled(self):
  #  import trman
  #  return trman.manager().isEnabled()

  @Slot(result=unicode)
  def translators(self): # [str translator_name]
    import trman
    return ','.join(trman.manager().enabledEngines())

  @Slot(unicode, unicode, result=unicode)
  def translateWith(self, text, engine):
    # I should not hardcode fr and to languages here
    import settings, trman
    lang = settings.reader().userLanguage()
    return trman.manager().translate(text, engine=engine, fr='ja', to=lang, async=True)[0] or ''

class SettingsBean(QObject):
  def __init__(self, parent):
    super(SettingsBean, self).__init__(parent)

  @Slot(result=bool)
  def isJlpEnabled(self):
    import settings
    return settings.global_().isMeCabEnabled()

  @Slot(result=bool)
  def isTtsEnabled(self):
    import settings
    return settings.global_().isTtsEnabled()

  @Slot(result=bool)
  def isTranslationEnabled(self):
    import settings
    return settings.global_().isTranslationEnabled()

# EOF
