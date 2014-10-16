# coding: utf8
# skevents.py
# 10/31/2012 jichi

from PySide import QtCore
from PySide.QtCore import Qt
from skdebug import debugfunc

def runlater(slot, interval=0):
  """
  @param  slot not None  callback function
  @param  interval  int  time in msecs
  @param  autoQuit  bool  if quit eventloop when qApp.aboutToQuit
  """
  QtCore.QTimer.singleShot(interval, slot)

@debugfunc
def waitsignal(signal, type=Qt.AutoConnection,
    abortSignal=None, autoQuit=True):
  """
  @param  signal  Signal not None
  @param* type  Qt.ConnectionType
  @param* abortSignal  Signal or None  signal with auto type
  @param* autoQuit  bool  if quit eventloop when qApp.aboutToQuit
  """
  loop = QtCore.QEventLoop()
  signal.connect(loop.quit, type)

  if abortSignal:
    abortSignal.connect(loop.quit)

  # Make sure the eventloop quit before closing
  if autoQuit:
    qApp = QtCore.QCoreApplication.instance()
    qApp.aboutToQuit.connect(loop.quit)

  loop.exec_()

  # FIXME 10/14/2014: Why disconnect will cause crash?!
  #signal.disconnect(loop.quit)
  #if abortSignal:
  #  abortSignal.disconnect(loop.quit)
  #if autoQuit:
  #  qApp.aboutToQuit.disconnect(loop.quit)

  #if parent:
  #  runlater(loop.deleteLater)

@debugfunc
def waitsignals(signals, type=Qt.AutoConnection, autoQuit=True):
  """
  @param  signals  [Signal] or None
  @param* type  Qt.ConnectionType
  @param* autoQuit  bool  if quit eventloop when qApp.aboutToQuit
  """
  if not signals:
    return

  loop = QtCore.QEventLoop()
  #if isinstance(signals, QtCore.Signal):
  #  signals.connect(loop.quit, type)
  for sig in signals:
    sig.connect(loop.quit, type)
  if autoQuit:
    qApp = QtCore.QCoreApplication.instance()
    qApp.aboutToQuit.connect(loop.quit)

  loop.exec_()

  #for sig in signals:
  #  sig.disconnect(loop.quit)
  #if autoQuit:
  #  qApp.aboutToQuit.disconnect(loop.quit)

  #if parent:
  #  runlater(loop.deleteLater)

@debugfunc
def sleep(timeout,
    signals=None, type=Qt.AutoConnection, autoQuit=True):
  """
  @param  signals  [Signal] or None
  @param* signals  [Signal] or None
  @param* type  Qt.ConnectionType
  @param* autoQuit  bool  if quit eventloop when qApp.aboutToQuit
  """

  loop = QtCore.QEventLoop()

  timer = QtCore.QTimer()
  timer.setSingleShot(True)
  timer.timeout.connect(loop.quit, type)

  if signals:
    for sig in signals:
      sig.connect(loop.quit, type)

  # Make sure the eventloop quit before closing
  if autoQuit:
    qApp = QtCore.QCoreApplication.instance()
    qApp.aboutToQuit.connect(loop.quit)

  timer.start(timeout)
  loop.exec_()

  #timer.timeout.disconnect(loop.quit)
  #if signals:
  #  for sig in signals:
  #    sig.disconnect(loop.quit)
  #if autoQuit:
  #  qApp.aboutToQuit.disconnect(loop.quit)

  #if parent:
  #  runlater(loop.deleteLater)
  #  runlater(timer.deleteLater)

if __name__ == '__main__':
  def f():
    sleep(1000)

  import sys
  a = QtCore.QCoreApplication(sys.argv)
  f()
  a.exec_()

# EOF
