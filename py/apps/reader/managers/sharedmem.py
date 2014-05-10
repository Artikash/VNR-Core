# coding: utf8
# sharedmem.py
# 5/8/2014 jichi

from PySide.QtCore import Signal, Slot, Property, Qt
from sakurakit.skdebug import dprint, dwarn
from pyvnrmem import VnrSharedMemory
import config


class VnrAgentSharedMemory(VnrSharedMemory):
  # Must be consistent with vnragent
  STATUS_EMPTY = 0
  STATUS_READY = 1
  STATUS_BUSY = 2
  STATUS_CANCEL = 3

  def __init__(self, parent=None):
    super(VnrAgentSharedMemory, self).__init__(parent)
    self.processId = 0 # long

  def detach(self): return self.detach_() # shiboken bug

  def attachProcess(self, pid): # long -> bool
    key = config.VNRAGENT_MEMORY_KEY % pid
    self.setKey(key)
    readOnly = False
    return self.attach(readOnly)

  def detachProcess(self, pid): # long -> bool
    return pid == self.processId and self.isAttached() and self.detach()

  def quit(self):
    if self.isAttached():
      self.setDataStatus(self.STATUS_CANCEL)
      self.detach()

# EOF
