# coding: utf8
# skwebkit.py
# 11/9/2011 jichi

__all__ = ['SkWebViewBean', 'SkWebView', 'SkReadOnlyWebView']

import os
from functools import partial
from PySide.QtCore import Signal, Slot, Qt, Property, QObject, QPoint, QUrl
from PySide.QtGui import QKeySequence
from PySide.QtWebKit import QWebPage, QWebView
from Qt5.QtWidgets import QApplication
import skthreads
from skclass import Q_Q
from skdebug import dprint, dwarn
from skwidgets import shortcut

# Read- only

class SkReadOnlyWebView(QWebView):
  def __init__(self, *args, **kwargs):
    super(SkReadOnlyWebView, self).__init__(*args, **kwargs)
    #ret.setContextMenuPolicy(Qt.NoContextMenu) # disable right click as well
    #self.setAttribute(Qt.WA_TransparentForMouseEvents) # disable all mouse events
    #self.setFocusPolicy(Qt.NoFocus) # ignore keyboard event as well

  def mousePressEvent(self, e): pass
  def mouseReleaseEvent(self, e): pass
  def mouseDoubleClickEvent(self, e): pass

## Views ##

class SkWebPage(QWebPage): # placeholder
  def __init__(self, parent=None):
    super(SkWebPage, self).__init__(parent)

class SkWebView(QWebView):
  def __init__(self, parent=None, f=0, page=None): # QWidget, Qt.WindowFlags, QWebPage
    super(SkWebView, self).__init__(parent)
    if f:
      self.setWindowFlags(f)

    if page:
      self.setPage(page)
    else:
      page = self.page()

    # Refresh
    a = page.action(QWebPage.Reload)
    a.setShortcut(QKeySequence('ctrl+r')) # force CTRL+R on different OSes
    shortcut("ctrl+r", a.trigger, parent=self)

    # Download
    page.downloadRequested.connect(self.downloadRequest)

    # Highlight selection
    #self.selectionChanged.connect(self.highlightSelection)

    # Mouse gestures
    # No effect?
    for g in Qt.PanGesture, Qt.SwipeGesture, Qt.PinchGesture:
      self.grabGesture(g)

  ## Slots ##

  def triggerReloadAction(self):
    # Trigger action instead of directly calling reload
    a = self.page().action(QWebPage.Reload)
    a.trigger()

  ## Highlight ##

  def enableHighlight(self):
    self.selectionChanged.connect(self.highlightSelection)

  def highlightSelection(self):
    #h = self.selectedHtml() # TOOD: parse and highlight ruby text
    t = self.selectedText().strip()
    self.rehighlight(t)

  def clearHighlight(self):
    self.findText('', QWebPage.HighlightAllOccurrences) # clear highlight

  def rehighlight(self, text):
    """
    @param  text  unicode
    """
    self.clearHighlight()
    if text:
      self.highlight(text)

  def highlight(self, text):
    """
    @param  text  unicode
    """
    self.findText(text, QWebPage.FindWrapsAroundDocument|QWebPage.HighlightAllOccurrences)

  ## Download ##

  def downloadRequest(self, req):
    """
    @param  req  QNetworkRequest
    """
    url = req.url()
    dprint(url)
    self.downloadUrl(url)

  @staticmethod
  def downloadUrl(url):
    """
    @param  url  QUrl
    """
    dprint(url)
    import skpaths
    location = skpaths.DESKTOP
    if url.isLocalFile():
      import shutil
      path = url.toLocalFile()
      try: shutil.copy(path, location)
      except Exception, e: dwarn(e)
    else:
      url = url.toString()
      import skfileio, sknetio
      name = sknetio.reqname(url)
      name = skfileio.escape(name or url)
      path = os.path.join(location, name)
      import skthreads
      skthreads.runasync(partial(sknetio.getfile, url, path))

  def clear(self): self.setContent("")

## Beans ##

class SkWebViewBean(QObject):
  def __init__(self, parent=None):
    """
    @parent  QWidget
    """
    super(SkWebViewBean, self).__init__(parent)

  @Slot(result=int)
  def width(self): return self.parent().width()
  @Slot(result=int)
  def height(self): return self.parent().height()

  @Slot(result=int)
  def x(self):
    w = self.parent()
    return w.mapToGlobal(QPoint(w.x(), 0)).x()
  @Slot(result=int)
  def y(self):
    w = self.parent()
    return w.mapToGlobal(QPoint(0, w.y())).y()

  @Slot()
  def reload(self): self.parent().triggerReloadAction()

  @Slot()
  def clearHighlight(self):
    w = self.parent()
    w.clearHighlight()

  @Slot(unicode)
  def highlight(self, text):
    w = self.parent()
    #if isinstance(w, SkWebView)
    w.highlight(text)

  @Slot(unicode)
  def rehighlight(self, text):
    w = self.parent()
    #if isinstance(w, SkWebView)
    w.rehighlight(text)

@Q_Q
class _SkClipboardProxy(object):
  def __init__(self):
    QApplication.clipboard().dataChanged.connect(
        self._onDataChanged)

  def _onDataChanged(self):
    q = self.q
    q.textChanged.emit(q.text)

#@QmlObject
class SkClipboardProxy(QObject):
  def __init__(self, parent=None):
    super(SkClipboardProxy, self).__init__(parent)
    self.__d = _SkClipboardProxy(self)

    QApplication.clipboard().dataChanged.connect(lambda:
        self.textChanged.emit(self.text))

  textChanged = Signal(unicode)
  text = Property(unicode,
      lambda _: QApplication.clipboard().text(),
      lambda _, v: QApplication.clipboard().setText(v),
      notify=textChanged)

# EOF
