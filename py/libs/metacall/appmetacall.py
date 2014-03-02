# coding: utf8
# appmetacall.py
# 11/20/2012 jichi
#
# IPC metacall for applications

from PySide.QtCore import Signal, Qt
from pymetacall import MetaCallPropagator

class AppMetaCallDelegate(MetaCallPropagator):
  def __init__(self, parent=None):
    super(AppMetaCallDelegate, self).__init__(parent)
    self.q_arguments.connect(self.arguments, Qt.QueuedConnection)
    self.q_activated.connect(self.activated, Qt.QueuedConnection)

  arguments = Signal(list) # [str] as sys.argv
  q_arguments = Signal(list) # queued

  activated = Signal()
  q_activated = Signal() # queued

DELEGATE = None
def delegate():
  """App metacall server delegate
  @return  AppMetaCallDelegate
  """
  global DELEGATE
  if not DELEGATE:
    DELEGATE = AppMetaCallDelegate()
  return DELEGATE

def start_server(port, async=True):
  """
  @param  port  int
  @param  async  bool
  @return  bool
  """
  return delegate().startServer('127.0.0.1', port)

def start_client(port, async=True):
  """
  @param  port  int
  @param  async  bool
  @return  bool
  """
  if delegate().startClient('127.0.0.1', port):
    if not async:
      DELEGATE.waitForReady()
  return DELEGATE.isActive()

def active():
  return bool(DELEGATE) and DELEGATE.isActive()

def stop():
  try: DELEGATE.stop()
  except AttributeError: pass

def open_(argv):
  """
  @param  argv  [str]  sys.argv
  """
  try: DELEGATE.q_arguments.emit(argv)
  except AttributeError: pass

def activate():
  """Show and raise the app main window"""
  try: DELEGATE.q_activated.emit()
  except AttributeError: pass

# EOF
