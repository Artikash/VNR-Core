# coding: utf8
# coffeebean.py
# 10/11/2013 jichi

__all__ = ['YoutubeBean', 'CoffeeBeanManager']

from PySide.QtCore import Slot, QObject
from sakurakit.skclass import memoized, memoizedproperty
from sakurakit.skdebug import dprint
from sakurakit.sktr import utr_
from mytr import my
import growl

class YoutubeBean(QObject):
  def __init__(self, parent=None):
    super(YoutubeBean, self).__init__(parent)

  @Slot(str)
  def get(self, vid):
    """
    @param  vid  str  youtube id
    """
    dprint(vid)
    #growl.msg("%s YouTube(%s) ..." % (my.tr("Downloading to Desktop"), vid))
    growl.msg(my.tr("Downloading YouTube video to Desktop") + " ...")
    import procutil
    procutil.getyoutube([vid])
    #return dl.get(vid)

class I18nBean(QObject):
  def __init__(self, parent=None):
    super(I18nBean, self).__init__(parent)

  @Slot(result=unicode)
  def lang(self):
    import config, dataman
    lang = dataman.manager().user().language
    return language2htmllocale(lang) or 'ja'

  @Slot(unicode, result=unicode)
  def tr(self, text):
    """
    @param  vid  str  youtube id
    """
    return utr_(text)

@memoized
def manager(): return CoffeeBeanManager()

class CoffeeBeanManager(object):
  def __init__(self, parent=None):
    """
    @param  parent  QObject
    """
    self._parent = parent

  def parent(self): return self._parent
  def setParent(self, v): self._parent = v

  @memoizedproperty
  def clipBean(self):
    from sakurakit import skwebkit
    return skwebkit.SkClipboardProxy(self.parent())

  @memoizedproperty
  def youtubeBean(self): return YoutubeBean(self.parent())

  @memoizedproperty
  def i18nBean(self): return I18nBean(self.parent())

  @memoizedproperty
  def yakuBean(self):
    import trman
    return trman.YakuCoffeeBean(self.parent())

  @memoizedproperty
  def mecabBean(self):
    import mecabman
    return mecabman.MeCabCoffeeBean(self.parent())

  @memoizedproperty
  def growlBean(self):
    import growl
    return growl.GrowlCoffeeProxy(self.parent())

  @memoizedproperty
  def shioriBean(self):
    import shiori
    return shiori.ShioriCoffeeProxy(self.parent())

  @memoizedproperty
  def ttsBean(self):
    import ttsman
    return ttsman.TtsCoffeeProxy(self.parent())

# EOF
