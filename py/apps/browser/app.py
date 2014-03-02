# coding: utf8
# app.py
# 12/13/2012 jichi

__all__ = ['Application']

from PySide.QtCore import QTranslator
from Qt5.QtWidgets import QApplication
from sakurakit.skdebug import dprint, dwarn
import config

class Application(QApplication):
  def __init__(self, argv):
    super(Application, self).__init__(argv)

    self.setApplicationName("%s - VNR" % self.tr("Web Browser"))
    self.setApplicationVersion(str(config.VERSION_TIMESTAMP))
    self.setOrganizationName(config.VERSION_ORGANIZATION)
    self.setOrganizationDomain(config.VERSION_DOMAIN)

    dprint("pass")

  def loadTranslations(self):
    pass

# EOF
