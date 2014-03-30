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

  @Slot(unicode, result=unicode)
  def parse(self, text):
    import json
    import jlpman
    ret = jlpman.manager().parseToRuby(text)
    return json.dumps(ret) if ret else ''

class TtsBean(QObject):
  def __init__(self, parent):
    super(TtsBean, self).__init__(parent)

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

# EOF
