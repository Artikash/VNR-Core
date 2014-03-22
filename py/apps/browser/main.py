# coding: utf8
# main.py
# 12/13/2012 jichi

__all__ = ['MainObject']

from PySide.QtCore import QCoreApplication, QObject
from sakurakit import skevents
from sakurakit.skdebug import dprint, dwarn, debugmethod
from sakurakit.skclass import memoizedproperty
import config

class MainObject(QObject):
  """Root of most objects"""
  instance = None

  # Supposed to be top-level, no parent allowed
  def __init__(self):
    dprint('enter')
    super(MainObject, self).__init__()
    self.__d = _MainObject()
    MainObject.instance = self

    dprint('leave')

  def run(self, args):
    """Starting point for the entire app"""
    dprint("enter: args =", args)
    d = self.__d

    dprint("show root window")
    w = d.mainWindow

    urls = [it for it in args[1:] if not it.startswith('-')]
    if urls:
      w.openUrls(urls)
    else:
      w.openDefaultPage()

    w.resize(800, 600)
    w.show()

    dprint("leave")

  ## Quit ##

  def quit(self, interval=200):
    dprint("enter: interval = %i" % interval)
    d = self.__d
    if d.hasQuit:
      dprint("leave: has quit")
      return

    d.mainWindow.hide()

    skevents.runlater(self.__d.quit, interval)
    dprint("leave")

  def confirmQuit(self):
    from Qt5.QtWidgets import QMessageBox
    yes = QMessageBox.Yes
    no = QMessageBox.No
    sel = QMessageBox.question(self.__d.rootWindow,
        self.tr("Web Browser"),
        self.tr("Quit the browser?"),
        yes|no, no)
    if sel == yes:
      self.quit()

# MainObject private data
class _MainObject(object):
  def __init__(self):
    self.hasQuit = False # if the application has quit
    self.widgets = [] # [QWidget]
    #q.destroyed.connect(self._onDestroyed)

  ## Windows ##

  @memoizedproperty
  def mainWindow(self):
    import mainwindow
    ret = mainwindow.MainWindow()
    ret.quitRequested.connect(self.quit)
    return ret

  ## Actions ##

  @debugmethod
  def quit(self):
    if self.hasQuit:
      return

    self.hasQuit = True

    for w in self.widgets:
      if w.isVisible():
        w.hide()

    # wait for for done or kill all threads
    from PySide.QtCore import QThreadPool
    if QThreadPool.globalInstance().activeThreadCount():
      dwarn("warning: wait for active threads")
      QThreadPool.globalInstance().waitForDone(config.QT_THREAD_TIMEOUT)
      dprint("leave qthread pool")

    dprint("send quit signal to qApp")
    qApp = QCoreApplication.instance()

    # Make sure settings.sync is the last signal conneced with aboutToQuit
    #qApp.aboutToQuit.connect(self.settings.sync)

    skevents.runlater(qApp.quit)

# EOF
