# coding: utf8
# googletts.py
# 11/25/2012 jichi
#
# The google TTS will return audio/mpeg layer 3 stream encoded in ADTS.
# This requires QuickTime plugin to play in webkit.
#
# The GoogleTtsEngine class is not threads-safe.

if __name__ == '__main__':
  import sys
  sys.path.append("..") # debug only

from PySide.QtCore import QUrl
#from sakurakit.skdebug import dprint

def defaulturl(): return "http://translate.google.com/translate_tts"
def seturl(url):
  """
  @param  url  str
  """
  global GOOGLE_TTS_API
  GOOGLE_TTS_API = url

GOOGLE_TTS_API = defaulturl()

class _GoogleTtsEngine(object):
  def __init__(self, parent):
    self.warmed = False # if q.say has been invoked at least once
    self._parentWidget = parent
    self._webView = None

  @property
  def webView(self):
    if not self._webView:
      from PySide.QtWebKit import QWebView
      self._webView = QWebView(self.parentWidget)
      update_web_settings(self._webView.settings())
      self._webView.resize(0, 0) # zero size
    return self._webView

  @property
  def parentWidget(self): return self._parentWidget

  @parentWidget.setter
  def parentWidget(self, value):
    self._parentWidget = value
    if self._webView:
      self._webView.setParent(value)

  def stop(self):
    if self._webView:
      self._webView.stop()

class GoogleTtsEngine(object):
  def __init__(self, parent=None):
    self.__d = _GoogleTtsEngine(parent)

  def parentWidget(self):
    """
    @return  QWidget or None
    """
    return self.__d.parentWidget

  def setParentWidget(self, widget):
    """
    @param  widget  QWidget or None
    """
    self.__d.parentWidget = widget

  def stop(self):
    self.__d.stop()

  def warmup(self):
    if not self.__d.warmed:
      self.speak(" ", language='ja') # speak an empty space

  def speak(self, text, language='en', async=False):
    """
    @param  text  str or unicode
    @param  language  str  two-characters lcode such as 'en' or 'ja'
    @param  async  bool  whether put Internet access into another thread
    """
    #dprint("enter: language = %s, async = %s" % (language, async))
    if len(language) > 2:
      language = language[:2] # only 2 characters are kept
    url = QUrl(GOOGLE_TTS_API)
    url.addQueryItem('tl', language)
    url.addEncodedQueryItem('q', QUrl.toPercentEncoding(text))
    self.__d.webView.load(url)
    #dprint(url.toString())
    self.__d.warmed = True
    #dprint("leave")

#@memoized
#def global_(): return GoogleTtsEngine()

def update_web_settings(settings=None):
  """
  @param  settings  QWebSettings or None
  """
  from PySide.QtWebKit import QWebSettings
  ws = settings or QWebSettings.globalSettings()
  ws.setAttribute(QWebSettings.PluginsEnabled, True)
  ws.setAttribute(QWebSettings.JavaEnabled, True)
  ws.setAttribute(QWebSettings.DnsPrefetchEnabled, True) # better performance

  ws.setAttribute(QWebSettings.AutoLoadImages, False) # do NOT load images

  #ws.setAttribute(QWebSettings.JavascriptCanOpenWindows, True)
  #ws.setAttribute(QWebSettings.JavascriptCanAccessClipboard, True)
  #ws.setAttribute(QWebSettings.DeveloperExtrasEnabled, True)

  #ws.setAttribute(QWebSettings.OfflineStorageDatabaseEnabled, True)
  #ws.setAttribute(QWebSettings.OfflineWebApplicationCacheEnabled, True)

  #ws.setAttribute(QWebSettings.LocalStorageEnabled, True)
  #ws.setAttribute(QWebSettings.LocalContentCanAccessRemoteUrls, True)

  #ws.setAttribute(QWebSettings.ZoomTextOnly, False)

  #ws.setDefaultTextEncoding("SHIFT-JIS")
  #ws.setDefaultTextEncoding("EUC-JP")

  #ws.setLocalStoragePath(G_PATH_CACHES)
  #QWebSettings.setIconDatabasePath(G_PATH_CACHES)
  #QWebSettings.setOfflineStoragePath(G_PATH_CACHES)
  #QWebSettings.setOfflineWebApplicationCachePath(G_PATH_CACHES)

  # See: http://webkit.org/blog/427/webkit-page-cache-i-the-basics/
  ws.setMaximumPagesInCache(10) # do not cache lots of pages

if __name__ == '__main__':
  print "enter"
  import sys

  from PySide.QtCore import QTimer
  from PySide.QtGui import QApplication
  from PySide.QtWebKit import QWebView

  q = u"ももももももももももまたもももももももももももともももいろいろ"
  #url = 'http://translate.google.com/translate_tts'
  url = 'http://translate.google.com/translate_tts?tl=ja'
  url = QUrl(url)
  url.addEncodedQueryItem('q', QUrl.toPercentEncoding(q))
  print url.toString()

  update_web_settings()

  a = QApplication(sys.argv)
  w = QWebView()
  w.load(url)
  w.show()

  #q = u"にゃにゃめにゃにゃじゅうにゃにゃどのにゃらびでにゃくにゃくいにゃにゃくにゃにゃはんにゃにゃだいにゃんにゃくにゃらべてにゃがにゃがめ"
  q = u"お花の匂い"
  g = GoogleTtsEngine()
  g.setParentWidget(w)
  QTimer.singleShot(6000, lambda: g.speak(q, language='ja'))

  ret = a.exec_()
  print "leave: ret = %i" % ret
  sys.exit(ret)

# EOF
