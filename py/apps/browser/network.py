# coding: utf8
# webbrowser.py
# 12/13/2012 jichi

__all__ = ['WbNetworkAccessManager']

from PySide.QtNetwork import QNetworkAccessManager

class WbNetworkAccessManager(QNetworkAccessManager):
  def __init__(self, parent=None):
    super(WbNetworkAccessManager, self).__init__(parent)

# EOF
