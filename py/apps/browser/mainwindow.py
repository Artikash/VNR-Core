# coding: utf8
# mainwindow.py
# 12/13/2012 jichi

__all__ = ['MainWindow']

from itertools import imap
from PySide.QtCore import QTimer
from sakurakit.skclass import memoizedproperty, Q_Q
from sakurakit.sktr import tr_
from webbrowser import WebBrowser
import config, rc, ui

@Q_Q
class _MainWindow(object):

  @memoizedproperty
  def statusBarTimer(self):
    q = self.q
    ret = QTimer(q)
    ret.setSingleShot(True)
    ret.setInterval(5000)
    ret.timeout.connect(q.statusBar().hide)
    return ret

class MainWindow(WebBrowser):
  def __init__(self, parent=None):
    #WINDOW_FLAGS = (
    #  Qt.Window
    #  | Qt.CustomizeWindowHint
    #  | Qt.WindowTitleHint
    #  | Qt.WindowSystemMenuHint
    #  | Qt.WindowMinMaxButtonsHint
    #  | Qt.WindowCloseButtonHint
    #)
    super(MainWindow, self).__init__(parent)
    self.setStyleSheet(''.join(imap(rc.qss, (
      #'bootstrap',
      #'share',
      'browser',
    ))).replace('$PWD', config.root_abspath()))

    self.setWindowTitle(u"Kagami (α)")
    self.setWindowIcon(rc.icon('logo-browser'))
    ui.glassifyWidget(self)
    self.__d = _MainWindow(self)

    b = self.statusBar()
    b.setGraphicsEffect(ui.glowEffect(b))

    self.messageReceived.connect(self.showStatusMessage)
    #self.showStatusMessage(tr_("Ready"))
    self.statusBar().hide()

  def showStatusMessage(self, t, type='message'):
    """@reimp
    @param  t  unicode
    @param  type  'message', 'warning', or 'error'
    """
    b = self.statusBar()
    #b.setStyleSheet(SS_STATUSBAR_MESSAGE);
    b.showMessage(t)
    if t:
      b.show()
      self.__d.statusBarTimer.start()

# EOF
