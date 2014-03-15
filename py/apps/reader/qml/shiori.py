# coding: utf8
# shiori.py
# 10/10/2012 jichi

from PySide.QtCore import Signal, Slot, Property, QObject
from sakurakit.skdebug import dprint, dwarn
#from sakurakit.skqml import QmlObject
from sakurakit.sktr import tr_
from mytr import my
from kagami import GrimoireBean
import dictman, qmldialog, settings

#@QmlObject
class ShioriBean(QObject):

  instance = None

  def __init__(self, parent=None):
    super(ShioriBean, self).__init__(parent)
    ShioriBean.instance = self
    self._enabled = True # bool

    ss = settings.global_()
    self._enabled = ss.isDictionaryEnabled()
    ss.dictionaryEnabledChanged.connect(self.setEnabled)
    dprint("pass")

  def isEnabled(self): return self._enabled
  def setEnabled(self, v):
    if self._enabled != v:
      self._enabled = v
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
    return dictman.manager().render(text, *args)

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
    QObject.__init__(self, parent)

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
