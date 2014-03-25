# coding: utf8
# rc.py
# 12/13/2012 jichi
# Runtime resource locations

import os
import jinja2
from sakurakit import skos, skpaths
import config

DIR_USER = (config.USER_PROFILES[skos.name]
    .replace('$HOME', skpaths.HOME)
    .replace('$APPDATA', skpaths.APPDATA))

DIR_USER_CACHE = DIR_USER + '/caches'       # $user/caches
DIR_CACHE_NETMAN = DIR_USER_CACHE + '/netman'  # $user/caches/netman
DIR_CACHE_WEBKIT = DIR_USER_CACHE + '/webkit'  # $user/caches/webkit

COOKIE_LOCATION = DIR_CACHE_NETMAN + '/cookies'  # $user/caches/netman/cookies

# Image locations

def icon(name):
  """
  @param  name  str  id
  @return  QIcon
  @throw  KeyError  when unknown name
  """
  from PySide.QtGui import QIcon
  return QIcon(config.ICON_LOCATIONS[name])

def image_path(name):
  """
  @param  name  str  id
  @return  unicode
  @throw  KeyError  when unknown name
  """
  return config.IMAGE_LOCATIONS[name]

def image_url(name):
  """
  @param  name  str  id
  @return  unicode
  @throw  KeyError  when unknown name
  """
  from PySide.QtCore import QUrl
  return QUrl.fromLocalFile(
      os.path.abspath(image_path(name))).toString()

def qss_path(name):
  """
  @param  name  str  id
  @return  unicode
  @throw  KeyError  when unknown name
  """
  return config.QSS_LOCATIONS[name]

def qss(name):
  """
  @param  name  str  id
  @return  unicode
  @throw  KeyError  when unknown name
  """
  from sakurakit import skfileio
  return skfileio.readfile(qss_path(name)).replace('$PWD', config.root_abspath())

# HAML Jinja

__jinja_loader = jinja2.FileSystemLoader(config.TEMPLATE_LOCATION)
JINJA = jinja2.Environment(
  loader = __jinja_loader,
  auto_reload = False,  # do not check if the template file is modified
  extensions = config.JINJA_HAML['extensions'],
)

JINJA_TEMPLATES = {} # {str name:jinja_template}
def jinja_template(name):
  """
  @param  name  str  id
  @return  jinjia2.template  path
  @throw  KeyError  when unknown name
  """
  key = 'haml/browser/' + name
  ret = JINJA_TEMPLATES.get(key)
  if not ret:
    ret = JINJA_TEMPLATES[key] = JINJA.get_template(config.TEMPLATE_ENTRIES[key])
  return ret

# EOF
