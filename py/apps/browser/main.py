# coding: utf8
# main.py
# 12/13/2012 jichi

__all__ = ['MainObject']

from PySide.QtCore import QCoreApplication, QObject
from sakurakit import skevents, skos
from sakurakit.skdebug import dprint, dwarn, debugmethod
from sakurakit.skclass import Q_Q, memoizedproperty
from i18n import i18n
import config

#def global_(): return MainObject.instance

class MainObject(QObject):
  """Root of most objects"""
  #instance = None

  # Supposed to be top-level, no parent allowed
  def __init__(self):
    dprint('enter')
    super(MainObject, self).__init__()
    self.__d = _MainObject(self)
    #MainObject.instance = self

    dprint('leave')

  def run(self, args):
    """Starting point for the entire app"""
    dprint("enter: args =", args)
    d = self.__d

    dprint("create managers")
    d.beanManager
    d.jlpManager
    d.cacheManager

    dprint("show root window")
    w = d.mainWindow

    urls = [it for it in args if not it.startswith('-')]
    args_offset = 2 if skos.WIN else 1
    urls = urls[args_offset:]
    if urls:
      w.openUrls(urls)
    elif not w.loadTabs():
      w.openDefaultPage()

    # TODO: Remember the last close size
    w.resize(700, 560)
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
        u"Kagami Browser",
        i18n.tr("Quit the Kagami Browser?"),
        yes|no, no)
    if sel == yes:
      self.quit()

  #def showAbout(self): _MainObject.showWindow(self.__d.aboutDialog)
  #def showHelp(self): _MainObject.showWindow(self.__d.helpDialog)
  #about = showAbout
  #help = showHelp

# MainObject private data
@Q_Q
class _MainObject(object):
  def __init__(self):
    self.hasQuit = False # if the application has quit
    self.widgets = [] # [QWidget]
    #q.destroyed.connect(self._onDestroyed)

  # Helpers
  @staticmethod
  def showWindow(w):
    """
    @param  w  QWidget
    """
    if w.isMaximized() and w.isMinimized():
      w.showMaximized()
    elif w.isMinimized():
      w.showNormal()
    else:
      w.show()
    w.raise_()
    #if not features.WINE:
    #  w.raise_()
    #  winutil.set_foreground_widget(w)

  ## Windows ##

  @memoizedproperty
  def mainWindow(self):
    import mainwindow
    ret = mainwindow.MainWindow()
    ret.quitRequested.connect(self.quit)
    return ret

  # Managers

  @memoizedproperty
  def beanManager(self):
    dprint("create bean manager")
    import beans
    ret = beans.manager()
    ret.setParent(self.q)
    return ret

  @memoizedproperty
  def jlpManager(self):
    dprint("create jlp manager")
    import jlpman, settings
    ret = jlpman.manager()

    reader = settings.reader()
    ret.setRubyType(reader.rubyType())
    ret.setMeCabDicType(reader.meCabDictionary())
    return ret

  @memoizedproperty
  def cacheManager(self):
    dprint("create cache manager")
    import cacheman
    ret = cacheman.manager()
    ret.setParent(self.q)

    #ret.setEnabled(self.networkManager.isOnline())
    #self.networkManager.onlineChanged.connect(ret.setEnabled)

    ret.clearTemporaryFiles()
    return ret

  @memoizedproperty
  def ttsManager(self):
    dprint("create tts manager")
    import ttsman
    ret = ttsman.manager()
    ret.setParent(self.q)

    #ret.setOnline(self.networkManager.isOnline())
    #self.networkManager.onlineChanged.connect(ret.setOnline)

    ss = settings.reader()
    ret.setDefaultEngine(ss.ttsEngine())
    ss.ttsEngineChanged.connect(ret.setDefaultEngine)
    return ret

  # Dialogs

  #@memoizedproperty
  #def aboutDialog(self):
  #  import about
  #  ret = about.AboutDialog(self.mainWindow)
  #  self.widgets.append(ret)
  #  return ret

  #@memoizedproperty
  #def helpDialog(self):
  #  import help
  #  ret = help.AppHelpDialog(self.mainWindow)
  #  self.widgets.append(ret)
  #  return ret

  ## Actions ##

  @debugmethod
  def quit(self):
    if self.hasQuit:
      return

    self.hasQuit = True

    import curtheme
    curtheme.unload()

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
