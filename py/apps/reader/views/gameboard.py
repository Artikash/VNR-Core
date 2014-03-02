# coding: utf8
# gameboard.py
# 7/31/2013 jichi

__all__ = ['GameBoardDialog']

#from functools import partial
#import operator
from PySide.QtCore import Qt, Signal, Slot
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
from sakurakit.skdebug import dprint
from sakurakit.sktr import tr_
from sakurakit.skwebkit import SkWebView, SkWebViewBean
from mytr import my, mytr_
import dataman, rc

class GameBoardBean(SkWebViewBean):
  def __init__(self, parent=None):
    """
    @parent  QWidget
    """
    super(GameBoardBean, self).__init__(parent)
    self._currentCount = 0  # int
    self._totalCount = 0    # int

  #@Slot(str, result=unicode)
  #def tr(self, t): return tr_(t)
  countChanged = Signal()

  @Slot(result=int)
  def currentCount(self): return self._currentCount

  @Slot(result=int)
  def totalCount(self): return self._totalCount

  @Slot(int, int, str, bool, unicode, result=unicode)
  def games(self, start, count, sort, reverse, filters):
    """
    @return  unicode  JSON
    """
    import jsonapi
    json, currentCount, totalCount = jsonapi.gameinfo(
        start=start, count=count, sort=sort, reverse=reverse, filters=filters)
    if start:
      self._totalCount = totalCount
      self._currentCount = min(totalCount, self._currentCount + currentCount)
    else:
      self._totalCount = totalCount
      self._currentCount = currentCount
    self.countChanged.emit()
    return json

  @Slot(int)
  def showItem(self, itemId):
    dprint("item id = %s" % itemId)
    import gameview
    gameview.manager().showItem(itemId)

@Q_Q
class _GameBoardDialog(object):

  def __init__(self, q):
    #self._locked = False

    self.bean = GameBoardBean(q)
    self.bean.countChanged.connect(self._updateTitle)

    # See: http://qt-project.org/doc/qt-4.8/qtwebkit-bridge.html
    from PySide.QtWebKit import QWebPage
    q.pageAction(QWebPage.Reload).triggered.connect(
        self.refresh, Qt.QueuedConnection)

    #q.page().setLinkDelegationPolicy(QWebPage.DelegateAllLinks) # Since there are local images
    #q.page().setLinkDelegationPolicy(QWebPage.DelegateExternalLinks)
    #import osutil
    #q.linkClicked.connect(osutil.open_url)

  def _updateTitle(self):
    t = mytr_("Game Board")
    b = self.bean
    if b.currentCount():
      t += " - %i/%i" % (b.currentCount(), b.totalCount())
    else:
      t += " - %s ><" % tr_("Empty")
    self.q.setWindowTitle(t)

  def refresh(self):
    """@reimp"""
    games = dataman.manager().getGameInfo()
    #games = sorted(games, reverse=True, key=lambda it:
    #    operator.itemgetter('timestamp'))
    import py
    q = self.q
    q.setHtml(rc.haml_template('haml/gameboard').render({
      #'cacher': cacheman.UrlCacher,
      'games': games,
      #'i18n': i18n,
      'rc': rc,
      'py': py,
      'tr': tr_,
    }))

    h = q.page().mainFrame()
    h.addToJavaScriptWindowObject('bean', self.bean)
    #from sakurakit import skevents
    #skevents.runlater((lambda: h.evaluateJavaScript('$(init)')), 3000)

class GameBoardDialog(SkWebView):
  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog | Qt.WindowMinMaxButtonsHint
    super(GameBoardDialog, self).__init__(parent, WINDOW_FLAGS)
    self.setWindowIcon(rc.icon('window-gameboard'))
    self.setWindowTitle(mytr_("Game Board"))
    self.__d = _GameBoardDialog(self)
    self.resize(750, 660)

    # Refresh only once
    self.__d.refresh()
    #from sakurakit import skevents
    #skevents.runlater(self.__d.refresh)

  #def setVisible(self, value):
  #  """@reimp @public"""
  #  if value and not self.isVisible():
  #    self.__d.refresh()
  #  super(GameBoardDialog, self).setVisible(value)
  #  if not value: # save memory
  #    self.clear()

# EOF
