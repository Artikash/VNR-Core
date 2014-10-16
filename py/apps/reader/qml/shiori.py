# coding: utf8
# shiori.py
# 10/10/2012 jichi

from functools import partial
from PySide.QtCore import Signal, Slot, Property, QObject, QMutex
from sakurakit import skthreads
from sakurakit.skdebug import dprint, dwarn
#from sakurakit.skqml import QmlObject
from sakurakit.sktr import tr_
from mytr import my
from kagami import GrimoireBean
import dictman, qmldialog, settings

class _ShioriBean:
  def __init__(self):
    self.enabled = True
    self.renderMutex = QMutex()

#@QmlObject
class ShioriBean(QObject):

  instance = None

  def __init__(self, parent=None):
    super(ShioriBean, self).__init__(parent)
    self.__d = _ShioriBean()
    ShioriBean.instance = self

    ss = settings.global_()
    self.__d.enabled = ss.isDictionaryEnabled()
    ss.dictionaryEnabledChanged.connect(self.setEnabled)
    dprint("pass")

  def isEnabled(self): return self.__d.enabled
  def setEnabled(self, v):
    if self.__d.enabled != v:
      self.__d.enabled = v
      self.enabledChanged.emit(v)
  enabledChanged = Signal(bool)
  enabled = Property(bool, isEnabled, setEnabled, notify=enabledChanged)

  @Slot(unicode, result=unicode)
  def render(self, text):
    """
    @param  text  Japanese phrase
    @return  unicode not None  html
    """
    args = GrimoireBean.instance.lookupFeature(text) or []
    #return dictman.manager().render(text, *args)
    mutex = self.__d.renderMutex
    if mutex.tryLock():
      ret = skthreads.runsync(partial(
          dictman.manager().render, text, *args))
      mutex.unlock()
      return ret
    else:
      dwarn("ignore thread contention")
      return ""

  popup = Signal(unicode, int, int)  # text, x, y

def popupshiori(text, x, y):
  """
  @param  text  unicode
  @param  x  int
  @param  y  int
  """
  #dprint("x = %s, y = %s" % (x,y))
  if ShioriBean.instance.isEnabled():
    qmldialog.Kagami.instance.raise_()
    ShioriBean.instance.popup.emit(text, x, y)

#@QmlObject
class ShioriQmlProxy(QObject):
  def __init__(self, parent=None):
    super(ShioriQmlProxy, self).__init__(parent)

  @Slot(unicode, int, int)
  def popup(self, text, x, y):
    popupshiori(text, x, y)

class ShioriCoffeeProxy(QObject):
  def __init__(self, parent=None):
    super(ShioriCoffeeProxy, self).__init__(parent)

  @Slot(unicode, int, int)
  def popup(self, text, x, y):
    popupshiori(text, x, y)

# EOF
