# coding: utf8
# termview.py
# 1/10/2015 jichi

from sakurakit.skdebug import dprint, dwarn
from PySide.QtCore import QObject, Signal, Slot

class TermViewBean(QObject):

  instance = None

  searchText = None # dict or None
  searchCol = None # dict or None

  def __init__(self, parent=None):
    super(TermViewBean, self).__init__(parent)
    TermViewBean.instance = self
    dprint("pass")

  searchRequested = Signal(unicode, unicode) # text, col

  @Slot(result=unicode)
  def getSearchText(self): return self.searchText
  @Slot(result=unicode)
  def getSearchCol(self): return self.searchCol

def search(text="", col=""):
  """
  @param  text  unicode
  @param  col  str
  """
  if TermViewBean.instance: # width & height are ignored
    TermViewBean.instance.searchRequested.emit(text, col)
  else:
    TermViewBean.searchText = text
    TermViewBean.searchCol = col

    dwarn("termview is not ready")

# EOF
