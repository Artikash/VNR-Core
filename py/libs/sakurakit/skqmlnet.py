# coding: utf8
# skqmlnet.py
# 2/20/2014 jichi

__all__ = [
  'QmlAjax',
]

import requests
from PySide.QtCore import QObject, Slot
import sknetio
from skdebug import dprint
from skqml import QmlObject

@QmlObject
class QmlAjax(QObject):
  def __init__(self, parent=None):
    super(QmlAjax, self).__init__(parent)

  # TODO: Change to emit signals instead of blocking
  @Slot(unicode, result=unicode)
  def get(self, url):
    dprint(url)
    return sknetio.getdata(url)

# EOF
