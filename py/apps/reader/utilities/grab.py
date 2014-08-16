# coding: utf8
# grab.py
# 12/6/2012 jichi

import os
from datetime import datetime
from PySide.QtGui import QPixmap
from sakurakit import skclip, skpaths, skwidgets

def _savepixmap(pm, path=None, format='png', clipboard=True, quality=-1):
  """
  @param  pm  QPixmap
  @param* path  unicode
  @param* format  str
  @param* clipboard  bool
  @param* quality  int
  @return  bool
  """
  ok = not pm.isNull()
  if ok:
    if clipboard:
      skclip.setpixmap(pm)
    if not path:
      ts = datetime.now().strftime("%H%M%S")
      fileName = "vnr-capture-%ix%i-%s.%s" % (pm.width(), pm.height(), ts, format)
      path = os.path.join(skpaths.DESKTOP, fileName)
    ok = pm.save(path, format, quality)
  return ok

def window(wid, x=0, y=0, width=-1, height=-1, **kwargs):
  """
  @param  wid  WID
  @param* x  int
  @param* y  int
  @param* width  int
  @param* height  int

  @param* path  unicode
  @param* format  str
  @param* clipboard  bool  whether save to clipboard
  @param* quality  int
  @return  bool
  """
  if not wid:
    return
  pm = QPixmap.grabWindow(skwidgets.to_wid(wid), x, y, width, height)
  return _savepixmap(pm, **kwargs)

def widget(w, x=0, y=0, width=-1, height=-1, **kwargs):
  """
  @param  w  QWidget
  @param* x  int
  @param* y  int
  @param* width  int
  @param* height  int

  @param* path  unicode
  @param* format  str
  @param* clipboard  bool  whether save to clipboard
  @param* quality  int
  @return  bool
  """
  if not w:
    return
  pm = QPixmap.grabWidget(w, x, y, width, height)
  return _savepixmap(pm, **kwargs)

def desktop(*args, **kwargs):
  """
  @param* x  int
  @param* y  int
  @param* width  int
  @param* height  int

  @param* path  unicode
  @param* format  str
  @param* clipboard  bool  whether save to clipboard
  @param* quality  int
  @return  bool
  """
  from PySide.QtCore import QApplication
  qApp = QApplication.instance()
  return bool(qApp) and widget(qApp.desktop(), *args, **kwargs)

# EOF
