# coding: utf8
# siteman.py
# 11/15/2014 jichi

class Site: # Abstract
  def match(self, url): return False # QUrl -> bool

class DmmSite(Site):
  def match(self, url):
    """@override"""
    print url
    return True

SITES = DmmSite,

# EOF
