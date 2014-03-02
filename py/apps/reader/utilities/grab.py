# coding: utf8
# grab.py
# 12/6/2012 jichi

import os
from datetime import datetime
from PySide.QtGui import QPixmap
from sakurakit import skclip, skpaths, skwidgets

#def widget(widget, path=None, x=0, y=0, width=-1, height=-1, format='png', clip=True, quality=-1):
#  pm = QPixmap.grabWidget(wid, x, y, width, height)

def window(wid, path=None, x=0, y=0, width=-1, height=-1, format='png', clip=True, quality=-1):
  if not wid:
    return
  pm = QPixmap.grabWindow(skwidgets.to_wid(wid),
      x, y, width, height)
  ok = not pm.isNull()
  if ok:
    if clip:
      skclip.setpixmap(pm)

    if not path:
      ts = datetime.now().strftime("%H%M%S")
      fileName = "vnr-capture-%ix%i-%s.%s" % (pm.width(), pm.height(), ts, format)
      path = os.path.join(skpaths.DESKTOP, fileName)
    ok = pm.save(path, format, quality)
  return ok

# EOF
