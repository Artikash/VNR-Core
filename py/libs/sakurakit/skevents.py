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
def waitsignal(signal, type=Qt.AutoConnection, parent=None,
    abortSignal=None, autoQuit=True):
  """
  @param  signal  Signal not None
  @param* type  Qt.ConnectionType
  @param* parent  QObject or None  parent of eventloop
  @param* abortSignal  Signal or None  signal with auto type
  @param* autoQuit  bool  if quit eventloop when qApp.aboutToQuit
  """
  loop = QtCore.QEventLoop(parent)
  signal.connect(loop.quit, type)

  if abortSignal:
    abortSignal.connect(loop.quit)

  # Make sure the eventloop quit before closing
  if autoQuit:
    QtCore.QCoreApplication.instance().aboutToQuit.connect(loop.quit)

  loop.exec_()
  if parent:
    runlater(loop.deleteLater)

@debugfunc
def waitsignals(signals, type=Qt.AutoConnection, parent=None, autoQuit=True):
  """
  @param  signals  [Signal] or None
  @param* type  Qt.ConnectionType
  @param* parent  QObject or None  parent of eventloop
  @param* autoQuit  bool  if quit eventloop when qApp.aboutToQuit
  """
  if not signals:
    return

  loop = QtCore.QEventLoop(parent)
  #if isinstance(signals, QtCore.Signal):
  #  signals.connect(loop.quit, type)
  for sig in signals:
    sig.connect(loop.quit, type)

  # Make sure the eventloop quit before closing
  if autoQuit:
    QtCore.QCoreApplication.instance().aboutToQuit.connect(loop.quit)

  loop.exec_()

  if parent:
    runlater(loop.deleteLater)

@debugfunc
def sleep(timeout, parent=None,
    signals=None, type=Qt.AutoConnection, autoQuit=True):
  """
  @param  signals  [Signal] or None
  @param* parent  QObject or None  parent of eventloop
  @param* signals  [Signal] or None
  @param* type  Qt.ConnectionType
  @param* autoQuit  bool  if quit eventloop when qApp.aboutToQuit
  """

  loop = QtCore.QEventLoop(parent)

  timer = QtCore.QTimer(parent)
  timer.setSingleShot(True)
  timer.timeout.connect(loop.quit, type)

  if signals:
    for sig in signals:
      sig.connect(loop.quit, type)

  # Make sure the eventloop quit before closing
  if autoQuit:
    QtCore.QCoreApplication.instance().aboutToQuit.connect(loop.quit)

  timer.start(timeout)
  loop.exec_()

  if parent:
    runlater(loop.deleteLater)
    runlater(timer.deleteLater)

if __name__ == '__main__':
  def f():
    sleep(1000)

  import sys
  a = QtCore.QCoreApplication(sys.argv)
  f()
  a.exec_()

# EOF
