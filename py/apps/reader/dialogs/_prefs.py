# coding: utf8
# _prefs.py
# 10/28/2012 jichi

import os
from functools import partial
from PySide.QtCore import Qt, QTimer
from PySide import QtCore, QtGui
from Qt5 import QtWidgets
from sakurakit import skevents, skpaths, skqss, skstr, skwidgets
from sakurakit.skclass import Q_Q, memoizedproperty
from sakurakit.skdebug import dprint
from sakurakit.sktr import tr_, notr_
from msime import msime
from dataman import GUEST
from mytr import my, mytr_
import voiceroid.online as vrapi
import voicetext.online as vtapi
import config, cacheman, defs, dicts, ebdict, features, growl, hkman, i18n, info, libman, netman, prompt, ocrman, osutil, rc, res, sapiman, settings, ttsman

def parent_window(): # replace self.q to make sure windows is always visible
  import windows
  return windows.top()

DOWNLOAD_REFRESH_INTERVAL = 3000 # 3 seconds

MECAB_DICT_NAMES = {
  'unidic': my.tr("UniDic modern Japanese dictionary"),
  #'unidic-mlj': my.tr("UniDic classical/ancient Japanese dictionary"),
  'ipadic': my.tr("IPAdic Japanese dictionary"),
}

MECAB_DICT_SIZES = {
  'ipadic': '51MB',
  'unidic': '247MB',
  #'unidic-mlj': '458MB',
}

CABOCHA_DICT_NAMES = {
  'unidic': my.tr("CaboCha model for {0} dictionary").format("UniDic"),
  'ipadic': my.tr("CaboCha model for {0} dictionary").format("IPAdic"),
  #'juman': my.tr("CaboCha model for {0} dictionary").format("JUMAN"),
}

CABOCHA_DICT_SIZES = {
  'ipadic': '81MB',
  'unidic': '75MB',
  #'juman': '78MB',
}

LINGOES_DICT_NAMES = {
  'ja-zh': my.tr("New Japanese-Chinese dictionary"),
  'ja-zh-gbk': my.tr("GBK Japanese-Chinese dictionary"),
  'ja-en': my.tr("Vicon Japanese-English dictionary"),
  'ja-ko': my.tr("Naver Japanese-Korean dictionary"),
  'ja-vi': my.tr("OVDP Japanese-Vietnamese dictionary"),
}

LINGOES_DICT_SIZES = {
  'ja-zh': '129MB',
  'ja-zh-gbk': '2MB',
  'ja-ko': '206MB',
  'ja-vi': '945MB',
  'ja-en': '248MB',
}

JMDICT_DICT_NAMES = {
  'fr': my.tr("JMDict Japanese-French dictionary"),
  'ru': my.tr("JMDict Japanese-Russian dictionary"),
  'nl': my.tr("JMDict Japanese-Dutch dictionary"),
}

# The sizes are to be fixed
JMDICT_DICT_SIZES = {
  'fr': '9MB',
  'ru': '4MB',
  'nl': '21MB',
}

#from sakurakit import skos
#if skos.MAC:
#  KEY_CTRL = "cmd"
#else:
#  KEY_CTRL = "Ctrl"

BROWSE_BTN_CLASS = 'btn btn-info'
CLEAR_BTN_CLASS = 'btn btn-default' #'btn btn-inverse'

# Account

REGISTER_URL = "http://sakuradite.com"

ICON_BUTTON_SIZE = QtCore.QSize(16, 16)

def create_cell_button(): # QPushButton ->
  ret = QtWidgets.QPushButton()
  ret.setMaximumWidth(18)
  ret.setMaximumHeight(18)
  skqss.class_(ret, 'btn btn-default btn-sm')
  return ret

class TabAdaptor(object):
  def save(self): pass
  def load(self): pass
  def refresh(self): pass
  def stop(self): pass

@Q_Q
class _UserTab(object):

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    infoEdit = QtWidgets.QTextBrowser()
    skqss.class_(infoEdit, 'texture')
    #infoEdit.setAlignment(Qt.AlignCenter)
    infoEdit.setReadOnly(True)
    infoEdit.setOpenExternalLinks(True)
    #infoEdit.setMaximumHeight(145)
    infoEdit.setHtml(info.renderAccountHelp())

    registerButton = QtWidgets.QPushButton(tr_("Register"))
    skqss.class_(registerButton, 'btn btn-primary')
    registerButton.setToolTip(my.tr("Get a new account online"))
    registerButton.clicked.connect(partial(
        osutil.open_url, REGISTER_URL))

    guestButton = QtWidgets.QPushButton(tr_("Guest"))
    skqss.class_(guestButton, 'btn btn-success')
    guestButton.setToolTip(my.tr("Switch to guest account"))
    guestButton.clicked.connect(self._loginAsGuest)

    grid = QtWidgets.QGridLayout()

    r = 0
    grid.addWidget(QtWidgets.QLabel(tr_("Avatar") + ":"), r, 0)
    grid.addWidget(self.userAvatarLabel, r, 1)

    r += 1
    grid.addWidget(QtWidgets.QLabel(tr_("Username") + ":"), r, 0)
    grid.addWidget(self.userNameEdit, r, 1)

    r += 1
    grid.addWidget(QtWidgets.QLabel(tr_("Password") + ":"), r, 0)
    grid.addWidget(self.userPasswordEdit, r, 1)

    # Language name is not translated
    r += 1
    grid.addWidget(QtWidgets.QLabel(notr_("Language") + ":"), r, 0)
    grid.addWidget(self.userLanguageEdit, r, 1)

    r += 1
    grid.addWidget(QtWidgets.QLabel(tr_("Gender") + ":"), r, 0)
    grid.addWidget(self.userGenderEdit, r, 1)

    r += 1
    grid.addWidget(QtWidgets.QLabel(tr_("Color") + ":"), r, 0)
    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.userColorButton)
    row.addWidget(self.resetUserColorButton)
    row.addStretch()
    grid.addLayout(row, r, 1)

    r += 1
    grid.addWidget(QtWidgets.QLabel(u"超能力:"), r, 0)
    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.userLevelLabel)
    self._refreshUserLevel()
    row.addStretch()
    grid.addLayout(row, r, 1)

    r += 1
    grid.addWidget(guestButton, r, 0)
    grid.addWidget(registerButton, r, 1)

    group = QtWidgets.QGroupBox(tr_("Account"))
    group.setLayout(grid)

    rows = QtWidgets.QVBoxLayout()
    rows.addWidget(group)
    rows.addWidget(infoEdit)

    rows.addStretch()
    q.setLayout(rows)

  @memoizedproperty
  def userLevelLabel(self):
    ret = QtWidgets.QLabel()
    skqss.class_(ret, 'readonly')
    ss = settings.global_()
    ss.userTermLevelChanged.connect(self._refreshUserLevel)
    ss.userCommentLevelChanged.connect(self._refreshUserLevel)
    return ret

  def _refreshUserLevel(self):
    ss = settings.global_()
    termLevel = ss.userTermLevel()
    commentLevel = ss.userCommentLevel()
    if not termLevel and not commentLevel:
      t = tr_("None")
    else:
      l = []
      if commentLevel > 0:
        l.append("- " + mytr_("Moderate anonymous subs"))
      if termLevel > 0:
        l.append("- " + mytr_("Moderate anonymous terms"))
      t = "\n".join(l)
    self.userLevelLabel.setText(t)

  @memoizedproperty
  def userAvatarLabel(self):
    ret = QtWidgets.QLabel()
    skqss.class_(ret, 'readonly')
    ret.setToolTip(tr_("Avatar"))
    e = QtWidgets.QGraphicsDropShadowEffect()
    #e.setBlurRadius(16)
    #e.setColor('black') # dark green
    #e.setOffset(4, 4)
    #ret.setGraphicsEffect(e)
    def _load():
      av = settings.global_().userAvatar()
      ok = False
      if av:
        path = rc.avatar_image_path(av)
        ok = os.path.exists(path)
        if not ok:
          ok = cacheman.manager().updateAvatar(av)
      if not ok:
        #path = rc.image_path('avatar')
        path = None
      #if not path:
      #  uid = ss.userId()
      #  path = rc.random_avatar_path(uid)
      if path:
        text = '<img src="%s" width=100 height=100 />' % path
      else:
        text = tr_("Not specified")
      ret.setText(text)
    _load()
    settings.global_().userAvatarChanged.connect(_load)
    return ret

  @memoizedproperty
  def resetUserColorButton(self):
    ret = create_cell_button()
    ret.setText(u"×") # ばつ
    ret.setToolTip(tr_("Reset"))
    ret.clicked.connect(self._resetUserColor)
    return ret

  @memoizedproperty
  def userColorButton(self):
    ret = QtWidgets.QPushButton()
    skqss.class_(ret, 'transp')
    ret.setToolTip(my.tr("Click to change the color"))
    ret.setIconSize(ICON_BUTTON_SIZE)
    ret.clicked.connect(lambda:
        self._userCanEdit() and self.userColorDialog.show())
    def _load():
      icon = QtGui.QPixmap(ICON_BUTTON_SIZE)
      icon.fill(settings.global_().userColor() or defs.USER_DEFAULT_COLOR)
      ret.setIcon(icon)
    _load()
    settings.global_().userColorChanged.connect(_load)
    return ret

  @memoizedproperty
  def userColorDialog(self):
    ret = QtWidgets.QColorDialog(settings.global_().userColor() or defs.USER_DEFAULT_COLOR, self.q)
    ret.setWindowTitle(my.tr("Your favorite color"))
    ret.setOption(QtWidgets.QColorDialog.NoButtons)
    ret.currentColorChanged.connect(self._saveColor)
    return ret

  @memoizedproperty
  def userNameEdit(self):
    ret = QtWidgets.QLineEdit()
    skqss.class_(ret, 'normal')
    ret.setPlaceholderText(tr_("Username"))
    #ret.setFixedWidth(200)
    #ret.editingFinished.connect(self._saveLogin)
    ret.setText(settings.global_().userName())
    ret.returnPressed.connect(self._saveLogin)
    return ret

  @memoizedproperty
  def userPasswordEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setEchoMode(QtWidgets.QLineEdit.Password)
    ret.setPlaceholderText(tr_("Password"))
    #ret.setFixedWidth(200)
    ret.setText(settings.global_().userPassword())
    ret.editingFinished.connect(self._saveLogin)
    ret.returnPressed.connect(self._saveLogin)
    return ret

  @memoizedproperty
  def userLanguageEdit(self):
    ret = QtWidgets.QComboBox()
    ret.setEditable(False)
    ret.addItems(map(config.language_name, config.LANGUAGES))
    ret.setMaxVisibleItems(ret.count())
    def _load():
      try: langIndex = config.LANGUAGES.index(settings.global_().userLanguage())
      except ValueError: langIndex = 1 # 'en'
      ret.setCurrentIndex(langIndex)
    _load()
    settings.global_().userLanguageChanged.connect(_load)
    ret.currentIndexChanged.connect(self._saveLanguage)
    return ret

  @memoizedproperty
  def userGenderEdit(self):
    ret = QtWidgets.QComboBox()
    ret.setEditable(False)
    ret.addItems(map(i18n.gender_name, defs.GENDERS))
    def _load():
      try: genderIndex = defs.GENDERS.index(settings.global_().userGender())
      except ValueError: genderIndex = 0
      ret.setCurrentIndex(genderIndex)
    _load()
    settings.global_().userGenderChanged.connect(_load)
    ret.currentIndexChanged.connect(self._saveGender)
    return ret

  def _saveColor(self, color):
    """
    @param  color  QColor
    """
    c = color.name()
    if c == defs.USER_DEFAULT_COLOR:
      c = ''
    settings.global_().setUserColor(c)

  def _userCanEdit(self):
    """
    @return  bool
    """
    import dataman
    if dataman.manager().user().isGuest():
      growl.warn(my.tr("Modification to guest account is not allowed"))
      return False
    elif not netman.manager().isOnline():
      growl.warn(my.tr("Modification requires Internet access"))
      return False
    else:
      return True

  def _resetUserColor(self):
    if self._userCanEdit():
      settings.global_().setUserColor('')

  def _loginAsGuest(self):
    self.userNameEdit.setText(GUEST.name)
    self.userPasswordEdit.setText(GUEST.password)
    self._saveLogin()

  def _saveGender(self):
    if self.userGenderEdit.currentIndex() or not settings.global_().userGender():
      gender = defs.GENDERS[self.userGenderEdit.currentIndex()]
      settings.global_().setUserGender(gender)
    else:
      g = settings.global_().userGender()
      try: genderIndex = defs.GENDERS.index(g)
      except IndexError: genderIndex = 1 # male
      self.userGenderEdit.setCurrentIndex(genderIndex)

  def _saveLanguage(self):
    lang = config.LANGUAGES[self.userLanguageEdit.currentIndex()]
    ss = settings.global_()
    if lang != ss.userLanguage():
      if lang in ('en', 'ja', 'zht', 'zhs'):
        growl.notify("<br/>".join((
          my.tr("GUI language changed."),
          my.tr("VNR will use the new language next time."),
        )))
      #elif lang in ('vi', 'id', 'ms', 'th'):
      #  growl.notify("<br/>".join((
      #    my.tr("I am so sorry that"),
      #    my.tr("Currently, {0} is the only machine translator that supports {1}.")
      #      .format(mytr_("Infoseek") + ".co.jp", i18n.language_name(lang)),
      #  )))
      #elif lang == 'ru':
      #  growl.notify("<br/>".join((
      #    my.tr("I am so sorry that"),
      #    my.tr("Currently, {0} is the only machine translator that supports {1}.")
      #      .format(mytr_("Bing"), i18n.language_name(lang)),
      #  )))
      ss.setUserLanguage(lang)

  def _saveLogin(self):
    """
    User color is not saved here.
    """
    ss = settings.global_()

    userName = self.userNameEdit.text().strip()
    password = self.userPasswordEdit.text()
    if userName == GUEST.name:
      password = GUEST.password

    ss.setUserName(userName)
    ss.setUserPassword(password)
    ss.invalidateLogin()

  save = _saveLogin

class UserTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(UserTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _UserTab(self)
    #self.setMinimumSize(300,  400)

  def save(self): self.__d.save()
  def load(self): pass
  def refresh(self): pass

## UI themes ##

@Q_Q
class _UiTab(object):

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    if not features.WINE:
      layout.addWidget(self.mouseGroup)
    layout.addWidget(self.springBoardGroup)
    #layout.addWidget(self.statusGroup)
    layout.addStretch()
    q.setLayout(layout)

  ## Comet ##

  @memoizedproperty
  def statusGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.cometCounterButton)

    ret = QtWidgets.QGroupBox(my.tr("Peer status"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def cometCounterButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Display online user counter on the Spring Board"))
    #ret.setToolTip(my.tr("Automatically change wallpaper for different games"))
    ss = settings.global_()
    ret.setChecked(ss.isCometCounterVisible())
    ret.toggled.connect(ss.setCometCounterVisible)
    return ret

  ## SpringBoard ##

  @memoizedproperty
  def springBoardGroup(self):
    row = QtWidgets.QHBoxLayout()
    row.addWidget(QtWidgets.QLabel(tr_("Wallpaper")))
    row.addWidget(self.springBoardWallpaperEdit)

    b = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(b, BROWSE_BTN_CLASS)
    b.setToolTip(my.tr("Select the location of the picture"))
    b.clicked.connect(self._getSpringBoardLocation)
    row.addWidget(b)

    row.addWidget(self.springBoardWallpaperButton)

    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.springBoardSlidesButton)
    layout.addLayout(row)

    layout.addWidget(QtWidgets.QLabel("\n".join((
my.tr("Just Drag-and-Drop your wallpaper file to the SpringBoard >_<"),
my.tr("But Drag-and-Drop does not work when VNR has admin privilege T_T"),
))))

    ret = QtWidgets.QGroupBox(tr_("Spring Board"))
    ret.setLayout(layout)

    self._refreshSpringBoardWallpaper()
    return ret

  @memoizedproperty
  def springBoardSlidesButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Automatically change wallpaper for different games"))
    #ret.setToolTip(my.tr("Automatically change wallpaper for different games"))
    ss = settings.global_()
    ret.setChecked(ss.isSpringBoardSlidesEnabled())
    ret.toggled.connect(ss.setSpringBoardSlidesEnabled)
    return ret

  @memoizedproperty
  def springBoardWallpaperEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(my.tr("SpringBoard wallpaper location"))
    settings.global_().springBoardWallpaperUrlChanged.connect(
        self._refreshSpringBoardWallpaper)
    return ret

  @memoizedproperty
  def springBoardWallpaperButton(self):
    ret = QtWidgets.QPushButton(tr_("Remove"))
    skqss.class_(ret, 'btn btn-danger')
    ret.setToolTip(my.tr("Reset to the default wallpaper"))
    ret.clicked.connect(partial(
        settings.global_().setSpringBoardWallpaperUrl, ""))
    return ret

  def _getSpringBoardLocation(self):
    import spring
    FILTERS = "%s (*.%s)" % (tr_("Pictures"), " *.".join(spring.IMAGE_SUFFICES))
    path = self.springBoardWallpaperEdit.text()
    path = path if path and os.path.exists(path) else skpaths.HOME
    path, filter = QtWidgets.QFileDialog.getOpenFileName(self.q,
        my.tr("Please select the location of the picture"), path, FILTERS)
    if path:
      url = osutil.path_url(path)
      settings.global_().setSpringBoardWallpaperUrl(url)
      self._refreshSpringBoardWallpaper()

  #def _clearSpringBoardLocation(self):
  #  url = ''
  #  settings.global_().setSpringBoardWallpaperUrl(url)
  #  self._refreshSpringBoardWallpaper()

  def _refreshSpringBoardWallpaper(self):
    url = settings.global_().springBoardWallpaperUrl()
    if not url:
      ok = True
      t = tr_("Default")
    else:
      t = osutil.url_path(url) or url
      ok = os.path.exists(t)
    self.springBoardWallpaperEdit.setText(t)
    skqss.class_(self.springBoardWallpaperEdit, 'normal' if ok else 'error')
    self.springBoardWallpaperButton.setVisible(bool(url))

  ## Mouse ##

  @memoizedproperty
  def mouseGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.cursorThemeButton)
    ret = QtWidgets.QGroupBox(tr_("Mouse"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def cursorThemeButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
      "Customize mouse cursor"
    ))

    ss = settings.global_()
    ret.setChecked(ss.isCursorThemeEnabled())
    ret.toggled.connect(ss.setCursorThemeEnabled)
    return ret

class UiTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(UiTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _UiTab(self)
    #self.setMinimumWidth(330)

  def save(self): pass
  def load(self): pass
  def refresh(self): pass

## Game ##

#@Q_Q
class _GameTab(object):

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    if not features.WINE:
      layout.addWidget(self.launcherGroup)
    layout.addWidget(self.snifferGroup)
    layout.addWidget(self.springBoardGroup)
    layout.addWidget(self.timeZoneGroup)
    #layout.addWidget(self.hcodeGroup) # Always enabled
    layout.addStretch()
    q.setLayout(layout)

  ## Sniffer ##

  @memoizedproperty
  def hcodeGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.hcodeButton)
    layout.addWidget(QtWidgets.QLabel(my.tr(
"""User-defined hook code could help VNR synchronize with the game.
But incorrect hook code might crash VNR during the synchronization.
When that happens, you can temporarily disable the user-defined code,
and then change the code to the correct one in Text Settings.""")))
    ret = QtWidgets.QGroupBox(my.tr("User-defined hook code"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def hcodeButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
      "Enable hook code from other users to recognize text threads"
    ))
    ss = settings.global_()
    ret.setChecked(ss.isHookCodeEnabled())
    ret.toggled.connect(ss.setHookCodeEnabled)
    return ret

  ## Spring board ##

  @memoizedproperty
  def springBoardGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.springBoardButton)

    label = QtWidgets.QLabel(my.tr(
"""When enabled, VNR will launch the game dropped onto the Spring Board.
When disabled, VNR will only add the new game without launching it.
Drag-drop does not work when VNR is launched with admin privileges."""))
    if features.ADMIN == True:
      skqss.class_(label, 'warning')
    layout.addWidget(label)

    ret = QtWidgets.QGroupBox(mytr_("Spring Board"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def springBoardButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
      "Launch the game (*.exe/*.lnk) dropped onto the Spring Board"
    ))
    if features.ADMIN == True:
      skqss.class_(ret, 'warning')
    ss = settings.global_()
    ret.setChecked(ss.springBoardLaunchesGame())
    ret.toggled.connect(ss.setSpringBoardLaunchesGame)
    return ret

  ## Time zone ##

  @memoizedproperty
  def timeZoneGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.timeZoneButton)
    ret = QtWidgets.QGroupBox(tr_("Time zone"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def timeZoneButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
      "Launch game in Japanese Time Zone"
    ) + " (UTC+09)")
    ss = settings.global_()
    ret.setChecked(ss.isTimeZoneEnabled())
    ret.toggled.connect(ss.setTimeZoneEnabled)
    return ret

  ## Sniffer ##

  @memoizedproperty
  def snifferGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.snifferButton)
    layout.addWidget(QtWidgets.QLabel(my.tr(
"""Whether VNR should automatically sync with the running game it knows.
It is equivalent to clicking the Launch icon docked in Spring Board.
But if you have a slow laptop, enabling it might slow down Windows.""")))
    ret = QtWidgets.QGroupBox(my.tr("Game detection"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def snifferButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
      "Automatically detect running game"
    ))
    ss = settings.global_()
    ret.setChecked(ss.isGameDetectionEnabled())
    ret.toggled.connect(ss.setGameDetectionEnabled)
    return ret

  ## Launcher ##

  @memoizedproperty
  def launcherGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.disableButton)
    layout.addWidget(self.applocButton)
    layout.addWidget(self.localeEmulatorButton)
    layout.addWidget(self.ntleasButton)
    layout.addWidget(self.ntleaButton)
    layout.addWidget(self.localeSwitchButton)
    #layout.addWidget(self.launchInfoLabel)
    ret = QtWidgets.QGroupBox(my.tr("Preferred game loader"))
    ret.setLayout(layout)
    self._loadLauncher()
    return ret

  #@memoizedproperty
  #def launchInfoLabel(self):
  #  ret = QtWidgets.QLabel(my.tr(
  #    "When embedding translation is enabled, if the game's encoding is SHIFT-JIS and your language is not SHIFT-JIS compatible, VNR will always launch the game using AppLocale"
  #   ))
  #  ret.setWordWrap(True)
  #  return ret

  @memoizedproperty
  def disableButton(self):
    ret = QtWidgets.QRadioButton("%s (%s)" % (
         my.tr("Launch the game WITHOUT loader"),
         my.tr("for Windows with Japanese locale")))
    ret.toggled.connect(self._saveLauncher)
    return ret

  @memoizedproperty
  def applocButton(self):
    ret = QtWidgets.QRadioButton("%s (%s, %s)" % (
        my.tr("Use {0} to change game locale").format(notr_("AppLocale")),
        tr_("download"), tr_("default")))
    ret.toggled.connect(self._saveLauncher)
    return ret

  @memoizedproperty
  def ntleaButton(self):
    ret = QtWidgets.QRadioButton("%s (%s)" % (
        my.tr("Use {0} to change game locale").format(notr_("NTLEA")),
        mytr_("built-in")))
    ret.toggled.connect(self._saveLauncher)
    return ret

  @memoizedproperty
  def localeSwitchButton(self):
    ret = QtWidgets.QRadioButton("%s (%s, %s)" % (
        my.tr("Use {0} to change game locale").format(notr_("LocaleSwitch")),
        mytr_("built-in"), my.tr("need admin priv")))
    if features.ADMIN == False:
      skqss.class_(ret, 'warning')
    ret.toggled.connect(self._saveLauncher)
    return ret

  @memoizedproperty
  def localeEmulatorButton(self):
    ret = QtWidgets.QRadioButton("%s (%s, %s)" % (
        my.tr("Use {0} to change game locale").format(notr_("Locale Emulator")),
        tr_("download"), tr_("recommended")))
    #if features.ADMIN == False:
    #  skqss.class_(ret, 'warning')
    ret.toggled.connect(self._saveLauncher)
    return ret

  @memoizedproperty
  def ntleasButton(self):
    ret = QtWidgets.QRadioButton("%s (%s)" % (
        my.tr("Use {0} to change game locale").format(notr_("Ntleas")),
        tr_("download")))
    #if features.ADMIN == False:
    #  skqss.class_(ret, 'warning')
    ret.toggled.connect(self._saveLauncher)
    return ret

  def _loadLauncher(self):
    ss = settings.global_()
    b = (self.applocButton if ss.isApplocEnabled() else
         self.ntleaButton if ss.isNtleaEnabled() else
         self.ntleasButton if ss.isNtleasEnabled() else
         self.localeSwitchButton if ss.isLocaleSwitchEnabled() else
         self.localeEmulatorButton if ss.isLocaleEmulatorEnabled() else
         self.disableButton)
    if not b.isChecked():
      b.setChecked(True)

  def _saveLauncher(self):
    ss = settings.global_()
    ss.setApplocEnabled(self.applocButton.isChecked())
    ss.setNtleaEnabled(self.ntleaButton.isChecked())
    ss.setNtleasEnabled(self.ntleasButton.isChecked())
    ss.setLocaleSwitchEnabled(self.localeSwitchButton.isChecked())
    ss.setLocaleEmulatorEnabled(self.localeEmulatorButton.isChecked())

  def refresh(self):
    self.localeEmulatorButton.setEnabled(libman.localeEmulator().exists())
    self.ntleasButton.setEnabled(libman.ntleas().exists())
    self.localeSwitchButton.setEnabled(features.ADMIN != False)

class GameTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(GameTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _GameTab(self)
    #self.setMinimumWidth(330)

  def save(self): pass
  def load(self): pass
  def refresh(self): self.__d.refresh()

## Shortcuts ##

@Q_Q
class _ShortcutsTab(object):

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    if features.WINE:
      label = QtWidgets.QLabel(my.tr("Global shortcuts are not supported in Wine."))
      l = skwidgets.SkWidgetLayout(label)
      w = QtWidgets.QGroupBox(tr_("About"))
      w.setLayout(l)
      layout.addWidget(w)
    else:
      layout.addWidget(self.globalGroup)
    layout.addStretch()
    q.setLayout(layout)

  ## Global shortcuts ##

  @memoizedproperty
  def globalGroup(self):
    grid = QtWidgets.QGridLayout()

    r = 0
    grid.addWidget(self.textButton, r, 0)
    grid.addWidget(self.textCheckBox, r, 1)
    r += 1
    grid.addWidget(self.ttsButton, r, 0)
    grid.addWidget(self.ttsCheckBox, r, 1)
    r += 1
    grid.addWidget(self.grabButton, r, 0)
    grid.addWidget(self.grabCheckBox, r, 1)
    r += 1

    layout = QtWidgets.QVBoxLayout()
    layout.addLayout(grid)
    infoLabel = QtWidgets.QLabel(my.tr("These shortcuts can only be enabled when the game is running."))
    infoLabel.setWordWrap(True)
    layout.addWidget(infoLabel)
    ret = QtWidgets.QGroupBox(my.tr("Global game shortcuts"))
    ret.setLayout(layout)
    return ret

  # Grab

  @memoizedproperty
  def grabButton(self):
    ret = QtWidgets.QPushButton()
    ret.setToolTip(mytr_("Shortcuts"))
    ss = settings.global_()

    def _refresh():
      t = ss.grabHotkey()
      ret.setText(i18n.combined_key_name(t) if t else tr_("Not specified"))
      skqss.class_(ret, 'btn btn-default' if t else 'btn btn-danger')
    _refresh()
    ss.grabHotkeyChanged.connect(_refresh)

    ret.clicked.connect(lambda: (
        self.grabDialog.setValue(ss.grabHotkey()),
        self.grabDialog.show()))

    ret.setEnabled(self.grabCheckBox.isChecked())
    self.grabCheckBox.toggled.connect(ret.setEnabled)
    return ret

  @memoizedproperty
  def grabCheckBox(self):
    ret = QtWidgets.QCheckBox(my.tr("Take a screenshot of the game window"))
    ss = settings.global_()
    ret.setChecked(ss.isGrabHotkeyEnabled())
    ret.toggled.connect(ss.setGrabHotkeyEnabled)
    ret.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
    return ret

  @memoizedproperty
  def grabDialog(self):
    import hkinput
    ret = hkinput.HotkeyInputDialog(parent_window())
    ret.setWindowTitle("%s - %s" % (
      ret.windowTitle(), tr_("Screenshot")))
    ss = settings.global_()
    ret.valueChanged.connect(ss.setGrabHotkey)
    return ret

  # TTS

  @memoizedproperty
  def ttsButton(self):
    ret = QtWidgets.QPushButton()
    ret.setToolTip(mytr_("Shortcuts"))
    ss = settings.global_()

    def _refresh():
      t = ss.ttsHotkey()
      ret.setText(i18n.combined_key_name(t) if t else tr_("Not specified"))
      skqss.class_(ret, 'btn btn-default' if t else 'btn btn-danger')
    _refresh()
    ss.ttsHotkeyChanged.connect(_refresh)

    ret.clicked.connect(lambda: (
        self.ttsDialog.setValue(ss.ttsHotkey()),
        self.ttsDialog.show()))

    ret.setEnabled(self.ttsCheckBox.isChecked())
    self.ttsCheckBox.toggled.connect(ret.setEnabled)
    return ret

  @memoizedproperty
  def ttsCheckBox(self):
    ret = QtWidgets.QCheckBox(my.tr("Speak the current game text using TTS"))
    ss = settings.global_()
    ret.setChecked(ss.isTtsHotkeyEnabled())
    ret.toggled.connect(ss.setTtsHotkeyEnabled)
    ret.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
    return ret

  @memoizedproperty
  def ttsDialog(self):
    import hkinput
    ret = hkinput.HotkeyInputDialog(self.q)
    ret.setWindowTitle("%s - %s" % (
      ret.windowTitle(), mytr_("Text-to-speech")))
    ss = settings.global_()
    ret.valueChanged.connect(ss.setTtsHotkey)
    return ret

  # Textbox visible toggler
  @memoizedproperty
  def textButton(self):
    ret = QtWidgets.QPushButton()
    ret.setToolTip(mytr_("Shortcuts"))
    ss = settings.global_()

    def _refresh():
      t = ss.textHotkey()
      ret.setText(i18n.combined_key_name(t) if t else tr_("Not specified"))
      skqss.class_(ret, 'btn btn-default' if t else 'btn btn-danger')
    _refresh()
    ss.textHotkeyChanged.connect(_refresh)

    ret.clicked.connect(lambda: (
        self.textDialog.setValue(ss.textHotkey()),
        self.textDialog.show()))

    ret.setEnabled(self.textCheckBox.isChecked())
    self.textCheckBox.toggled.connect(ret.setEnabled)
    return ret

  @memoizedproperty
  def textCheckBox(self):
    ret = QtWidgets.QCheckBox(my.tr("Toggle visiblility of the text box"))
    ss = settings.global_()
    ret.setChecked(ss.isTextHotkeyEnabled())
    ret.toggled.connect(ss.setTextHotkeyEnabled)
    ret.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
    return ret

  @memoizedproperty
  def textDialog(self):
    import hkinput
    ret = hkinput.HotkeyInputDialog(self.q)
    ret.setWindowTitle("%s - %s" % (
      ret.windowTitle(), my.tr("Toggle text box")))
    ss = settings.global_()
    ret.valueChanged.connect(ss.setTextHotkey)
    return ret

class ShortcutsTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(ShortcutsTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _ShortcutsTab(self)
    #self.setMinimumWidth(330)

  def save(self): pass
  def load(self): pass
  def refresh(self): pass

## TTS ##

@Q_Q
class _TtsTab(object):

  def __init__(self, q):
    self.onlineWidgets = [] # [QWidget] any widget that involves online TTS
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.testGroup)
    layout.addWidget(self.engineGroup)
    layout.addStretch()
    q.setLayout(layout)

  ## Test ##

  @memoizedproperty
  def testGroup(self):
    layout = QtWidgets.QVBoxLayout()

    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.testButton)
    row.addWidget(self.testLanguageButton)
    row.addWidget(self.testEdit)
    layout.addLayout(row)

    ret = QtWidgets.QGroupBox() #tr_("Test"))
    #self.disableButton.toggled.connect(lambda value:
    #    ret.setEnabled(not value))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def testEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setText(u"お花の匂い")
    skqss.class_(ret, 'normal')
    return ret

  @memoizedproperty
  def testButton(self):
    ret = QtWidgets.QPushButton()
    ret.setText(tr_("Test"))
    ret.setToolTip(my.tr("Read using the selected TTS engine"))
    skqss.class_(ret, 'btn btn-success')
    ret.clicked.connect(self._test)
    return ret

  @memoizedproperty
  def testLanguageButton(self):
    ret = QtWidgets.QComboBox()
    ret.setToolTip(tr_("Language"))
    ret.addItems(map(i18n.language_name2, config.LANGUAGES2))
    ret.setMaxVisibleItems(ret.count())
    return ret

  def _testLanguage(self): # -> str or None
    index = self.testLanguageButton.currentIndex()
    try: return config.LANGUAGES2[index]
    except IndexError: pass

  def _test(self, engine=''):
    """
    @param  engine  unicode
    """
    t = self.testEdit.text().strip()
    if t:
      lang = self._testLanguage()
      ttsman.speak(t, engine=engine, language=lang, verbose=True)

  def createTestButton(self, engine='', parent=None):
    """
    @param  engine  str
    @return  QPushButton
    """
    ret = QtWidgets.QPushButton(parent or self.q)
    ret.setText(tr_("Test"))
    ret.setToolTip(my.tr("Read using this TTS"))
    #skqss.class_(ret, 'btn btn-success')
    skqss.class_(ret, 'btn btn-default')
    ret.clicked.connect(partial(self._test, engine))
    return ret

  def createVoiceroidLaunchButton(self, engine, parent=None):
    """
    @param  engine  str
    """
    ret = QtWidgets.QPushButton(parent or self.q)
    ret.setText(tr_("Launch"))
    ret.setToolTip(tr_("Launch"))
    #skqss.class_(ret, 'btn btn-success')
    skqss.class_(ret, 'btn btn-default')
    run = getattr(ttsman.manager(), 'run' + engine.capitalize())
    ret.clicked.connect(run)
    return ret

  # TTS ##

  @memoizedproperty
  def engineGroup(self):
    grid = QtWidgets.QGridLayout()

    r = 0
    grid.addWidget(self.disableButton, r, 1)

    # Multilingual
    r += 1
    grid.addWidget(QtWidgets.QLabel("%s (%s):" % (mytr_("Multilingual"), tr_("online"))), r, 0, 1, 2)

    for k,b,url in ((
        ('baidu', self.baiduButton, "http://fanyi.baidu.com"),
        ('google', self.googleButton, "http://translate.google.com"),
        ('bing', self.bingButton, "http://www.bing.com/translator"),
      )):
      r += 1
      c = 0
      w = self._createBrowseButton(url)
      self.onlineWidgets.append(w)
      grid.addWidget(w, r, c)

      c += 1
      self.onlineWidgets.append(b)
      grid.addWidget(b, r, c)

      c += 1
      w = self.createTestButton(k)
      self.onlineWidgets.append(w)
      grid.addWidget(w, r, c)

      c += 1
      w = self.createVolumeEdit(k)
      self.onlineWidgets.append(w)
      grid.addWidget(w, r, c)

      c += 1
      w = self.createSpeedEdit(k)
      self.onlineWidgets.append(w)
      grid.addWidget(w, r, c)

    # Voiceroid
    r += 1
    grid.addWidget(QtWidgets.QLabel("VOICEROID+ (%s):" % tr_("online")), r, 0, 1, 2)

    for i,(k,b) in enumerate(self.voiceroidButtons):
      v = vrapi.VOICES[k]
      r += 1
      c = 0
      w = self._createBrowseButton(v.url)
      self.onlineWidgets.append(w)
      grid.addWidget(w, r, c)

      c += 1
      self.onlineWidgets.append(b)
      grid.addWidget(b, r, c)

      c += 1
      w = self.createTestButton(k)
      self.onlineWidgets.append(w)
      grid.addWidget(w, r, c)

      c += 1
      w = self.createVolumeEdit(k)
      self.onlineWidgets.append(w)
      grid.addWidget(w, r, c)

      c += 1
      w = self.createSpeedEdit(k)
      self.onlineWidgets.append(w)
      grid.addWidget(w, r, c)

      c += 1
      w = self.createPitchEdit(k)
      self.onlineWidgets.append(w)
      grid.addWidget(w, r, c)

      if not i: # add span for the first item
        c += 1
        span = QtWidgets.QWidget()
        span.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
        grid.addWidget(span, r, c)

    # VoiceText
    r += 1
    grid.addWidget(QtWidgets.QLabel("VoiceText (%s):" % tr_("online")), r, 0, 1, 2)

    url = "http://voicetext.jp"
    blans = settings.global_().blockedLanguages()
    for k,b in self.voicetextButtons:
      v = vtapi.VOICES[k]
      if v.language not in blans:
        r += 1
        c = 0
        w = self._createBrowseButton(url)
        self.onlineWidgets.append(w)
        grid.addWidget(w, r, c)

        c += 1
        self.onlineWidgets.append(b)
        grid.addWidget(b, r, c)

        c += 1
        w = self.createTestButton(k)
        self.onlineWidgets.append(w)
        grid.addWidget(w, r, c)

        c += 1
        w = self.createVolumeEdit(k)
        self.onlineWidgets.append(w)
        grid.addWidget(w, r, c)

        c += 1
        w = self.createSpeedEdit(k)
        self.onlineWidgets.append(w)
        grid.addWidget(w, r, c)

        c += 1
        w = self.createPitchEdit(k)
        self.onlineWidgets.append(w)
        grid.addWidget(w, r, c)

    # SAPI
    if not self.sapiButtons:
      r += 1
      grid.addWidget(QtWidgets.QLabel("SAPI (%s)" % mytr_("not installed")), r, 0, 1, 2)
    else:
      r += 1
      label = QtWidgets.QLabel("SAPI (%s):" % tr_("offline"))
      if features.ADMIN:
        skqss.class_(label, 'text-error')
      grid.addWidget(label, r, 0, 1, 2)
      for k,b in self.sapiButtons:
        r += 1
        c = 1
        if features.ADMIN:
          skqss.class_(b, 'text-error')
        grid.addWidget(b, r, c)
        w = self.createTestButton(k)

        c += 1
        grid.addWidget(w, r, c)

        c += 1
        w = self.createVolumeEdit(k)
        grid.addWidget(w, r, c)

        c += 1
        w = self.createSpeedEdit(k)
        grid.addWidget(w, r, c)

        c += 1
        w = self.createPitchEdit(k)
        grid.addWidget(w, r, c)

    # VOICEROID
    r += 1
    grid.addWidget(QtWidgets.QLabel("VOICEROID+ (%s):" % tr_("offline")), r, 0, 1, 2)

    for k,b,bb in ((
        ('yukari', self.yukariButton, self.yukariBrowseButton),
        ('zunko', self.zunkoButton, self.zunkoBrowseButton),
      )):
      koffline = k + 'offline'
      r += 1
      c = 0
      self.onlineWidgets.append(bb)
      grid.addWidget(bb, r, c)

      c += 1
      grid.addWidget(b, r, c)

      c += 1
      w = self.createTestButton(koffline)
      setattr(self, k + 'TestButton', w)
      grid.addWidget(w, r, c)

      c += 1
      w = self.createVoiceroidLaunchButton(k)
      setattr(self, k + 'LaunchButton', w)
      grid.addWidget(w, r, c)

    layout = QtWidgets.QVBoxLayout()
    layout.addLayout(grid)

    if self.sapiButtons:
      info = QtWidgets.QLabel(tr_("Note") + ": " + my.tr("SAPI TTS won't work when VNR is launched as Administrator."))
      info.setWordWrap(True)
      if features.ADMIN:
        skqss.class_(info, 'text-error')
      layout.addWidget(info)

    ret = QtWidgets.QGroupBox(my.tr("Preferred text-to-speech voice"))
    ret.setLayout(layout)
    self._loadEngine()
    return ret

  def _createBrowseButton(self, path): # unicode or function -> QPushButton
    ret = create_cell_button()
    ret.setText("+")
    if isinstance(path, str) or isinstance(path, unicode):
      ret.setToolTip("%s: %s" % (tr_("Browse"), path))
      open = osutil.open_url if path.startswith('http') else osutil.open_url
      slot = partial(open, path)
    else:
      ret.setToolTip(tr_("Browse"))
      slot = partial(lambda path: osutil.open_location(path()), path)
    ret.clicked.connect(slot)
    return ret

  @memoizedproperty
  def disableButton(self):
    ret = QtWidgets.QRadioButton(my.tr("Disable TTS"))
    ret.toggled.connect(self._saveEngine)
    return ret

  @memoizedproperty
  def yukariButton(self):
    ret = QtWidgets.QRadioButton(u"結月ゆかり (♀, %s)" % tr_("ja"))
    ret.toggled.connect(self._saveEngine)
    return ret
  @memoizedproperty
  def yukariBrowseButton(self):
    return self._createBrowseButton(ttsman.manager().yukariLocation)

  @memoizedproperty
  def zunkoButton(self):
    ret = QtWidgets.QRadioButton(u"東北ずん子 (♀, %s)" % tr_("ja"))
    ret.toggled.connect(self._saveEngine)
    return ret
  @memoizedproperty
  def zunkoBrowseButton(self):
    return self._createBrowseButton(ttsman.manager().zunkoLocation)

  @memoizedproperty
  def googleButton(self):
    ret = QtWidgets.QRadioButton(u"Google (♀)")
    ret.toggled.connect(self._saveEngine)
    return ret

  @memoizedproperty
  def baiduButton(self):
    ret = QtWidgets.QRadioButton(u"%s (♀, %s)" % (mytr_("Baidu"), tr_("default")))
    ret.toggled.connect(self._saveEngine)
    return ret

  @memoizedproperty
  def bingButton(self):
    ret = QtWidgets.QRadioButton(u"%s (♀)" % mytr_("Bing"))
    ret.toggled.connect(self._saveEngine)
    return ret

  @memoizedproperty
  def sapiButtons(self):
    """
    @return  [unicode key, QRadioButton]
    """
    ret = []
    for it in sapiman.voices():
      text = "%s (%s, %s)" % (
        it.name or it.key,
        u"♂" if it.gender == 'm' else u"♀",
        i18n.language_name2(it.language),
      )
      b = QtWidgets.QRadioButton(text)
      b.toggled.connect(self._saveEngine)
      ret.append((it.key, b))
    return ret

  @memoizedproperty
  def voiceroidButtons(self):
    """
    @return  [unicode key, QRadioButton]
    """
    ret = []
    for it in vrapi.VOICES.itervalues():
      text = "%s (%s, %s)" % (
        it.name,
        u"♂" if it.gender == 'm' else u"♀",
        tr_("ja"),
      )
      b = QtWidgets.QRadioButton(text)
      b.toggled.connect(self._saveEngine)
      ret.append((it.key, b))
    return ret

  @memoizedproperty
  def voicetextButtons(self):
    """
    @return  [unicode key, QRadioButton]
    """
    ret = []
    for it in vtapi.VOICES.itervalues():
      text = "%s (%s, %s)" % (
        it.name,
        u"♂" if it.gender == 'm' else u"♀",
        i18n.language_name2(it.language),
      )
      b = QtWidgets.QRadioButton(text)
      b.toggled.connect(self._saveEngine)
      ret.append((it.key, b))
    return ret

  @memoizedproperty
  def engineButtons(self):
    """
    @return  {unicode key:QRadioButton}
    """
    ret = {
      '': self.disableButton,

      'google': self.googleButton,
      'baidu': self.baiduButton,
      'bing': self.bingButton,

      'yukarioffline': self.yukariButton,
      'zunkooffline': self.zunkoButton,
    }
    for k,b in self.sapiButtons:
      ret[k] = b
    for k,b in self.voiceroidButtons:
      ret[k] = b
    for k,b in self.voicetextButtons:
      ret[k] = b
    return ret

  def createSpeedEdit(self, key, parent=None):
    ret = QtWidgets.QSpinBox(parent or self.q)
    ret.setToolTip("%s [-10,10]" % tr_("Speed"))
    ret.setRange(-10, 10)
    ret.setSingleStep(1)
    ret.setPrefix(tr_("Speed") + " ")
    tm = ttsman.manager()
    ret.setValue(tm.getSpeed(key))
    ret.valueChanged[int].connect(partial(tm.setSpeed, key))
    return ret

  def createPitchEdit(self, key, parent=None):
    ret = QtWidgets.QSpinBox(parent or self.q)
    ret.setToolTip("%s [-10,10]" % tr_("Pitch"))
    ret.setRange(-10, 10)
    ret.setSingleStep(1)
    ret.setPrefix(tr_("Pitch") + " ")
    tm = ttsman.manager()
    ret.setValue(tm.getPitch(key))
    ret.valueChanged[int].connect(partial(tm.setPitch, key))
    return ret

  def createVolumeEdit(self, key, parent=None):
    ret = QtWidgets.QSpinBox(parent or self.q)
    ret.setToolTip("%s [0,100]" % tr_("Volume"))
    ret.setRange(0, 100)
    ret.setSingleStep(10)
    ret.setPrefix(tr_("Volume") + " ")
    ret.setSuffix("%")
    tm = ttsman.manager()
    ret.setValue(tm.getVolume(key))
    ret.valueChanged[int].connect(partial(tm.setVolume, key))
    return ret

  #@memoizedproperty
  #def googleLanguageEdit(self):
  #  ret = QtWidgets.QComboBox()
  #  ret.setToolTip(tr_("Language"))
  #  ret.addItems(map(i18n.language_name2, config.LANGUAGES2))
  #  ret.setMaxVisibleItems(ret.count())
  #  try: langIndex = config.LANGUAGES2.index(settings.global_().googleTtsLanguage())
  #  except (ValueError, TypeError): langIndex = 1 # 'en'
  #  ret.setCurrentIndex(langIndex)
  #  ret.currentIndexChanged.connect(self._saveGoogleLanguage)
  #  return ret

  #def _saveGoogleLanguage(self):
  #  index = self.googleLanguageEdit.currentIndex()
  #  try: lang = config.LANGUAGES2[index]
  #  except IndexError: lang = 'ja'
  #  settings.global_().setGoogleTtsLanguage(lang)

  def _loadEngine(self):
    v = settings.global_().ttsEngine()
    b = self.engineButtons.get(v) or self.disableButton
    if not b.isChecked():
      b.setChecked(True)

  def _saveEngine(self):
    v = ''
    for k,b in self.engineButtons.iteritems():
      if b.isChecked():
        v = k
    dprint("engine = %s" % v)
    settings.global_().setTtsEngine(v)

  def refresh(self):
    ss = settings.global_()

    #enabled = libman.quicktime().exists()
    enabled = libman.wmp().exists()
    for w in self.onlineWidgets:
      w.setEnabled(enabled)

    path = settings.global_().yukariLocation() or ttsman.manager().yukariLocation()
    enabled = bool(path)
    for w in self.yukariButton, self.yukariTestButton, self.yukariLaunchButton, self.yukariBrowseButton:
      w.setEnabled(enabled)

    path = settings.global_().zunkoLocation() or ttsman.manager().zunkoLocation()
    enabled = bool(path)
    for w in self.zunkoButton, self.zunkoTestButton, self.zunkoLaunchButton, self.zunkoBrowseButton:
      w.setEnabled(enabled)

class TtsTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(TtsTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _TtsTab(self)
    #self.setMinimumWidth(330)

  def save(self): pass
  def load(self): pass
  def refresh(self): self.__d.refresh()

## OCR ##

@Q_Q
class _OcrTab(object):

  def __init__(self, q):
    self.languageButtons = [] # [QCheckBox]
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.ocrGroup)
    layout.addWidget(self.shortcutsGroup)
    layout.addWidget(self.screenGroup)
    layout.addWidget(self.textGroup)
    layout.addWidget(self.languageGroup)
    layout.addStretch()
    q.setLayout(layout)

    ss = settings.global_()
    ocrEnabled = ss.isOcrEnabled() and features.ADMIN != False
    for w in self.screenGroup, self.shortcutsGroup, self.textGroup, self.languageGroup:
      w.setEnabled(ocrEnabled)
      ss.ocrEnabledChanged.connect(w.setEnabled)

  ## Screen group ##

  @memoizedproperty
  def screenGroup(self):
    layout = QtWidgets.QVBoxLayout()

    # Translation wait time
    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.screenRefreshTimeButton)
    row.addWidget(QtWidgets.QLabel(my.tr("sec. <= Refresh interval to capture the screen")))
    row.addStretch()
    layout.addLayout(row)

    layout.addWidget(self.screenInfoLabel)
    ret = QtWidgets.QGroupBox(my.tr("Screen capture options"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def screenInfoLabel(self):
    ret = QtWidgets.QLabel(my.tr(
      "When the interval is smaller, you will wait less time to get the new OCR text. But it will also require more CPU usage."
    ))
    ret.setWordWrap(True)
    return ret

  @memoizedproperty
  def screenRefreshTimeButton(self):
    ret = QtWidgets.QDoubleSpinBox()
    ret.setToolTip("%s: %s sec." % (tr_("Default"), 2))
    ret.setRange(0.1, 10.0)
    ret.setSingleStep(0.1)
    ret.setDecimals(1) # 0.1
    ss = settings.global_()
    ret.setValue(ss.ocrRefreshInterval() / 1000.0)
    ret.valueChanged[float].connect(lambda v: ss.setOcrRefreshInterval(int(v * 1000)))
    return ret

  ## Text option ##

  @memoizedproperty
  def textGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.spaceButton)
    ret = QtWidgets.QGroupBox(my.tr("Text transformation settings"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def spaceButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Insert spaces between words")
        + " (%s: %s)" % (tr_("for example"), "Howareyou! => How are you!"))
    ss = settings.global_()
    ret.setChecked(ss.isOcrSpaceEnabled())
    ret.toggled.connect(ss.setOcrSpaceEnabled)
    return ret

  ## Languages ##

  @memoizedproperty
  def languageGroup(self):
    ss = settings.global_()

    layout = QtWidgets.QVBoxLayout()

    langs = frozenset(ss.ocrLanguages())
    for lang in config.OCR_LANGUAGES:
      b = self._createLanguageButton(lang, lang in langs)
      layout.addWidget(b)

    layout.addWidget(self.languageInfoLabel)

    ret = QtWidgets.QGroupBox(my.tr("Text character languages"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def languageInfoLabel(self):
    ret = QtWidgets.QLabel(my.tr(
      "Characters are matched in the same order as the selected languages."
    ))
    ret.setWordWrap(True)
    #ret.setOpenExternalLinks(True)
    return ret

  def _createLanguageButton(self, lang, t): # str, bool -> QCheckBox
    ret = QtWidgets.QCheckBox(i18n.language_name(lang))
    ret.language = lang
    ret.setChecked(t)
    ret.toggled.connect(self._saveLanguages)
    self.languageButtons.append(ret)
    return ret

  def _saveLanguages(self):
    langs = [b.language for b in self.languageButtons if b.isChecked()]
    settings.global_().setOcrLanguages(langs)

  ## Shortcuts ##

  @memoizedproperty
  def shortcutsGroup(self):
    layout = QtWidgets.QHBoxLayout()

    layout.addWidget(self.comboKeyButton)
    label = QtWidgets.QLabel("<= " + my.tr("Combine with mouse to select OCR region"))
    layout.addWidget(label)
    layout.addStretch()

    #grid = QtWidgets.QGridLayout()
    #r = 0
    #grid.addWidget(self.comboKeyButton, r, 0)
    #label = QtWidgets.QLabel("<= " + my.tr("Combine with mouse to select OCR region"))
    #grid.addWidget(label, r, 1)
    #r += 1

    #layout = QtWidgets.QVBoxLayout()
    #layout.addLayout(grid)
    #infoLabel = QtWidgets.QLabel()
    #infoLabel.setWordWrap(True)
    #layout.addWidget(infoLabel)

    ret = QtWidgets.QGroupBox(my.tr("Keyboard shortcuts"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def comboKeyButton(self):
    ret = QtWidgets.QPushButton()
    ret.setToolTip(mytr_("Shortcuts"))
    ss = settings.global_()

    import vkinput
    def _refresh():
      vk = ss.ocrComboKey()
      ret.setText(vkinput.vk_name(vk) if vk else tr_("Not specified"))
      skqss.class_(ret, 'btn btn-default' if vk else 'btn btn-danger')
    _refresh()
    ss.ocrComboKeyChanged.connect(_refresh)

    ret.clicked.connect(lambda: (
        self.comboKeyDialog.setValue(ss.ocrComboKey()),
        self.comboKeyDialog.show()))
    return ret

  @memoizedproperty
  def comboKeyDialog(self):
    import vkinput
    ret = vkinput.VirtualKeyInputDialog(parent_window())
    ret.setWindowTitle("%s - %s" % (
      ret.windowTitle(), "OCR"))
    ret.setDeletable(False)
    ss = settings.global_()
    ret.valueChanged.connect(ss.setOcrComboKey)
    return ret

  ## OCR ##

  @memoizedproperty
  def ocrGroup(self):
    layout = QtWidgets.QVBoxLayout()

    layout.addWidget(self.modiButton)
    layout.addWidget(self.ocrInfoLabel)
    layout.addWidget(self.modiAdminLabel)

    ret = QtWidgets.QGroupBox(mytr_("Optical character recognition") + " (OCR)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def modiButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Start OCR screen reader for Japanese"))
    ret.setChecked(settings.global_().isOcrEnabled())
    ret.toggled.connect(settings.global_().setOcrEnabled)
    return ret

  @memoizedproperty
  def ocrInfoLabel(self):
    ret = QtWidgets.QLabel("\n".join((
      my.tr("When enabled, you can press HotKey+Mouse to select the text to read."),
      my.tr("OCR requires MODI OCR from Microsoft Office 2007 to be installed."),
    )))
    ret.setWordWrap(True)
    #ret.setOpenExternalLinks(True)
    return ret

  @memoizedproperty
  def modiAdminLabel(self):
    ret = QtWidgets.QLabel(my.tr("OCR requires starting VNR as administrator to work."))
    ret.setWordWrap(True)
    #ret.setOpenExternalLinks(True)
    return ret

  def refresh(self):
    self.modiButton.setEnabled(features.ADMIN != False and ocrman.manager().isInstalled())
    skqss.class_(self.modiAdminLabel, 'text-error' if features.ADMIN == False else 'text-success')

class OcrTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(OcrTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _OcrTab(self)
    #self.setMinimumWidth(330)

  def save(self): pass
  def load(self): pass
  def refresh(self): self.__d.refresh()

## i18n ##

#@Q_Q
class _I18nTab(object):

  LANGUAGES = 'en', 'zh', 'ko', 'th', 'vi', 'ms', 'id', 'ar', 'de', 'es', 'fr', 'it', 'nl', 'pl', 'pt', 'ru'

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.languageGroup)
    layout.addStretch()
    q.setLayout(layout)

  @memoizedproperty
  def languageGroup(self):
    ss = settings.global_()
    def _button(lang):
      ret = QtWidgets.QCheckBox(i18n.language_name(lang))
      ret.setChecked(lang in ss.blockedLanguages())
      ret.clicked[bool].connect(lambda value:
          self._toggleLanguage(lang, value))
      ret.setEnabled(ret.isChecked() or not ss.userLanguage().startswith(lang))
      ss.userLanguageChanged.connect(lambda value:
          ret.setEnabled(ret.isChecked() or not value.startswith(lang)))
      return ret

    layout = QtWidgets.QVBoxLayout()
    for lang in self.LANGUAGES:
      layout.addWidget(_button(lang))

    infoLabel = QtWidgets.QLabel("\n".join((
      my.tr("Block these languages as much as possible."),
      my.tr("This includes user comments and subtitles."),
      my.tr("Language-specific preferences will disappear after restart."),
    )))
    #infoLabel.setWordWrap(True)
    layout.addWidget(infoLabel)
    ret = QtWidgets.QGroupBox(my.tr("Blocked languages"))
    ret.setLayout(layout)
    return ret

  def _toggleLanguage(self, lang, value):
    """
    @param  lang  str
    @param  value  bool
    @return  bool  whether changed
    """
    ss = settings.global_()
    if value and lang[:2] == ss.userLanguage()[:2]:
      if not prompt.confirmBlockLanguage(lang):
        return False
    total = ss.blockedLanguages()
    if value: # add
      if lang not in total:
        total.add(lang)
        ss.setBlockedLanguages(total)
    else: # remove
      if lang in total:
        total.remove(lang)
        ss.setBlockedLanguages(total)
    return True

class I18nTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(I18nTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _I18nTab(self)

  def save(self): pass
  def load(self): pass
  def refresh(self): pass

## Term ##

#@Q_Q
class _TermTab(object):

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.generalGroup)
    layout.addWidget(self.rbmtGroup)
    layout.addStretch()
    q.setLayout(layout)

  @memoizedproperty
  def generalGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.underlineButton)
    #layout.addWidget(self.hentaiButton)
    ret = QtWidgets.QGroupBox(my.tr("Machine translation option"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def underlineButton(self):
    ret = QtWidgets.QCheckBox("%s (%s)" % (
        my.tr("Underline the text modified by the Shared Dictionary if possible"),
        tr_("default")))
    ss = settings.global_()
    ret.setChecked(ss.isTermMarked())
    ret.toggled.connect(ss.setTermMarked)
    return ret

  #@memoizedproperty
  #def hentaiButton(self):
  #  ret = QtWidgets.QRadioButton(my.tr("Enable hentai rules for H-scene"))
  #  ss = settings.global_()
  #  ret.setChecked(ss.isHentaiEnabled())
  #  ret.toggled.connect(ss.setHentaiEnabled)
  #  return ret

  @memoizedproperty
  def rbmtGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.syntaxButton)
    layout.addWidget(self.syntaxInfo)
    ret = QtWidgets.QGroupBox("%s (%s)" % (
      my.tr("VNR's machine translation system"),
      my.tr("require {0}").format(','.join((
        'MeCab',
        'CaboCha',
        'UniDic',
      ))),
    ))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def syntaxButton(self):
    ret = QtWidgets.QCheckBox("%s (alpha)" % my.tr("Enable Japanese syntax terms to enhance machine translation quality"))
    ss = settings.global_()
    ret.setChecked(ss.isTranslationSyntaxEnabled())
    ret.toggled.connect(ss.setTranslationSyntaxEnabled)
    ret.toggled.connect(self._onTranslationSyntaxChanged)
    return ret

  def _onTranslationSyntaxChanged(self):
    ss = settings.global_()
    if ss.isTranslationSyntaxEnabled():
      ss.setCaboChaEnabled(True)
      ss.setMeCabDictionary('unidic')

  @memoizedproperty
  def syntaxInfo(self):
    ret = QtWidgets.QLabel("<br/><br/>".join((
      '<span style="color:purple">%s</span>' % my.tr('This feature is under development. Please <span style="color:red">DO NOT</span> enable this option unless you are familiar with Japanese and the Shared Dictionary.'),
      my.tr("VNR's machine translator is based on the syntax terms in the Shared Dictionary. Currently, the existing syntax terms are insufficient to translate most Japanese sentences. When there is no matched syntax for a sentence, the selected 3rd machine translators will be used as alternative."),
    )))
    ret.setWordWrap(True)
    return ret

  def refresh(self):
    ss = settings.global_()
    self.syntaxButton.setEnabled(
        ss.isTranslationSyntaxEnabled() or
        dicts.mecab('unidic').exists() and dicts.cabocha('unidic').exists())
    self.syntaxButton.setChecked(ss.isTranslationSyntaxEnabled()) # force reload settings

class TermTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(TermTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _TermTab(self)

  def save(self): pass
  def load(self): pass
  def refresh(self): self.__d.refresh()

## Text ##

@Q_Q
class _TextTab(object):

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    rows = QtWidgets.QVBoxLayout()
    rows.addWidget(self.colorGroup)
    rows.addWidget(self.fontGroup)
    rows.addStretch()
    q.setLayout(rows)

  ## Text font ##

  @staticmethod
  def _createResetFontButton(defval, sig=None):
    ret = create_cell_button()
    ret.setText(u"×") # ばつ
    ret.setToolTip(tr_("Reset") + ": " + defval)
    if sig:
      ret.clicked.connect(sig)
    return ret

  @staticmethod
  def _createFontEdit(sig=None):
    def refresh():
      t = ret.currentText().strip()
      ok = ret.findText(t, Qt.MatchFixedString) >= 0 # case-insensitive match
      skqss.class_(ret, 'default' if ok else 'error')
    ret = QtWidgets.QFontComboBox()
    ret.setEditable(True)
    ret.setMaximumWidth(150)
    ret.setToolTip(tr_("Font"))
    ret.editTextChanged.connect(refresh)
    if sig:
      ret.currentFontChanged.connect(sig)
    return ret

  LANGUAGES = "Japanese", "English", "Chinese", "Korean", "Thai", "Vietnamese", "Malaysian", "Indonesian", "Arabic", "German", "Spanish", "French", "Italian",  "Dutch", "Polish", "Portuguese", "Russian"
  LANGS =     'ja',       'en',      'zht',     'ko',     'th',   'vi',         'ms',        'id',         'ar',     'de',     'es',      'fr',     'it',       'nl',    'pl',     'pt',         'ru'
  assert len(LANGUAGES) == len(LANGS)

  def _synthesizeLoadSaveFont(self, Lang):
    """
    @return  load method, save method
    """
    #lang = Lang.lower()
    lang = Lang[0].lower() + Lang[1:]
    ss = settings.global_()
    fontEdit = lambda: getattr(self, lang + 'FontEdit')
    def _load(this):
      getter = getattr(ss, lang + 'Font')
      family = getter()
      fontEdit().setCurrentFont(QtGui.QFont(family))
      #fontEdit().setEditText(family)
      #self.resetJapaneseButton.setVisible(
      #    QtGui.QFont(family).family().lower() != QtGui.QFont(config.FONT_JA).family().lower())
    def _save(this):
      setter = getattr(ss, "set{0}Font".format(Lang))
      t = fontEdit().currentFont().family()
      if t:
        setter(t)
    return _load, _save

  @memoizedproperty
  def fontGroup(self):
    # QColorDialog.getColor is not used, which does not support live coding

    grid = QtWidgets.QGridLayout()
    ss = settings.global_()

    blans = ss.blockedLanguages()

    r = 0
    for i, la in enumerate(self.LANGS):
      if la == 'ja' or la[:2] not in blans:
        r += 1
        LangName = self.LANGUAGES[i]
        Lang = LangName.replace(' ', '')
        lang = Lang[0].lower() + Lang[1:]

        load, save = self._synthesizeLoadSaveFont(Lang)
        setattr(self, "_load{0}Font".format(Lang), load)
        setattr(self, "_save{0}Font".format(Lang), save)

        deffont = i18n.font_family(la)
        resetButton = self._createResetFontButton(deffont, partial(
            getattr(ss, "set{0}Font".format(Lang)), deffont))

        fontEdit = self._createFontEdit(save)
        setattr(self, lang + 'FontEdit', fontEdit)
        getattr(ss, lang + 'FontChanged').connect(load)

        c = 0; grid.addWidget(fontEdit, r, c)
        c += 1; grid.addWidget(resetButton, r, c)
        c += 1; grid.addWidget(QtWidgets.QLabel("<= " + tr_(LangName)), r, c)

    ret = QtWidgets.QGroupBox(tr_("Font family"))
    ret.setLayout(grid)
    return ret

  ## Text color ##

  @staticmethod
  def _createResetColorButton(sig=None):
    ret = create_cell_button()
    ret.setText(u"×") # ばつ
    ret.setToolTip(my.tr("Reset default color"))
    if sig:
      ret.clicked.connect(sig)
    return ret

  @staticmethod
  def _createColorButton():
    ret = QtWidgets.QPushButton()
    skqss.class_(ret, 'transp')
    ret.setToolTip(my.tr("Click to change the color"))
    ret.setIconSize(ICON_BUTTON_SIZE)
    return ret

  def _synthesizeLoadColor(self, name):
    def load(this):
      icon = QtGui.QPixmap(ICON_BUTTON_SIZE)
      icon.fill(getattr(settings.global_(), name + 'Color')())
      getattr(self, name + 'ColorButton').setIcon(icon)
    return load

  @staticmethod
  def _createColorDialog(title, setter, defval=None, parent=None):
    ret = QtWidgets.QColorDialog(defval, parent)
    ret.setWindowTitle(title)
    ret.setOption(QtWidgets.QColorDialog.NoButtons)
    ret.currentColorChanged.connect(setter)
    return ret

  #@staticmethod
  def _synthesizeGetColor(self, name, Name, tr):
    ss = settings.global_()
    setter = lambda color: getattr(ss, 'set{0}Color'.format(Name))(color.name())
    pty = '_{0}Dialog'.format(name)
    def ret():
      if not hasattr(self, pty):
        w = _TextTab._createColorDialog(
          tr,
          setter,
          getattr(ss, name + 'Color')(),
          self.q)
        setattr(self, pty, w)
      return getattr(self, pty).show()
    return ret

  @memoizedproperty
  def colorGroup(self):
    # QColorDialog.getColor is not used, which does not support live coding
    ss = settings.global_()
    #layout = QtWidgets.QVBoxLayout()

    grid = QtWidgets.QGridLayout()

    conf = (
      ('shadow', mytr_("Background shadow")),
      ('font', mytr_("Fill color")),
      ('text', mytr_("Game text")),
      ('subtitle', mytr_("Community subtitle")),
      ('comment', mytr_("User comment")),
      ('danmaku', mytr_("User danmaku")),
    )

    r = c = 0
    for i, (name, tr) in enumerate(conf):
      Name = name.capitalize()
      NAME = name.upper()
      label = QtWidgets.QLabel("<= " + tr)
      resetButton = self._createResetColorButton(partial(
          getattr(ss, 'setGrimoire{0}Color'.format(Name)),
          getattr(config, 'SETTINGS_{0}_COLOR'.format(NAME))))

      colorButton = self._createColorButton()
      colorButton.clicked.connect(self._synthesizeGetColor(
          'grimoire' + Name, 'Grimoire' + Name, tr))
      setattr(self, 'grimoire{0}ColorButton'.format(Name), colorButton)

      if i % 2 == 0:
        c = 0
      grid.addWidget(colorButton, r, c)
      c += 1
      grid.addWidget(resetButton, r, c)
      c += 1
      grid.addWidget(label, r, c)
      c += 1
      if i % 2:
        r += 1

      load = self._synthesizeLoadColor('grimoire' + Name)
      setattr(self, '_load{0}Color'.format(Name), load)
      getattr(ss, 'grimoire{0}ColorChanged'.format(Name)).connect(load)

    grid.addWidget(QtWidgets.QLabel(), r, 0)
    r += 1

    # 8 Total
    blans = settings.global_().blockedLanguages()
    conf = ( # Name, name, tr, lang
      #('Lougo', None,  u"ルー語", None),
      ('Bing', None,  mytr_("Bing"), None),
      ('Google', None,  mytr_("Google"), None),
      ('LecOnline', 'lecOnline',  mytr_("LEC Online"), None),
      ('Infoseek', None,  mytr_("Infoseek"), None),
      ('Excite', None,  mytr_("Excite"), None),
      ('Transru', None,  mytr_("Translate.Ru"), None),
      ('Baidu', None,  mytr_("Baidu"), 'zh'),
      #('Youdao', None,  mytr_("Youdao"), 'zh'),
      ('JBeijing', None,  mytr_("JBeijing"), 'zh'),
      ('Fastait', None,  mytr_("FastAIT"), 'zh'),
      ('Dreye', None,  mytr_("Dr.eye"), 'zh'),
      ('EzTrans', 'ezTrans',  mytr_("ezTrans"), 'ko'),
      ('HanViet', 'hanViet',  u"Hán Việt", 'vi'),
      ('Atlas', None,  mytr_("ATLAS"), 'en'),
      ('Lec', None,  mytr_("LEC"), 'en'),
    )
    i = 0
    for Name, name, tr, lang in conf:
      if not lang in blans:
        if not name:
          name = Name.lower()
        NAME = Name.upper()
        t = u"<= " + tr
        if lang:
          t += " (%s)" % i18n.language_name(lang)
        label = QtWidgets.QLabel(t)
        label.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
        resetButton = self._createResetColorButton(partial(
            getattr(ss, 'set{0}Color'.format(Name)),
            getattr(config, 'SETTINGS_{0}_COLOR'.format(NAME))))

        colorButton = self._createColorButton()
        colorButton.clicked.connect(self._synthesizeGetColor(
            name, Name, tr))
        setattr(self, name + 'ColorButton', colorButton)

        if i % 2 == 0:
          c = 0
        grid.addWidget(colorButton, r, c)
        c += 1
        grid.addWidget(resetButton, r, c)
        c += 1
        grid.addWidget(label, r, c)
        c += 1
        if i % 2:
          r += 1
        i += 1

        load = self._synthesizeLoadColor(name)
        setattr(self, '_load{0}Color'.format(Name), load)
        getattr(ss, name + 'ColorChanged').connect(load)

    ret = QtWidgets.QGroupBox(mytr_("Text color"))
    ret.setLayout(grid)
    return ret

  def load(self):
    blans = settings.global_().blockedLanguages()
    for i, la in enumerate(self.LANGS):
      if la == 'ja' or la[:2] not in blans:
        lang = self.LANGUAGES[i].replace(' ', '')
        f = getattr(self, "_load{0}Font".format(lang))
        f(self)

    for Name in ('Font', 'Shadow', 'Text', 'Subtitle', 'Comment', 'Danmaku',
                 #'Lougo',
                 'Bing', 'Google', 'LecOnline', 'Infoseek', 'Excite', 'Transru', 'Baidu', 'JBeijing', 'Fastait', 'Dreye', 'EzTrans', 'HanViet', 'Atlas', 'Lec'):
      try: getattr(self, '_load{0}Color'.format(Name))(self)
      except AttributeError: pass

class TextTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(TextTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _TextTab(self)
    self.__d.load()

  def save(self): pass
  def load(self): pass
  def refresh(self): pass

## Translation ##

class TranslationTab(QtWidgets.QDialog):
  def __init__(self, parent=None):
    super(TranslationTab, self).__init__(parent)
    skqss.class_(self, 'texture')

    layout = QtWidgets.QVBoxLayout()
    label = QtWidgets.QLabel(my.tr(
"You can select your preferred machine translators and look-up dictionaries here. The translators and dictionaries are independent that do not require each other. Look-up dictionaries require downloading at least one offline MeCab dictionary."
))
    label.setWordWrap(True)
    l = skwidgets.SkWidgetLayout(label)
    w = QtWidgets.QGroupBox(tr_("About"))
    w.setLayout(l)
    layout.addWidget(w)
    layout.addStretch()
    self.setLayout(layout)

  def save(self): pass
  def load(self): pass
  def refresh(self): pass

#@Q_Q
#class _TranslationScriptTab(object):
#
#  def __init__(self, q):
#    self._createUi(q)
#
#  def _createUi(self, q):
#    layout = QtWidgets.QVBoxLayout()
#    layout.addWidget(self.tahGroup)
#    layout.addStretch()
#    q.setLayout(layout)
#
#  # TAH group
#
#  @memoizedproperty
#  def tahGroup(self):
#    layout = QtWidgets.QVBoxLayout()
#    layout.addWidget(self.tahJaButton)
#    layout.addWidget(self.tahEnButton)
#    layout.addWidget(self.tahInfoEdit)
#    layout.addStretch()
#    ret = QtWidgets.QGroupBox(my.tr("TAH replacement script for Japanese machine translation"))
#    ret.setLayout(layout)
#    return ret
#
#  @memoizedproperty
#  def tahJaButton(self):
#    ret = QtWidgets.QCheckBox(my.tr("Enable Japanse-Japanese correction for all languages"))
#    ret.setChecked(settings.global_().isTranslationScriptJaEnabled())
#    ret.toggled.connect(settings.global_().setTranslationScriptJaEnabled)
#    return ret
#
#  @memoizedproperty
#  def tahEnButton(self):
#    ret = QtWidgets.QCheckBox(my.tr("Enable Japanese-English correction for Latin languages"))
#    ret.setChecked(settings.global_().isTranslationScriptEnEnabled())
#    ret.toggled.connect(settings.global_().setTranslationScriptEnEnabled)
#    return ret
#
#  @memoizedproperty
#  def tahInfoEdit(self):
#    ret = QtWidgets.QTextBrowser()
#    #ret.setTextFormat(Qt.RichText)
#    #ret.setWordWrap(True)
#    #ret.setOpenExternalLinks(True)
#    skqss.class_(ret, 'texture')
#    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
#    #ret.setAlignment(Qt.AlignCenter)
#    ret.setReadOnly(True)
#    ret.setOpenExternalLinks(True)
#
#    url = "http://sakuradite.com/wiki/en/VNR/Translation_Scripts"
#    ret.setHtml(my.tr(
#"""TAH script is a set of <a href="http://en.wikipedia.org/wiki/Regular_expression">regular expression</a> rules originally written by @errzotl to enhance ATLAS Japanese-English translation.
#But the script serves to correct and normalize spoken Japanese rather than translation.
#<br/><br/>
#VNR's script is branched from @errzotl's 0.16 script and maintained at <a href="{0}">{0}</a>.
#VNR will periodically sync the local script with the online wiki.
#You usually don't want to disable this unless for debugging purpose.
#<br/><br/>
#Normal users don't need to touch this script.
#But if you want, you can try VNR's Machine Translation Tester to see how TAH script participates in the machine translation pipeline.
#You can also edit the local translation script in caches, but it will be overwritten after the update.
#It will be better to merge your changes with the online wiki."""
#).format(url))
#    return ret
#
#class TranslationScriptTab(QtWidgets.QDialog):
#
#  def __init__(self, parent=None):
#    super(TranslationScriptTab, self).__init__(parent)
#    skqss.class_(self, 'texture')
#    self.__d = _TranslationScriptTab(self)
#
#  def save(self): pass
#  def load(self): pass
#  def refresh(self): pass #self.__d.refresh()

#@Q_Q
class _MachineTranslationTab(object):

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    blans = settings.global_().blockedLanguages()
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.honyakuGroup)
    #layout.addWidget(self.correctionGroup)
    if 'zh' not in blans:
      layout.addWidget(self.chineseGroup)
    if 'en' not in blans:
      layout.addWidget(self.tahGroup)
    layout.addStretch()
    q.setLayout(layout)

  # IME correction group

  #@memoizedproperty
  #def correctionGroup(self):
  #  layout = QtWidgets.QVBoxLayout()
  #  layout.addWidget(self.correctionButton)
  #  ret = QtWidgets.QGroupBox(my.tr("Japanese spell correction"))
  #  ret.setLayout(layout)
  #  return ret

  #@memoizedproperty
  #def correctionButton(self):
  #  ret = QtWidgets.QCheckBox(
  #      "%s, %s:\n%s" %
  #      (my.tr("Use Microsoft Japanese IME to convert kana to kanji"),
  #       my.tr("like this"), u"すもももももも => スモモも桃も"))
  #  ret.setChecked(settings.global_().isMsimeCorrectionEnabled())
  #  ret.toggled.connect(settings.global_().setMsimeCorrectionEnabled)
  #  return ret

  # TAH script group

  @memoizedproperty
  def tahGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.atlasTahButton)
    layout.addWidget(self.lecTahButton)
    layout.addWidget(self.tahInfoLabel)
    #layout.addStretch()
    ret = QtWidgets.QGroupBox(my.tr("TAH script for Japanese-English machine translators"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def atlasTahButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
      "ATLAS offline machine translation"))
    ss = settings.global_()
    ret.setChecked(ss.isAtlasScriptEnabled())
    ret.toggled.connect(ss.setAtlasScriptEnabled)
    ret.setEnabled(ss.isAtlasEnabled())
    ss.atlasEnabledChanged.connect(ret.setEnabled)
    return ret

  @memoizedproperty
  def lecTahButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
      "LEC online and offline machine translation"))
    ss = settings.global_()
    ret.setChecked(ss.isLecScriptEnabled())
    ret.toggled.connect(ss.setLecScriptEnabled)
    def load(): ret.setEnabled(ss.isLecEnabled() or ss.isLecOnlineEnabled())
    load()
    ss.lecEnabledChanged.connect(load)
    ss.lecOnlineEnabledChanged.connect(load)
    return ret

  @memoizedproperty
  def tahInfoLabel(self):
    ret = QtWidgets.QLabel()
    #ret.setTextFormat(Qt.RichText)
    #ret.setWordWrap(True)
    #ret.setOpenExternalLinks(True)
    #skqss.class_(ret, 'texture')
    #ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    #ret.setAlignment(Qt.AlignCenter)
    #ret.setReadOnly(True)
    ret.setWordWrap(True)
    ret.setOpenExternalLinks(True)

    email = 'TAHscript@outlook.com'
    ret.setText(my.tr(
"""TAH script is a set of <a href="http://en.wikipedia.org/wiki/Regular_expression">regular expression</a> rules originally written by @errzotl sensei to enhance Japanese-English translation quality.
You can report the bugs to <a href="mailto:{0}">{0}</a>."""
).format(email))
    return ret

  # Chinese group

  @memoizedproperty
  def chineseGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.chineseButton)
    layout.addWidget(self.yueButton)
    ret = QtWidgets.QGroupBox(my.tr("Preferred Chinese characters"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def chineseButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
        "Convert Simplified Chinese subtitles to Traditional Chinese"))
    ret.setChecked(settings.global_().convertsChinese())
    ret.toggled.connect(settings.global_().setConvertsChinese)
    return ret

  @memoizedproperty
  def yueButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
        "Convert Mandarin Chinese machine translation to Yue Chinese (using Baidu)"))
    ret.setChecked(settings.global_().isYueEnabled())
    ret.toggled.connect(settings.global_().setYueEnabled)
    return ret

  # Machine translator

  @memoizedproperty
  def honyakuGroup(self):
    blans = settings.global_().blockedLanguages()
    grid = QtWidgets.QGridLayout()

    # Online
    r = 0
    grid.addWidget(QtWidgets.QLabel(my.tr("Online translators") + ":"), r, 0, 1, 2)

    if 'zh' not in blans:
      r += 1
      grid.addWidget(self._createBrowseButton("http://fanyi.baidu.com"), r, 0)
      grid.addWidget(self.baiduButton, r, 1)

    r += 1
    grid.addWidget(self._createBrowseButton("http://www.bing.com/translator"), r, 0)
    grid.addWidget(self.bingButton, r, 1)

    r += 1
    grid.addWidget(self._createBrowseButton("http://translate.google.com"), r, 0)
    grid.addWidget(self.googleButton, r, 1)

    r += 1
    grid.addWidget(self._createBrowseButton("http://www.lec.com/translate-demos.asp"), r, 0)
    grid.addWidget(self.lecOnlineButton, r, 1)

    r += 1
    grid.addWidget(self._createBrowseButton("http://translation.infoseek.ne.jp"), r, 0)
    grid.addWidget(self.infoseekButton, r, 1)

    r += 1
    grid.addWidget(self._createBrowseButton("http://www.excite.co.jp/world"), r, 0)
    grid.addWidget(self.exciteButton, r, 1)

    if 'ru' not in blans:
      r += 1
      grid.addWidget(self._createBrowseButton("http://www.translate.ru"), r, 0)
      grid.addWidget(self.transruButton, r, 1)

    # Offline
    r += 1
    grid.addWidget(QtWidgets.QLabel(my.tr("Offline translators") + ":"), r, 0, 1, 2)

    if 'zh' not in blans:
      r += 1
      grid.addWidget(self.jbeijingBrowseButton, r, 0)
      grid.addWidget(self.jbeijingButton, r, 1)

      r += 1
      grid.addWidget(self.fastaitBrowseButton, r, 0)
      grid.addWidget(self.fastaitButton, r, 1)

      r += 1
      grid.addWidget(self.dreyeBrowseButton, r, 0)
      grid.addWidget(self.dreyeButton, r, 1)

    if 'ko' not in blans:
      r += 1
      grid.addWidget(self.ezTransBrowseButton, r, 0)
      grid.addWidget(self.ezTransButton, r, 1)

    if 'vi' not in blans: #and 'zh' not in blans:
      r += 1
      #grid.addWidget(self.hanVietBrowseButton, r, 0)
      grid.addWidget(self.hanVietButton, r, 1)

    if 'en' not in blans:
      r += 1
      grid.addWidget(self.atlasBrowseButton, r, 0)
      grid.addWidget(self.atlasButton, r, 1)

      r += 1
      grid.addWidget(self.lecBrowseButton, r, 0)
      grid.addWidget(self.lecButton, r, 1)

    ret = QtWidgets.QGroupBox(my.tr("Preferred machine translation providers"))
    ret.setLayout(grid)
    return ret

  def _createBrowseButton(self, path): # unicode or function -> QPushButton
    ret = create_cell_button()
    ret.setText("+")
    if isinstance(path, str) or isinstance(path, unicode):
      ret.setToolTip("%s: %s" % (tr_("Browse"), path))
      open = osutil.open_url if path.startswith('http') else osutil.open_url
      slot = partial(open, path)
    else:
      ret.setToolTip(tr_("Browse"))
      slot = partial(lambda path: osutil.open_location(path()), path)
    ret.clicked.connect(slot)
    return ret

  @memoizedproperty
  def bingButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Microsoft Bing.com multilingual translation service"))
    ret.setChecked(settings.global_().isBingEnabled())
    ret.toggled.connect(settings.global_().setBingEnabled)
    return ret

  @memoizedproperty
  def googleButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Google.com multilingual translation service"))
    ret.setChecked(settings.global_().isGoogleEnabled())
    ret.toggled.connect(settings.global_().setGoogleEnabled)
    return ret

  @memoizedproperty
  def infoseekButton(self):
    ret = QtWidgets.QCheckBox("%s (%s)" % (
        my.tr("Infoseek.ne.jp multilingual translation service"),
        my.tr("excluding {0}").format(', '.join((
            tr_("ms"),
            tr_("ar"),
            tr_("nl"),
            tr_("pl"),
            tr_("ru"),
    )))))
    ret.setChecked(settings.global_().isInfoseekEnabled())
    ret.toggled.connect(settings.global_().setInfoseekEnabled)
    return ret

  @memoizedproperty
  def exciteButton(self):
    ret = QtWidgets.QCheckBox("%s (%s)" % (
        my.tr("Excite.co.jp multilingual translation service"),
        my.tr("excluding {0}").format(', '.join((
            tr_("ms"),
            tr_("th"),
            tr_("vi"),
            tr_("id"),
            tr_("ar"),
            tr_("nl"),
            tr_("pl"),
            )))))
    ret.setChecked(settings.global_().isExciteEnabled())
    ret.toggled.connect(settings.global_().setExciteEnabled)
    return ret

  @memoizedproperty
  def lecOnlineButton(self):
    ret = QtWidgets.QCheckBox("%s (%s, %s)" % (
        my.tr("LEC.com multilingual translation service"),
        my.tr("recommended for European"),
        my.tr("excluding {0}").format(', '.join((
            tr_("ms"),
            tr_("th"),
            tr_("vi"),
    )))))
    ret.setChecked(settings.global_().isLecOnlineEnabled())
    ret.toggled.connect(settings.global_().setLecOnlineEnabled)
    return ret

  @memoizedproperty
  def baiduButton(self):
    ret = QtWidgets.QCheckBox("%s (%s)" % (
        my.tr("Baidu.com Chinese translation service"),
        my.tr("recommended for Chinese")))
    ret.setChecked(settings.global_().isBaiduEnabled())
    ret.toggled.connect(settings.global_().setBaiduEnabled)
    return ret

  @memoizedproperty
  def transruButton(self):
    ret = QtWidgets.QCheckBox("%s (%s, %s)" % (
        my.tr("Translate.Ru multilingual translation service"),
        my.tr("recommended for Russian"),
        my.tr("including {0}").format(', '.join((
            tr_("ja"),
            tr_("en"),
            tr_("de"),
            tr_("es"),
            tr_("fr"),
            tr_("it"),
            tr_("pt"),
            tr_("ru"),
    )))))
    ret.setChecked(settings.global_().isTransruEnabled())
    ret.toggled.connect(settings.global_().setTransruEnabled)
    return ret

  #@memoizedproperty
  #def youdaoButton(self):
  #  ret = QtWidgets.QCheckBox(my.tr("Youdao.com Chinese translation service"))
  #  ret.setChecked(settings.global_().isYoudaoEnabled())
  #  ret.toggled.connect(settings.global_().setYoudaoEnabled)
  #  return ret

  #@memoizedproperty
  #def lougoButton(self):
  #  ret = QtWidgets.QCheckBox(u"ルー語%s" % (
  #      my.tr("Japanese-English hybrid language")))
  #  ret.setChecked(settings.global_().isLougoEnabled())
  #  ret.toggled.connect(settings.global_().setLougoEnabled)
  #  return ret

  @memoizedproperty
  def dreyeButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Dr.eye Chinese-Japanese/English translator"))
    ret.setChecked(settings.global_().isDreyeEnabled())
    ret.toggled.connect(settings.global_().setDreyeEnabled)
    return ret
  @memoizedproperty
  def dreyeBrowseButton(self):
    return self._createBrowseButton(libman.dreye().location)

  @memoizedproperty
  def jbeijingButton(self):
    ret = QtWidgets.QCheckBox("%s (%s)" % (
        my.tr("JBeijing Chinese translator"),
        my.tr("recommended for Chinese")))
    ret.setChecked(settings.global_().isJBeijingEnabled())
    ret.toggled.connect(settings.global_().setJBeijingEnabled)
    return ret
  @memoizedproperty
  def jbeijingBrowseButton(self):
    return self._createBrowseButton(libman.jbeijing().location)

  @memoizedproperty
  def fastaitButton(self):
    ret = QtWidgets.QCheckBox("%s (%s)" % (
        my.tr("FastAIT Chinese-Japanese/English translator"),
        my.tr("recommended for Chinese")))
    ret.setChecked(settings.global_().isFastaitEnabled())
    ret.toggled.connect(settings.global_().setFastaitEnabled)
    return ret
  @memoizedproperty
  def fastaitBrowseButton(self):
    return self._createBrowseButton(libman.fastait().location)

  @memoizedproperty
  def ezTransButton(self):
    ret = QtWidgets.QCheckBox(my.tr("ezTrans XP Korean translator"))
    ret.setChecked(settings.global_().isEzTransEnabled())
    ret.toggled.connect(settings.global_().setEzTransEnabled)
    return ret
  @memoizedproperty
  def ezTransBrowseButton(self):
    return self._createBrowseButton(libman.eztrans().location)

  @memoizedproperty
  def hanVietButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Han Viet Chinese-Vietnamese translator"))
    ret.setChecked(settings.global_().isHanVietEnabled())
    ret.toggled.connect(settings.global_().setHanVietEnabled)
    return ret

  @memoizedproperty
  def atlasButton(self):
    ret = QtWidgets.QCheckBox(my.tr("ATLAS English translator"))
    ret.setChecked(settings.global_().isAtlasEnabled())
    ret.toggled.connect(settings.global_().setAtlasEnabled)
    return ret
  @memoizedproperty
  def atlasBrowseButton(self):
    return self._createBrowseButton(libman.atlas().location)

  @memoizedproperty
  def lecButton(self):
    ret = QtWidgets.QCheckBox(my.tr("LEC English translator"))
    ret.setChecked(settings.global_().isLecEnabled())
    ret.toggled.connect(settings.global_().setLecEnabled)
    return ret
  @memoizedproperty
  def lecBrowseButton(self):
    return self._createBrowseButton(libman.lec().location)

  #def selectedTranslator(self):
  #  return ('atlas' if self.atlasButton.isChecked() else
  #          'jbeijing' if self.jbeijingButton.isChecked() else
  #          'infoseek')

  def refresh(self):
    ss = settings.global_()
    blans = ss.blockedLanguages()

    # Translators
    if 'zh' not in blans:
      t = ss.isJBeijingEnabled() or bool(libman.jbeijing().location())
      self.jbeijingButton.setEnabled(t)
      self.jbeijingBrowseButton.setEnabled(t)

      t = ss.isFastaitEnabled() or bool(libman.fastait().location())
      self.fastaitButton.setEnabled(t)
      self.fastaitBrowseButton.setEnabled(t)

      t = ss.isDreyeEnabled() or bool(libman.dreye().location())
      self.dreyeButton.setEnabled(t)
      self.dreyeBrowseButton.setEnabled(t)

    if 'ko' not in blans:
      t = ss.isEzTransEnabled() or bool(libman.eztrans().location())
      self.ezTransButton.setEnabled(t)
      self.ezTransBrowseButton.setEnabled(t)

    if 'en' not in blans:
      t = ss.isAtlasEnabled() or bool(libman.atlas().location())
      self.atlasButton.setEnabled(t)
      self.atlasBrowseButton.setEnabled(t)

      t = ss.isLecEnabled() or bool(libman.lec().location())
      self.lecButton.setEnabled(t)
      self.lecBrowseButton.setEnabled(t)

class MachineTranslationTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(MachineTranslationTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _MachineTranslationTab(self)

  def save(self): pass
  def load(self): pass
  def refresh(self): self.__d.refresh()

#@Q_Q
class _DictionaryTranslationTab(object):

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    blans = settings.global_().blockedLanguages()
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.meCabGroup)
    layout.addWidget(self.furiganaGroup)
    layout.addWidget(self.dictGroup)
    layout.addStretch()
    q.setLayout(layout)

  # Chinese group

  @memoizedproperty
  def chineseGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.chineseButton)
    ret = QtWidgets.QGroupBox(my.tr("Preferred Chinese characters"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def chineseButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
      "Convert Simplified Chinese subtitles to Traditional Chinese"))
    ret.setChecked(settings.global_().convertsChinese())
    ret.toggled.connect(settings.global_().setConvertsChinese)
    return ret

  # Japanese parser group

  #@memoizedproperty
  #def parserGroup(self):
  #  layout = QtWidgets.QVBoxLayout()
  #  layout.addWidget(self.mecabButton)
  #  layout.addWidget(self.msimeButton)
  #  ret = QtWidgets.QGroupBox(my.tr("Preferred Japanese syntax parser"))
  #  ret.setLayout(layout)
  #  self._loadParser()
  #  return ret

  #@memoizedproperty
  #def mecabButton(self):
  #  ret = QtWidgets.QRadioButton(
  #     my.tr("MeCab from Nara Institute of Science and Technology") + " (%s)" % tr_("default"))
  #  ret.toggled.connect(self._saveParser)
  #  return ret

  #@memoizedproperty
  #def msimeButton(self):
  #  ret = QtWidgets.QRadioButton(
  #     my.tr("Microsoft Japanese IME")) #+ " (%s)" % tr_("recommended"))
  #  ret.toggled.connect(self._saveParser)
  #  return ret

  #def _loadParser(self):
  #  b = (self.msimeButton if settings.global_().isMsimeParserEnabled() else
  #       self.mecabButton)
  #  if not b.isChecked():
  #    b.setChecked(True)

  #def _saveParser(self):
  #  settings.global_().setMsimeParserEnabled(
  #      self.msimeButton.isChecked())

  # MeCab dictionaries

  @memoizedproperty
  def meCabGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.disableMeCabButton)
    layout.addWidget(self.unidicButton)
    #layout.addWidget(self.mljButton)
    layout.addWidget(self.ipadicButton)

    layout.addWidget(self.caboChaButton)

    infoLabel = QtWidgets.QLabel(my.tr(
      "Changing the MeCab dictionary after it is used might require restarting VNR (always needed on Windows XP)."
    ))
    infoLabel.setWordWrap(True)
    #skqss.class_(infoLabel, 'text-error')
    layout.addWidget(infoLabel)

    ret = QtWidgets.QGroupBox(my.tr("Preferred MeCab dictionary for parsing Japanese"))
    ret.setLayout(layout)
    self._loadMeCab()
    return ret

  @memoizedproperty
  def disableMeCabButton(self):
    ret = QtWidgets.QRadioButton("%s (%s)" % (my.tr("Do not parse Japanese to furigana"), tr_("default")))
    ret.toggled.connect(self._saveMeCab)
    ret.toggled.connect(self._checkRBMT)
    return ret

  @memoizedproperty
  def unidicButton(self):
    ret = QtWidgets.QRadioButton("%s (%s)" % (MECAB_DICT_NAMES['unidic'], tr_("recommended")))
    ret.toggled.connect(self._saveMeCab)
    return ret

  @memoizedproperty
  def mljButton(self):
    ret = QtWidgets.QRadioButton(MECAB_DICT_NAMES['unidic-mlj'])
    ret.toggled.connect(self._saveMeCab)
    ret.toggled.connect(self._checkRBMT)
    return ret

  @memoizedproperty
  def ipadicButton(self):
    ret = QtWidgets.QRadioButton(MECAB_DICT_NAMES['ipadic'])
    ret.toggled.connect(self._saveMeCab)
    ret.toggled.connect(self._checkRBMT)
    return ret

  def _loadMeCab(self):
    t = settings.global_().meCabDictionary()
    b = (self.unidicButton if t == 'unidic' else
         self.ipadicButton if t == 'ipadic' else
         #self.mljButton if t == 'unidic-mlj' else
         self.disableMeCabButton)
    if not b.isChecked():
      b.setChecked(True)

  def _saveMeCab(self):
    t = ('ipadic' if self.ipadicButton.isChecked() else
         'unidic' if self.unidicButton.isChecked() else
         #'unidic-mlj' if self.mljButton.isChecked() else
         '')
    settings.global_().setMeCabDictionary(t)
    self.refreshCaboCha()

  @memoizedproperty
  def caboChaButton(self):
    ret = QtWidgets.QCheckBox("%s (%s)" % (
      my.tr("Syntax highlighting Japanese using CaboCha"),
      my.tr("recommended for Japanese learners"),
    ))
    ss = settings.global_()
    ret.setChecked(ss.isCaboChaEnabled())
    ret.toggled.connect(ss.setCaboChaEnabled)
    ret.toggled.connect(self._checkRBMT)
    return ret

  def refreshCaboCha(self):
    ss = settings.global_()
    self.caboChaButton.setEnabled(ss.isMeCabEnabled() and (
        ss.isCaboChaEnabled() or
        ss.meCabDictionary() == 'unidic' and dicts.cabocha('unidic').exists() or
        ss.meCabDictionary() == 'ipadic' and dicts.cabocha('ipadic').exists()))

  def _checkRBMT(self):
    ss = settings.global_()
    if ss.isTranslationSyntaxEnabled() and not (
        ss.isCaboChaEnabled() and ss.meCabDictionary() == 'unidic'):
      if prompt.confirmTranslationSyntaxDisabled():
        ss.setTranslationSyntaxEnabled(False)
      else:
        if not ss.isCaboChaEnabled():
          ss.setCaboChaEnabled(True)
          self.caboChaButton.setChecked(True)

        if ss.meCabDictionary() != 'unidic':
          ss.setMeCabDictionary('unidic')
          self._loadMeCab()

  # Furigana

  @memoizedproperty
  def furiganaGroup(self):
    blans = settings.global_().blockedLanguages()
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.hiraganaButton)
    layout.addWidget(self.kataganaButton)
    layout.addWidget(self.romajiButton)
    #if 'en' not in blans:
    layout.addWidget(self.trButton)
    if 'ko' not in blans:
      layout.addWidget(self.hangulButton)
    if 'th' not in blans:
      layout.addWidget(self.thaiButton)
    if 'zh' not in blans:
      layout.addWidget(self.kanjiButton)
    ret = QtWidgets.QGroupBox("%s (%s)" % (
        my.tr("Preferred Japanese furigana characters"),
        my.tr("require MeCab dictionaries")))
    ret.setLayout(layout)
    self._loadFurigana()

    ss = settings.global_()
    ret.setEnabled(bool(ss.meCabDictionary()))
    ss.meCabDictionaryChanged.connect(lambda v:
        ret.setEnabled(bool(v)))
    return ret

  @memoizedproperty
  def hiraganaButton(self):
    ret = QtWidgets.QRadioButton(
      "%s, %s: %s" %
      (tr_("Hiragana") + " (" + tr_("default") + ")",
        my.tr("like this"), u"可愛い（かわいい）"))
    ret.toggled.connect(self._saveFurigana)
    return ret

  @memoizedproperty
  def kataganaButton(self):
    ret = QtWidgets.QRadioButton(
      "%s, %s: %s" %
      (tr_("Katagana"), my.tr("like this"), u"可愛い（カワイイ)"))
    ret.toggled.connect(self._saveFurigana)
    return ret

  @memoizedproperty
  def romajiButton(self):
    ret = QtWidgets.QRadioButton(
      "%s, %s: %s" %
      (tr_("Romaji"), my.tr("like this"), u"可愛い（kawaii）"))
    ret.toggled.connect(self._saveFurigana)
    return ret

  @memoizedproperty
  def hangulButton(self):
    ret = QtWidgets.QRadioButton(
      "%s, %s: %s" %
      (tr_("Hangul"), my.tr("like this"), u"可愛い（가와이이）"))
    ret.toggled.connect(self._saveFurigana)
    return ret

  @memoizedproperty
  def thaiButton(self):
    ret = QtWidgets.QRadioButton(
      "%s, %s: %s" %
      (tr_("Thai"), my.tr("like this"), u"可愛い（ขะหวะอิอิ）"))
    ret.toggled.connect(self._saveFurigana)
    return ret

  @memoizedproperty
  def kanjiButton(self):
    ret = QtWidgets.QRadioButton(
      "%s, %s: %s" %
      (tr_("Kanji"), my.tr("like this"), u"可愛い（卡哇伊）"))
    ret.toggled.connect(self._saveFurigana)
    return ret

  @memoizedproperty
  def trButton(self):
    ret = QtWidgets.QRadioButton(
      "%s, %s: %s (%s)" %
      (tr_("English"), my.tr("like this"), u"可愛い（cute）",
        my.tr("require EDICT")))
    ret.toggled.connect(self._saveFurigana)
    return ret

  def _loadFurigana(self):
    t = settings.global_().rubyType()
    b = (self.romajiButton if t == defs.FURI_ROMAJI else
         self.hangulButton if t == defs.FURI_HANGUL else
         self.thaiButton if t == defs.FURI_THAI else
         self.trButton if t == defs.FURI_TR else
         self.kanjiButton if t == defs.FURI_KANJI else
         self.kataganaButton if t == defs.FURI_KATA else
         self.hiraganaButton)
    if not b.isChecked():
      b.setChecked(True)

  def _saveFurigana(self):
    t = (defs.FURI_ROMAJI if self.romajiButton.isChecked() else
         defs.FURI_HANGUL if self.hangulButton.isChecked() else
         defs.FURI_THAI if self.thaiButton.isChecked() else
         defs.FURI_TR if self.trButton.isChecked() else
         defs.FURI_KANJI if self.kanjiButton.isChecked() else
         defs.FURI_KATA if self.kataganaButton.isChecked() else
         defs.FURI_HIRA)
    settings.global_().setRubyType(t)

  # Dictionaries

  @memoizedproperty
  def dictGroup(self):
    blans = settings.global_().blockedLanguages()
    #if 'en' not in blans:
    ret = QtWidgets.QGroupBox(my.tr("Preferred Japanese phrase dictionaries")) #+ " (%s)" % tr_("offline")) # looked very bad in Korean langua
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(QtWidgets.QLabel(my.tr("Download required") + ":"))
    if 'zh' not in blans:
      layout.addWidget(self.lingoesJaZhButton)
      layout.addWidget(self.lingoesJaZhGbkButton)
    if 'ko' not in blans:
      layout.addWidget(self.lingoesJaKoButton)
    if 'vi' not in blans:
      layout.addWidget(self.lingoesJaViButton)
    if 'en' not in blans:
      layout.addWidget(self.lingoesJaEnButton)
    if 'de' not in blans:
      layout.addWidget(self.wadokuButton)
    if 'en' not in blans:
      layout.addWidget(self.edictButton)
    if 'fr' not in blans:
      layout.addWidget(self.jmdictFrButton)
    if 'ru' not in blans:
      layout.addWidget(self.jmdictRuButton)
    if 'nl' not in blans:
      layout.addWidget(self.jmdictNlButton)
    layout.addWidget(QtWidgets.QLabel(my.tr("Manual installation required") + ":"))
    if 'ja' not in blans:
      layout.addWidget(self.daijirinButton)
      layout.addWidget(self.kojienButton)
    if 'zh' not in blans:
      layout.addWidget(self.zhongriButton)
    ret.setLayout(layout)

    ss = settings.global_()
    ret.setEnabled(bool(ss.meCabDictionary()))
    ss.meCabDictionaryChanged.connect(lambda v:
        ret.setEnabled(bool(v)))
    return ret

  @memoizedproperty
  def edictButton(self):
    ret = QtWidgets.QCheckBox(my.tr("EDICT Japanese-English dictionary"))
    ret.setChecked(settings.global_().isEdictEnabled())
    ret.toggled.connect(settings.global_().setEdictEnabled)
    return ret

  @memoizedproperty
  def daijirinButton(self):
    ret = QtWidgets.QCheckBox("%s (%s)" % (
        my.tr("Daijirin (大辞林) Japanese-Japanese/English dictionary"),
        my.tr("recommended for Japanese learners")))
    ret.setChecked(settings.global_().isDaijirinEnabled())
    ret.toggled.connect(settings.global_().setDaijirinEnabled)
    return ret

  @memoizedproperty
  def kojienButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Kojien (広辞苑) Japanese-Japanese dictionary"))
    ret.setChecked(settings.global_().isKojienEnabled())
    ret.toggled.connect(settings.global_().setKojienEnabled)
    return ret

  @memoizedproperty
  def wadokuButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Wadoku (和独) Japanese-German dictionary"))
    ret.setChecked(settings.global_().isWadokuEnabled())
    ret.toggled.connect(settings.global_().setWadokuEnabled)
    return ret

  @memoizedproperty
  def zhongriButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Zhongri (日中) Japanese-Chinese dictionary"))
    ret.setChecked(settings.global_().isZhongriEnabled())
    ret.toggled.connect(settings.global_().setZhongriEnabled)
    return ret

  @memoizedproperty
  def jmdictFrButton(self):
    ret = QtWidgets.QCheckBox(JMDICT_DICT_NAMES['fr'])
    ret.setChecked(settings.global_().isJMDictFrEnabled())
    ret.toggled.connect(settings.global_().setJMDictFrEnabled)
    return ret

  @memoizedproperty
  def jmdictRuButton(self):
    ret = QtWidgets.QCheckBox(JMDICT_DICT_NAMES['ru'])
    ret.setChecked(settings.global_().isJMDictRuEnabled())
    ret.toggled.connect(settings.global_().setJMDictRuEnabled)
    return ret

  @memoizedproperty
  def jmdictNlButton(self):
    ret = QtWidgets.QCheckBox(JMDICT_DICT_NAMES['nl'])
    ret.setChecked(settings.global_().isJMDictNlEnabled())
    ret.toggled.connect(settings.global_().setJMDictNlEnabled)
    return ret

  @memoizedproperty
  def lingoesJaZhButton(self):
    ret = QtWidgets.QCheckBox("%s (%s)" % (
        LINGOES_DICT_NAMES['ja-zh'],
        my.tr("recommended for Chinese")))
    ret.setChecked(settings.global_().isLingoesJaZhEnabled())
    ret.toggled.connect(settings.global_().setLingoesJaZhEnabled)
    return ret

  @memoizedproperty
  def lingoesJaZhGbkButton(self):
    ret = QtWidgets.QCheckBox(LINGOES_DICT_NAMES['ja-zh-gbk'])
    ret.setChecked(settings.global_().isLingoesJaZhGbkEnabled())
    ret.toggled.connect(settings.global_().setLingoesJaZhGbkEnabled)
    return ret

  @memoizedproperty
  def lingoesJaKoButton(self):
    ret = QtWidgets.QCheckBox(LINGOES_DICT_NAMES['ja-ko'])
    ret.setChecked(settings.global_().isLingoesJaKoEnabled())
    ret.toggled.connect(settings.global_().setLingoesJaKoEnabled)
    return ret

  @memoizedproperty
  def lingoesJaViButton(self):
    ret = QtWidgets.QCheckBox(LINGOES_DICT_NAMES['ja-vi'])
    ret.setChecked(settings.global_().isLingoesJaViEnabled())
    ret.toggled.connect(settings.global_().setLingoesJaViEnabled)
    return ret

  @memoizedproperty
  def lingoesJaEnButton(self):
    ret = QtWidgets.QCheckBox("%s (%s)" % (
        LINGOES_DICT_NAMES['ja-en'],
        my.tr("recommended for English")))
    ret.setChecked(settings.global_().isLingoesJaEnEnabled())
    ret.toggled.connect(settings.global_().setLingoesJaEnEnabled)
    return ret

  def refresh(self):
    ss = settings.global_()
    blans = ss.blockedLanguages()

    # Dictionaries
    if 'zh' not in blans:
      name = 'ja-zh'
      b = self.lingoesJaZhButton
      b.setEnabled(ss.isLingoesDictionaryEnabled(name) or dicts.lingoes(name).exists())

      name = 'ja-zh-gbk'
      b = self.lingoesJaZhGbkButton
      b.setEnabled(ss.isLingoesDictionaryEnabled(name) or dicts.lingoes(name).exists())

    if 'ko' not in blans:
      name = 'ja-ko'
      b = self.lingoesJaKoButton
      b.setEnabled(ss.isLingoesDictionaryEnabled(name) or dicts.lingoes(name).exists())
    if 'vi' not in blans:
      name = 'ja-vi'
      b = self.lingoesJaViButton
      b.setEnabled(ss.isLingoesDictionaryEnabled(name) or dicts.lingoes(name).exists())
    if 'en' not in blans:
      name = 'ja-en'
      b = self.lingoesJaEnButton
      b.setEnabled(ss.isLingoesDictionaryEnabled(name) or dicts.lingoes(name).exists())

    if 'fr' not in blans:
      lang = 'fr'
      b = self.jmdictFrButton
      b.setEnabled(ss.isJMDictEnabled(lang) or dicts.jmdict(lang).exists())
    if 'ru' not in blans:
      lang = 'ru'
      b = self.jmdictRuButton
      b.setEnabled(ss.isJMDictEnabled(lang) or dicts.jmdict(lang).exists())
    if 'nl' not in blans:
      lang = 'nl'
      b = self.jmdictNlButton
      b.setEnabled(ss.isJMDictEnabled(lang) or dicts.jmdict(lang).exists())

    if 'de' not in blans:
      b = ss.isWadokuEnabled() or dicts.wadoku().exists()
      self.wadokuButton.setEnabled(b)

    if 'en' not in blans:
      b = ss.isEdictEnabled() or dicts.edict().exists()
      self.edictButton.setEnabled(b)
      #self.trButton.setEnabled(b)

    self.trButton.setEnabled(self.trButton.isChecked() or ss.isEdictEnabled() or dicts.edict().exists())

    # EPWING
    if 'ja' not in blans:
      self.daijirinButton.setEnabled(ss.isDaijirinEnabled() or bool(ebdict.daijirin().location()))
      self.kojienButton.setEnabled(ss.isKojienEnabled() or bool(ebdict.kojien().location()))
    if 'zh' not in blans:
      self.zhongriButton.setEnabled(ss.isZhongriEnabled() or bool(ebdict.zhongri().location()))
    #if 'de' not in blans:
    #  self.wadokuButton.setEnabled(ss.isWadokuEnabled() or bool(ebdict.wadoku().location()))

    # MeCab
    self.ipadicButton.setEnabled(dicts.mecab('ipadic').exists())
    self.unidicButton.setEnabled(dicts.mecab('unidic').exists())
    #self.mljButton.setEnabled(dicts.mecab('unidic-mlj').exists())

    self.refreshCaboCha()

    # Refresh
    self.caboChaButton.setChecked(ss.isCaboChaEnabled()) # force reload cabocha
    self._loadMeCab()

class DictionaryTranslationTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(DictionaryTranslationTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _DictionaryTranslationTab(self)

  def save(self): pass
  def load(self): pass
  def refresh(self): self.__d.refresh()

## Internet options ##

#@Q_Q
class _InternetTab(object):

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.connectionGroup)
    layout.addWidget(self.proxyGroup)
    layout.addStretch()
    q.setLayout(layout)

  ## Proxy group ##

  @memoizedproperty
  def proxyGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.chinaButton)
    layout.addWidget(self.chinaLabel)
    ret = QtWidgets.QGroupBox(tr_("Mainland China"))
    ret.setLayout(layout)
    self._updateProxyGroup()
    return ret

  @memoizedproperty
  def chinaLabel(self):
    ret = QtWidgets.QLabel(my.tr(
"""Mainland China has blocked many international services including Google.
If you enable this option, VNR will try providing alternative services."""
    ))
    #ret.setWordWrap(True)
    #skqss.class_(ret, 'error')
    return ret

  @memoizedproperty
  def chinaButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
      "You are currently living in Mainland China"
    ))
    ss = settings.global_()
    ret.setChecked(ss.isMainlandChina())
    ret.toggled.connect(ss.setMainlandChina)
    ret.toggled.connect(self._updateProxyGroup)
    return ret

  def _updateProxyGroup(self):
    ok = self.chinaButton.isChecked()
    for it in self.chinaButton, self.chinaLabel:
      skqss.class_(it, 'normal' if ok else '')

  ## Internet ##

  @memoizedproperty
  def connectionGroup(self):
    ret = QtWidgets.QGroupBox(my.tr("Internet connection"))
    layout = QtWidgets.QVBoxLayout()

    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.internetEdit)
    row.addStretch()
    layout.addLayout(row)

    infoLabel = QtWidgets.QLabel(my.tr(
"By default, VNR will automatically detect whether you have got Internet access. If the detection does not work well, you can force enabling/disabling the Internet here."))
    infoLabel.setWordWrap(True)
    layout.addWidget(infoLabel)

    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def internetEdit(self):
    ITEMS = (
      (defs.INTERNET_CONNECTION_AUTO, my.tr("Automatically detect Internet status")),
      (defs.INTERNET_CONNECTION_ON, my.tr("Always enable Internet access")),
      (defs.INTERNET_CONNECTION_OFF, my.tr("Always disable Internet access")),
    )
    KEYS = [it[0] for it in ITEMS]
    DESC = [it[1] for it in ITEMS]
    ret = QtWidgets.QComboBox()
    ret.setEditable(False)
    ret.addItems(DESC)
    def _refresh():
      skqss.class_(ret, 'warning' if ret.currentIndex() else '')
    def _load():
      try: index = KEYS.index(settings.global_().internetConnection())
      except ValueError: index = 0
      ret.setCurrentIndex(index)
      _refresh()
    def _save():
      v = KEYS[ret.currentIndex()]
      settings.global_().setInternetConnection(v)
      _refresh()
    _load()
    ret.currentIndexChanged.connect(_save)
    ret.currentIndexChanged.connect(netman.manager().updateOnline)
    return ret

class InternetTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(InternetTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _InternetTab(self)

  def save(self): pass
  def load(self): pass
  def refresh(self): pass


## App features ##

#@Q_Q
class _FeatureTab(object):

  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.featureGroup)
    layout.addWidget(self.fullScreenGroup) # disabled as people seldom use it
    layout.addStretch()
    q.setLayout(layout)

  ## Full screen ##

  @memoizedproperty
  def fullScreenGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.kagamiFocusButton)
    layout.addWidget(self.kagamiFocusLabel)
    ret = QtWidgets.QGroupBox(tr_("Full screen"))
    ret.setLayout(layout)
    self._updatetKagamiFocusGroup()
    return ret

  @memoizedproperty
  def kagamiFocusLabel(self):
    ret = QtWidgets.QLabel(my.tr(
      "Since context menu would break full screen in most games, you normally don't want to enable this unless you have a very small screen size."
    ))
    ret.setWordWrap(True)
    skqss.class_(ret, 'error')
    return ret

  @memoizedproperty
  def kagamiFocusButton(self):
    ret = QtWidgets.QCheckBox(my.tr(
      "Force enabling context menu in full screen"
    ))
    ss = settings.global_()
    ret.setChecked(ss.isKagamiFocusEnabled())
    ret.toggled.connect(ss.setKagamiFocusEnabled)
    ret.toggled.connect(self._updatetKagamiFocusGroup)
    return ret

  def _updatetKagamiFocusGroup(self):
    ok = not self.kagamiFocusButton.isChecked()
    for it in self.kagamiFocusButton, self.kagamiFocusLabel:
      skqss.class_(it, 'warning' if ok else 'error')

  ## Feature ##

  @memoizedproperty
  def featureGroup(self):
    ret = QtWidgets.QGroupBox(tr_("Features"))
    layout = QtWidgets.QVBoxLayout()
    #layout.addWidget(self.internetAccessButton)

    for it in self.machineTranslationButton, self.userCommentButton:
      layout.addWidget(it)
      skqss.class_(it, 'warning' if it.isChecked() else '')
      it.toggled.connect(partial(lambda it, v:
        skqss.class_(it, 'warning' if v else '')
      , it))

#    introLabel = QtWidgets.QLabel("\n" + my.tr(
#"""These are some of VNR's core features that might take large amount of system resources.
#You can disable the features you don't need to to speed up VNR."""))
#    introLabel.setWordWrap(True)
#    layout.addWidget(introLabel)

    ret.setLayout(layout)
    return ret

  #@memoizedproperty
  #def internetAccessButton(self):
  #  ss = settings.global_()
  #  ret = QtWidgets.QCheckBox(my.tr(
  #    "Allow Internet access to search for translations and game information"
  #  ))
  #  ret.setChecked(ss.allowsInternetAccess())
  #  ret.toggled.connect(ss.setAllowsInternetAccess)
  #  import netman
  #  ret.toggled.connect(netman.manager().updateOnline)
  #  return ret

  @memoizedproperty
  def machineTranslationButton(self):
    ss = settings.global_()
    ret = QtWidgets.QCheckBox(my.tr(
      "Disable online and offline machine translation"
    ))
    ret.setChecked(not ss.allowsMachineTranslation())
    ret.toggled.connect(lambda v: ss.setAllowsMachineTranslation(not v))
    return ret

  @memoizedproperty
  def userCommentButton(self):
    ss = settings.global_()
    ret = QtWidgets.QCheckBox(my.tr(
      "Disable user-contributed comments and danmaku"
    ))
    ret.setChecked(not ss.allowsUserComment())
    ret.toggled.connect(lambda v: ss.setAllowsUserComment(not v))
    return ret

  #@memoizedproperty
  #def ttsButton(self):
  #  ss = settings.global_()
  #  ret = QtWidgets.QCheckBox(my.tr(
  #    "Disable speaking Japanese through text-to-speech"
  #  ))
  #  ret.setChecked(not ss.allowsTextToSpeech())
  #  ret.toggled.connect(lambda v: ss.setAllowsTextToSpeech(not v))
  #  return ret

class FeatureTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(FeatureTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _FeatureTab(self)

  def save(self): pass
  def load(self): pass
  def refresh(self): pass

# Downloads

#DOWNLOADS_MINIMUM_WIDTH = 320
#DOWNLOADS_TEXTEDIT_MAXIMUM_HEIGHT = 120

INSTALLED_STATUS_STYLE = "color:#468847" # green, the same as text-success

class DownloadsTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(DownloadsTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    #self.setMinimumWidth(DOWNLOADS_MINIMUM_WIDTH)

    label = QtWidgets.QLabel(my.tr(
"""Some of the free resources are large but not useful to all users.
So, these resources are not distributed together with VNR by default.
You can select only the resources you need to download here."""))
    #label.setWordWrap(True)

    layout = skwidgets.SkWidgetLayout(label)
    w = QtWidgets.QGroupBox(tr_("About"))
    w.setLayout(layout)

    rows = QtWidgets.QVBoxLayout()
    rows.addWidget(w)
    rows.addStretch()
    self.setLayout(rows)

  def save(self): pass
  def load(self): pass
  def refresh(self): pass

# Dictionary downloads

#@Q_Q
class _DictionaryDownloadsTab(object):
  def __init__(self, q):
    self.meCabButtons = {}
    self.meCabStatusLabels = {}
    self.meCabIntroLabels = {}

    self.caboChaButtons = {}
    self.caboChaStatusLabels = {}
    self.caboChaIntroLabels = {}

    self.lingoesButtons = {}
    self.lingoesStatusLabels = {}
    self.lingoesIntroLabels = {}

    self.jmdictButtons = {}
    self.jmdictStatusLabels = {}
    self.jmdictIntroLabels = {}

    t = self.refreshTimer = QTimer(q)
    t.setSingleShot(False)
    t.setInterval(DOWNLOAD_REFRESH_INTERVAL)
    t.timeout.connect(self._tryRefresh)

    self._refreshTasks = [] # [(dic, refresh ->)]

    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.meCabGroup)
    layout.addWidget(self.caboChaGroup)

    layout.addWidget(self.phraseGroup)

    #row = QtWidgets.QHBoxLayout()
    #row.addWidget(self.refreshButton)
    #row.addStretch()
    #layout.addLayout(row)

    layout.addStretch()
    q.setLayout(layout)
    #q.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)

  # - MeCab dictionaries -

  @memoizedproperty
  def meCabGroup(self): # MeCab dictionaries
    grid = QtWidgets.QGridLayout()

    r = 0
    #for lang in config.MECAB_DICS:
    #for name in 'unidic', 'unidic-mlj', 'ipadic':
    for name in 'unidic', 'ipadic':
      grid.addWidget(self.getMeCabButton(name), r, 0)
      grid.addWidget(self.getMeCabStatusLabel(name), r, 1)
      grid.addWidget(self.getMeCabIntroLabel(name), r, 2)
      grid.addWidget(QtWidgets.QWidget(), r, 3) # stretch
      r += 1

    ret = QtWidgets.QGroupBox(my.tr("MeCab dictionaries for parsing Japanese"))
    ret.setLayout(grid)
    return ret

  def getMeCabButton(self, name):
    ret = self.meCabButtons.get(name)
    if not ret:
      ret = self.meCabButtons[name] = QtWidgets.QPushButton()
      ret.role = ''
      ret.clicked.connect(partial(lambda name:
        self._getMeCab(name) if ret.role == 'get' else
        self._removeMeCab(name) if ret.role == 'remove' else
        None,
      name))
    return ret

  def getMeCabStatusLabel(self, name):
    ret = self.meCabStatusLabels.get(name)
    if not ret:
      ret = self.meCabStatusLabels[name] = QtWidgets.QLabel()
      dic = dicts.mecab(name)
      ret.linkActivated.connect(dic.open)
      path = QtCore.QDir.toNativeSeparators(dic.path)
      ret.setToolTip(path)
    return ret

  def getMeCabIntroLabel(self, name):
    ret = self.meCabIntroLabels.get(name)
    if not ret:
      if name == 'unidic':
        t = "%s (%s, %s)" % (MECAB_DICT_NAMES[name], MECAB_DICT_SIZES[name], tr_("recommended"))
      else:
        t = "%s (%s)" % (MECAB_DICT_NAMES[name], MECAB_DICT_SIZES[name])
      ret = self.meCabIntroLabels[name] = QtWidgets.QLabel(t)
    ret.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
    return ret

  def _getMeCab(self, name):
    if prompt.confirmDownloadDictionary('MeCab (%s)' % name):
      dic = dicts.mecab(name)
      if not dic.exists(): #and not dic.locked():
        dic.get()
      refresh = partial(lambda name:
          (self.refreshMeCab(name), self.refreshCaboCha(name)),
          name)
      if not refresh():
        self.startRefresh(dic, refresh)

  def _removeMeCab(self, name):
    if prompt.confirmRemoveDictionary('MeCab (%s)' % name):
      dicts.mecab(name).remove()
      #settings.global_().setMeCabDictionaryEnabled(name, False)
      self.refreshMeCab(name)

  def refreshMeCab(self, name): # -> bool exists
    b = self.getMeCabButton(name)
    status = self.getMeCabStatusLabel(name)
    dic = dicts.mecab(name)
    if dic.exists():
      #status.setText(mytr_("Installed"))
      status.setText('<a href="#" style="%s">%s</a>' % (INSTALLED_STATUS_STYLE, mytr_("Installed")))
      skqss.class_(status, 'text-success')
      b.role = 'remove'
      b.setEnabled(True)
      b.setText(tr_("Remove"))
      skqss.class_(b, 'btn btn-default')
      return True
    elif dic.locked():
      status.setText(mytr_("Installing"))
      skqss.class_(status, 'text-info')
      b.role = ''
      b.setEnabled(False)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    else:
      online = netman.manager().isOnline()
      status.setText(mytr_("Not installed"))
      skqss.class_(status, 'text-error')
      b.role = 'get'
      b.setEnabled(online)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    return False

  # - CaboCha dictionaries -

  @memoizedproperty
  def caboChaGroup(self): # CaboCha dictionaries
    grid = QtWidgets.QGridLayout()

    r = 0
    for name in 'unidic', 'ipadic':
      grid.addWidget(self.getCaboChaButton(name), r, 0)
      grid.addWidget(self.getCaboChaStatusLabel(name), r, 1)
      label = self.getCaboChaIntroLabel(name)
      grid.addWidget(label, r, 2)
      #grid.addWidget(QtWidgets.QWidget(), r, 3) # stretch
      r += 1

    ret = QtWidgets.QGroupBox(my.tr("CaboCha models for highlighting Japanese"))
    ret.setLayout(grid)
    return ret

  def getCaboChaButton(self, name):
    ret = self.caboChaButtons.get(name)
    if not ret:
      ret = self.caboChaButtons[name] = QtWidgets.QPushButton()
      ret.role = ''
      ret.clicked.connect(partial(lambda name:
        self._getCaboCha(name) if ret.role == 'get' else
        self._removeCaboCha(name) if ret.role == 'remove' else
        None,
      name))
    return ret

  def getCaboChaStatusLabel(self, name):
    ret = self.caboChaStatusLabels.get(name)
    if not ret:
      ret = self.caboChaStatusLabels[name] = QtWidgets.QLabel()
      dic = dicts.cabocha(name)
      ret.linkActivated.connect(dic.open)
      path = QtCore.QDir.toNativeSeparators(dic.path)
      ret.setToolTip(path)
    return ret

  def getCaboChaIntroLabel(self, name):
    ret = self.caboChaIntroLabels.get(name)
    if not ret:
      t = "%s (%s)" % (CABOCHA_DICT_NAMES[name], CABOCHA_DICT_SIZES[name])
      ret = self.caboChaIntroLabels[name] = QtWidgets.QLabel(t)
    ret.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
    return ret

  def _getCaboCha(self, name):
    if prompt.confirmDownloadDictionary('CaboCha (%s)' % name):
      dic = dicts.cabocha(name)
      if not dic.exists(): #and not dic.locked():
        dic.get()
      refresh = partial(self.refreshCaboCha, name)
      if not refresh():
        self.startRefresh(dic, refresh)

  def _removeCaboCha(self, name):
    if prompt.confirmRemoveDictionary('CaboCha (%s)' % name):
      dicts.cabocha(name).remove()
      #settings.global_().setCaboChaDictionaryEnabled(name, False)
      self.refreshCaboCha(name)

  def refreshCaboCha(self, name): # -> bool exists
    b = self.getCaboChaButton(name)
    status = self.getCaboChaStatusLabel(name)
    dic = dicts.cabocha(name)
    if dic.exists():
      #status.setText(mytr_("Installed"))
      status.setText('<a href="#" style="%s">%s</a>' % (INSTALLED_STATUS_STYLE, mytr_("Installed")))
      skqss.class_(status, 'text-success')
      b.role = 'remove'
      b.setEnabled(True)
      b.setText(tr_("Remove"))
      skqss.class_(b, 'btn btn-default')
      return True
    elif dic.locked():
      status.setText(mytr_("Installing"))
      skqss.class_(status, 'text-info')
      b.role = ''
      b.setEnabled(False)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    else:
      online = netman.manager().isOnline()
      status.setText(mytr_("Not installed"))
      skqss.class_(status, 'text-error')
      b.role = 'get'
      mecabdic = dicts.mecab(name)
      b.setEnabled(online and mecabdic.exists())
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    return False

  # - Phrase dictionaries -

  @memoizedproperty
  def phraseGroup(self): # Phrase dictionaries

    grid = QtWidgets.QGridLayout()

    r = 0

    infoLabel = QtWidgets.QLabel(my.tr("Using phrase dictionaries requires at least one MeCab dictionary to be installed."))
    infoLabel.setWordWrap(True)
    skqss.class_(infoLabel, 'text-info')
    grid.addWidget(infoLabel, r, 0, 1, 3)
    r += 1

    blans = settings.global_().blockedLanguages()

    for lang in config.LINGOES_LANGS:
      if lang[:2] not in blans:
        name = 'ja-' + lang
        grid.addWidget(self.getLingoesButton(name), r, 0)
        grid.addWidget(self.getLingoesStatusLabel(name), r, 1)
        grid.addWidget(self.getLingoesIntroLabel(name), r, 2)
        r += 1

    if 'de' not in blans:
      grid.addWidget(self.wadokuButton, r, 0)
      grid.addWidget(self.wadokuStatusLabel, r, 1)
      grid.addWidget(self.wadokuIntroLabel, r, 2)
      r += 1

    if 'en' not in blans:
      grid.addWidget(self.edictButton, r, 0)
      grid.addWidget(self.edictStatusLabel, r, 1)
      grid.addWidget(self.edictIntroLabel, r, 2)
      r += 1

    for lang in config.JMDICT_LANGS:
      if lang not in blans:
        grid.addWidget(self.getJMDictButton(lang), r, 0)
        grid.addWidget(self.getJMDictStatusLabel(lang), r, 1)
        grid.addWidget(self.getJMDictIntroLabel(lang), r, 2)
        r += 1

    ret = QtWidgets.QGroupBox(my.tr("Dictionaries for looking up Japanese phrases"))
    ret.setLayout(grid)
    #ret.setLayout(skwidgets.SkWidgetLayout(skwidgets.SkLayoutWidget(grid)))

    # Increase the width for English
    #ret.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
    return ret

  @memoizedproperty
  def refreshButton(self):
    ret = QtWidgets.QPushButton(tr_("Refresh"))
    ret.setToolTip(tr_("Refresh"))
    skqss.class_(ret, 'btn btn-info')
    ret.clicked.connect(self.refresh)
    ret.clicked.connect(lambda: growl.msg(my.tr("Reload dictionary caches")))
    return ret

  ## Wadoku

  @memoizedproperty
  def wadokuButton(self):
    ret = QtWidgets.QPushButton()
    ret.role = ''
    ret.clicked.connect(lambda:
        self._getWadoku() if ret.role == 'get' else
        self._removeWadoku() if ret.role == 'remove' else
        None)
    return ret

  @memoizedproperty
  def wadokuStatusLabel(self):
    ret = QtWidgets.QLabel()
    dic = dicts.wadoku()
    ret.linkActivated.connect(dic.open)
    path = QtCore.QDir.toNativeSeparators(dic.path)
    ret.setToolTip(path)
    return ret

  @memoizedproperty
  def wadokuIntroLabel(self):
    return QtWidgets.QLabel(my.tr("Wadoku (和独) Japanese-German dictionary") + " (134MB)")

  def _getWadoku(self):
    if prompt.confirmDownloadDictionary('Wadoku'):
      dic = dicts.wadoku()
      if not dic.exists(): #and not dic.locked():
        dic.get()
      if not self.refreshWadoku():
        self.startRefresh(dic, self.refreshWadoku)

  def _removeWadoku(self):
    if prompt.confirmRemoveDictionary('Wadoku'):
      dicts.wadoku().remove()
      settings.global_().setWadokuEnabled(False)
      self.refreshWadoku()

  def refreshWadoku(self): # -> bool exists
    b = self.wadokuButton
    status = self.wadokuStatusLabel
    dic = dicts.wadoku()
    if dic.exists():
      #status.setText(mytr_("Installed"))
      status.setText('<a href="#" style="%s">%s</a>' % (INSTALLED_STATUS_STYLE, mytr_("Installed")))
      skqss.class_(status, 'text-success')
      b.role = 'remove'
      b.setEnabled(True)
      b.setText(tr_("Remove"))
      skqss.class_(b, 'btn btn-default')
      return True
    elif dic.locked():
      status.setText(mytr_("Installing"))
      skqss.class_(status, 'text-info')
      b.role = ''
      b.setEnabled(False)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    else:
      online = netman.manager().isOnline()
      status.setText(mytr_("Not installed"))
      skqss.class_(status, 'text-error')
      b.role = 'get'
      b.setEnabled(online)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    return False

  ## EDICT

  @memoizedproperty
  def edictButton(self):
    ret = QtWidgets.QPushButton()
    ret.role = ''
    ret.clicked.connect(lambda:
        self._getEdict() if ret.role == 'get' else
        self._removeEdict() if ret.role == 'remove' else
        None)
    return ret

  @memoizedproperty
  def edictStatusLabel(self):
    ret = QtWidgets.QLabel()
    dic = dicts.edict()
    ret.linkActivated.connect(dic.open)
    path = QtCore.QDir.toNativeSeparators(dic.path)
    ret.setToolTip(path)
    return ret

  @memoizedproperty
  def edictIntroLabel(self):
    return QtWidgets.QLabel(my.tr("EDICT Japanese-English dictionary") + " (30MB)")

  def _getEdict(self):
    #if prompt.confirmDownloadDictionary('EDICT'):
    if prompt.confirmDownloadEDICT():
      dic = dicts.edict()
      if not dic.exists(): #and not dic.locked():
        dic.get()
      if not self.refreshEdict():
        self.startRefresh(dic, self.refreshEdict)

  def _removeEdict(self):
    #if prompt.confirmRemoveDictionary('EDICT'):
    if prompt.confirmRemoveEDICT():
      dicts.edict().remove()
      settings.global_().setEdictEnabled(False)
      self.refreshEdict()

  def refreshEdict(self): # -> bool exists
    b = self.edictButton
    status = self.edictStatusLabel
    dic = dicts.edict()
    if dic.exists():
      #status.setText(mytr_("Installed"))
      status.setText('<a href="#" style="%s">%s</a>' % (INSTALLED_STATUS_STYLE, mytr_("Installed")))
      skqss.class_(status, 'text-success')
      b.role = 'remove'
      b.setEnabled(True)
      b.setText(tr_("Remove"))
      skqss.class_(b, 'btn btn-default')
      return True
    elif dic.locked():
      status.setText(mytr_("Installing"))
      skqss.class_(status, 'text-info')
      b.role = ''
      b.setEnabled(False)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    else:
      online = netman.manager().isOnline()
      status.setText(mytr_("Not installed"))
      skqss.class_(status, 'text-error')
      b.role = 'get'
      b.setEnabled(online)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    return False

  ## Lingoes

  def getLingoesButton(self, name):
    ret = self.lingoesButtons.get(name)
    if not ret:
      ret = self.lingoesButtons[name] = QtWidgets.QPushButton()
      ret.role = ''
      ret.clicked.connect(partial(lambda name:
        self._getLingoes(name) if ret.role == 'get' else
        self._removeLingoes(name) if ret.role == 'remove' else
        None,
      name))
    return ret

  def getLingoesStatusLabel(self, name):
    ret = self.lingoesStatusLabels.get(name)
    if not ret:
      ret = self.lingoesStatusLabels[name] = QtWidgets.QLabel()
      dic = dicts.lingoes(name)
      ret.linkActivated.connect(dic.open)
      path = QtCore.QDir.toNativeSeparators(dic.path)
      ret.setToolTip(path)
    return ret

  def getLingoesIntroLabel(self, name):
    ret = self.lingoesIntroLabels.get(name)
    if not ret:
      if name.startswith('ja-zh'):
        t = "%s (%s, %s)" % (LINGOES_DICT_NAMES[name], LINGOES_DICT_SIZES[name], my.tr("recommended for Chinese"))
      elif name == 'ja-en':
        t = "%s (%s, %s)" % (LINGOES_DICT_NAMES[name], LINGOES_DICT_SIZES[name], my.tr("recommended for English"))
      else:
        t = "%s (%s)" % (LINGOES_DICT_NAMES[name], LINGOES_DICT_SIZES[name])
      ret = self.lingoesIntroLabels[name] = QtWidgets.QLabel(t)
    ret.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
    return ret

  def _getLingoes(self, name):
    if prompt.confirmDownloadDictionary('Lingoes (%s)' % name):
      dic = dicts.lingoes(name)
      if not dic.exists(): #and not dic.locked():
        dic.get()
      refresh = partial(self.refreshLingoes, name)
      if not refresh():
        self.startRefresh(dic, refresh)

  def _removeLingoes(self, name):
    if prompt.confirmRemoveDictionary('Lingoes (%s)' % name):
      dicts.lingoes(name).remove()
      settings.global_().setLingoesDictionaryEnabled(name, False)
      self.refreshLingoes(name)

  def refreshLingoes(self, name): # -> bool exists
    b = self.getLingoesButton(name)
    status = self.getLingoesStatusLabel(name)
    dic = dicts.lingoes(name)
    if dic.exists():
      #status.setText(mytr_("Installed"))
      status.setText('<a href="#" style="%s">%s</a>' % (INSTALLED_STATUS_STYLE, mytr_("Installed")))
      skqss.class_(status, 'text-success')
      b.role = 'remove'
      b.setEnabled(True)
      b.setText(tr_("Remove"))
      skqss.class_(b, 'btn btn-default')
      return True
    elif dic.locked():
      status.setText(mytr_("Installing"))
      skqss.class_(status, 'text-info')
      b.role = ''
      b.setEnabled(False)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    else:
      online = netman.manager().isOnline()
      status.setText(mytr_("Not installed"))
      skqss.class_(status, 'text-error')
      b.role = 'get'
      b.setEnabled(online)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    return False

  ## JMDict

  def getJMDictButton(self, name):
    ret = self.jmdictButtons.get(name)
    if not ret:
      ret = self.jmdictButtons[name] = QtWidgets.QPushButton()
      ret.role = ''
      ret.clicked.connect(partial(lambda name:
        self._getJMDict(name) if ret.role == 'get' else
        self._removeJMDict(name) if ret.role == 'remove' else
        None,
      name))
    return ret

  def getJMDictStatusLabel(self, name):
    ret = self.jmdictStatusLabels.get(name)
    if not ret:
      ret = self.jmdictStatusLabels[name] = QtWidgets.QLabel()
      dic = dicts.jmdict(name)
      ret.linkActivated.connect(dic.open)
      path = QtCore.QDir.toNativeSeparators(dic.path)
      ret.setToolTip(path)
    return ret

  def getJMDictIntroLabel(self, name):
    ret = self.jmdictIntroLabels.get(name)
    if not ret:
      ret = self.jmdictIntroLabels[name] = QtWidgets.QLabel(
          "%s (%s)" % (JMDICT_DICT_NAMES[name], JMDICT_DICT_SIZES[name]))
    ret.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
    return ret

  def _getJMDict(self, name):
    if prompt.confirmDownloadDictionary('JMDict (%s)' % name):
      dic = dicts.jmdict(name)
      if not dic.exists(): #and not dic.locked():
        dic.get()
      refresh = partial(self.refreshJMDict, name)
      if not refresh():
        self.startRefresh(dic, refresh)

  def _removeJMDict(self, name):
    if prompt.confirmRemoveDictionary('JMDict (%s)' % name):
      dicts.jmdict(name).remove()
      settings.global_().setJMDictEnabled(name, False)
      self.refreshJMDict(name)

  def refreshJMDict(self, name): # -> bool exists
    b = self.getJMDictButton(name)
    status = self.getJMDictStatusLabel(name)
    dic = dicts.jmdict(name)
    if dic.exists():
      #status.setText(mytr_("Installed"))
      status.setText('<a href="#" style="%s">%s</a>' % (INSTALLED_STATUS_STYLE, mytr_("Installed")))
      skqss.class_(status, 'text-success')
      b.role = 'remove'
      b.setEnabled(True)
      b.setText(tr_("Remove"))
      skqss.class_(b, 'btn btn-default')
      return True
    elif dic.locked():
      status.setText(mytr_("Installing"))
      skqss.class_(status, 'text-info')
      b.role = ''
      b.setEnabled(False)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    else:
      online = netman.manager().isOnline()
      status.setText(mytr_("Not installed"))
      skqss.class_(status, 'text-error')
      b.role = 'get'
      b.setEnabled(online)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    return False

  def refresh(self):
    self.refreshEdict()
    self.refreshWadoku()

    map(self.refreshMeCab, config.MECAB_DICS)
    map(self.refreshCaboCha, config.CABOCHA_DICS)
    map(self.refreshJMDict, config.JMDICT_LANGS)

    for lang in config.LINGOES_LANGS:
      name = 'ja-' + lang
      self.refreshLingoes(name)

  def startRefresh(self, dic, refresh):  # dic, ->
    self._refreshTasks.append((dic, refresh))
    if not self.refreshTimer.isActive():
      self.refreshTimer.start()

  def stopRefresh(self):
    if self.refreshTimer.isActive():
      self.refreshTimer.stop()
    if self._refreshTasks:
      self._refreshTasks = []

  def _tryRefresh(self):
    if not self._refreshTasks:
      self.stopRefresh()
      return

    exists = False
    tasks = []
    for t in self._refreshTasks:
      dic, refresh = t
      if not dic.exists():
        tasks.append(t)
      #else:
      refresh()

    if len(tasks) != len(self._refreshTasks):
      self._refreshTasks = tasks
      if not tasks:
        self.stopRefresh()

class DictionaryDownloadsTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(DictionaryDownloadsTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _DictionaryDownloadsTab(self)
    #self.setChildrenCollapsible(False)
    #self.setMinimumWidth(DOWNLOADS_MINIMUM_WIDTH)

  def save(self): pass #self.__d.save()
  def load(self): pass
  def refresh(self): self.__d.refresh()

  def stop(self): self.__d.stopRefresh()

# Launcher downloads

#@Q_Q
class _LauncherDownloadsTab(object):
  def __init__(self, q):
    t = self.refreshTimer = QTimer(q)
    t.setSingleShot(False)
    t.setInterval(DOWNLOAD_REFRESH_INTERVAL)
    t.timeout.connect(self._tryRefresh)

    self._createUi(q)


  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    if not features.WINE:
      layout.addWidget(self.applocGroup)
    layout.addStretch()
    q.setLayout(layout)

  ## AppLocale

  @memoizedproperty
  def applocGroup(self):
    ret = QtWidgets.QGroupBox("Microsoft AppLocale (@piaip, 13MB)")
    layout = QtWidgets.QVBoxLayout()

    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.applocButton)
    row.addWidget(self.applocLabel)
    row.addStretch()
    layout.addLayout(row)

    layout.addWidget(self.applocEdit)
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def applocButton(self):
    ret = QtWidgets.QPushButton()
    ret.role = ''
    ret.clicked.connect(lambda:
        self._getApploc() if ret.role == 'get' else
        self._removeApploc() if ret.role == 'remove' else
        None)
    return ret

  @memoizedproperty
  def applocLabel(self):
    return QtWidgets.QLabel()

  @memoizedproperty
  def applocEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    #ret.setMaximumHeight(DOWNLOADS_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)

    url = res.AppLocale.URL
    ret.setHtml(my.tr(
"""Microsoft AppLocale is required to change the game locale to Japanese.
It is not needed if you have already changed your Windows locale to Japanese.
<br/>
<br/>
In 2004, @piaip from Taiwan University released a bugfix version of AppLocale (named as pAppLocale) which eliminated the annoying popups.
VNR can automatically download pAppLocale from Taiwan University.
You can also install pAppLocale manually from the @piaip's homepage at Taiwan University:
<center><a href="%s">%s</a></center>""") % (url, url))
    return ret

  def _getApploc(self):
    if prompt.confirmDownloadApploc():
      r = res.apploc()
      if not r.exists(): #and not dic.locked():
        r.get()
      if not self.refreshApploc():
        self.startRefresh()

  def _removeApploc(self):
    if prompt.confirmRemoveApploc():
      res.apploc().remove()
      self.refreshApploc()

  def refreshApploc(self): # -> bool exists
    b = self.applocButton
    label = self.applocLabel
    r = res.apploc()
    if r.exists():
      label.setText(mytr_("Installed"))
      skqss.class_(label, 'text-success')
      b.role = 'remove'
      b.setEnabled(True)
      b.setText(tr_("Remove"))
      skqss.class_(b, 'btn btn-default')
      return True
    elif r.locked():
      label.setText(mytr_("Installing"))
      skqss.class_(label, 'text-info')
      b.role = ''
      b.setEnabled(False)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    else:
      online = netman.manager().isOnline()
      label.setText(mytr_("Not installed"))
      skqss.class_(label, 'text-error')
      b.role = 'get'
      b.setEnabled(online)
      b.setText(tr_("Install"))
      skqss.class_(b, 'btn btn-primary')
    return False

  def refresh(self):
    if not features.WINE:
      self.refreshApploc()

  def startRefresh(self):
    self.refreshTimer.start()
  def stopRefresh(self):
    if self.refreshTimer.isActive():
      self.refreshTimer.stop()

  def _tryRefresh(self):
    self.refreshApploc()
    r = res.apploc()
    if r.exists():
      self.stopRefresh()

class LauncherDownloadsTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(LauncherDownloadsTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _LauncherDownloadsTab(self)
    #self.setChildrenCollapsible(False)
    #self.setMinimumWidth(DOWNLOADS_MINIMUM_WIDTH)

  def save(self): pass #self.__d.save()
  def load(self): pass
  def refresh(self): self.__d.refresh()

  def stop(self): self.__d.stopRefresh()

# External libraries

#LIBRARY_MINIMUM_WIDTH = 320
LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT = 100

class LibraryTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(LibraryTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    #self.setMinimumWidth(LIBRARY_MINIMUM_WIDTH)

    label = QtWidgets.QLabel(my.tr(
"""Some of the features of VNR need external applications to be installed.
You can specify the locations of external libraries here."""))
    label.setWordWrap(True)

    layout = skwidgets.SkWidgetLayout(label)
    w = QtWidgets.QGroupBox(tr_("About"))
    w.setLayout(layout)

    rows = QtWidgets.QVBoxLayout()
    rows.addWidget(w)
    rows.addStretch()
    self.setLayout(rows)

  def save(self): pass #self.__d.save()
  def load(self): pass
  def refresh(self): pass

# Locale library

@Q_Q
class _LocaleLibraryTab(object):
  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    if features.WINE:
      label =QtWidgets.QLabel(my.tr(
"""You don't have to change Japanese games' locale in Wine.
Just don't forget to export LC_ALL=ja_JP.UTF8 before launching VNR."""))
      l = skwidgets.SkWidgetLayout(label)
      w = QtWidgets.QGroupBox(tr_("About"))
      w.setLayout(l)
      layout.addWidget(w)
    else:
      #layout.addWidget(self.appLocaleGroup)
      layout.addWidget(self.localeEmulatorGroup)
      layout.addWidget(self.ntleasGroup)
    layout.addStretch()
    q.setLayout(layout)

  ## Locale Emulator ##

  @memoizedproperty
  def localeEmulatorGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.localeEmulatorLocationEdit)
    editRow.addWidget(self.localeEmulatorLocationButton)
    editRow.addWidget(self.localeEmulatorLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.localeEmulatorInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("Locale Emulator (2MB)"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def localeEmulatorLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def localeEmulatorLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(notr_("Locale Emulator")))
    ret.clicked.connect(self._getLocaleEmulatorLocation)
    return ret

  @memoizedproperty
  def localeEmulatorLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearLocaleEmulatorLocation)
    return ret

  @memoizedproperty
  def localeEmulatorInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _getLocaleEmulatorLocation(self):
    path = libman.localeEmulator().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"LEProc.exe"'),
        path, 0)
    if path:
      if not libman.localeEmulator().verifyLocation(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format(notr_("Locale Emulator")))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        libman.localeEmulator().setLocation(path)
        self._refreshLocaleEmulator()

  def _clearLocaleEmulatorLocation(self):
    libman.localeEmulator().setLocation('')
    self._refreshLocaleEmulator()

  def _refreshLocaleEmulator(self):
    libman.localeEmulator().refresh()
    ok = libman.localeEmulator().exists()
    if ok:
      path = libman.localeEmulator().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)

    self.localeEmulatorLocationClearButton.setVisible(ok)

    skqss.class_(self.localeEmulatorLocationEdit, 'normal' if ok else 'muted')
    self.localeEmulatorLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(notr_("Locale Emulator")))

    url = libman.LocaleEmulator.URL
    thank_url = 'http://bbs.sumisora.org/read.php?tid=11045407'
    self.localeEmulatorInfoEdit.setHtml(my.tr(
"""Locale Emulator could help change Japanese game to your locale.
It is similar to NTLEA, and seems to work better in Windows 7/8.
You could get the latest version here:
<center><a href="%s">%s</a></center>
You can say THANKS to its author here:
<center><a href="%s">%s</a></center>
""") % (url,url, thank_url,thank_url))

  ## Ntleas ##

  @memoizedproperty
  def ntleasGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.ntleasLocationEdit)
    editRow.addWidget(self.ntleasLocationButton)
    editRow.addWidget(self.ntleasLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.ntleasInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("Ntleas (1MB)"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def ntleasLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def ntleasLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(notr_("Ntleas")))
    ret.clicked.connect(self._getNtleasLocation)
    return ret

  @memoizedproperty
  def ntleasLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearNtleasLocation)
    return ret

  @memoizedproperty
  def ntleasInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _getNtleasLocation(self):
    path = libman.ntleas().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"ntleas.exe (x86)"'),
        path, 0)
    if path:
      if not libman.ntleas().verifyLocation(path):
        path = os.path.join(path, 'x86') # append x86
        if not libman.ntleas().verifyLocation(path):
          growl.error(my.tr("Couldn't find {0} from the specified location").format(notr_("Ntleas")))
          return;
      path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      libman.ntleas().setLocation(path)
      self._refreshNtleas()

  def _clearNtleasLocation(self):
    libman.ntleas().setLocation('')
    self._refreshNtleas()

  def _refreshNtleas(self):
    libman.ntleas().refresh()
    ok = libman.ntleas().exists()
    if ok:
      path = libman.ntleas().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)

    self.ntleasLocationClearButton.setVisible(ok)

    skqss.class_(self.ntleasLocationEdit, 'normal' if ok else 'muted')
    self.ntleasLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(notr_("Ntleas")))

    url = libman.Ntleas.URL
    self.ntleasInfoEdit.setHtml(my.tr(
"""Ntleas is an application that could help change Japanese game to your locale.<br/>
It is a descendant of NTLEA whose development has stopped.
Ntleas is <span style="color:purple">free</span> and open source. You could get the latest version here:
<center><a href="{0}">{0}</a></center>""").format(url))

#  ## AppLocale ##
#
#  @memoizedproperty
#  def appLocaleGroup(self):
#    layout = QtWidgets.QVBoxLayout()
#    layout.addWidget(self.appLocaleLocationEdit)
#    layout.addWidget(self.appLocaleInfoEdit)
#    ret = QtWidgets.QGroupBox(notr_("Microsoft AppLocale (13MB)"))
#    ret.setLayout(layout)
#    return ret
#
#  @memoizedproperty
#  def appLocaleLocationEdit(self):
#    ret = QtWidgets.QLineEdit()
#    ret.setReadOnly(True)
#    ret.setToolTip(tr_("Location"))
#    return ret
#
#  @memoizedproperty
#  def appLocaleInfoEdit(self):
#    ret = QtWidgets.QTextBrowser()
#    skqss.class_(ret, 'texture')
#    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
#    ret.setAlignment(Qt.AlignCenter)
#    ret.setReadOnly(True)
#    ret.setOpenExternalLinks(True)
#    return ret
#
#  def _refreshAppLocale(self):
#    libman.apploc().refresh()
#    ok = libman.apploc().exists()
#    if ok:
#      path = libman.apploc().location()
#      path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
#    skqss.class_(self.appLocaleLocationEdit, 'normal' if ok else 'muted')
#    self.appLocaleLocationEdit.setText(path if ok else tr_("Not found"))
#
#    url = libman.AppLocale.URL
#    self.appLocaleInfoEdit.setHtml(my.tr(
#"""Microsoft AppLocale is used to change the game locale to Japanese.<br/>
#It is not needed if you have changed your OS locale to Japanese.<br/>
#AppLocale is detected on your system at the above location.""")
#    if ok else my.tr(
#"""Microsoft AppLocale is required to change the game locale to Japanese.<br/>
#It is not needed if you have already changed your Windows locale to Japanese.<br/>
#You can get a free version of AppLocale here from Microsoft:
#<center><a href="%s">%s</a></center>""") % (url, url))

  def refresh(self):
    #self._refreshNtlea()
    #self._refreshAppLocale()
    self._refreshLocaleEmulator()
    self._refreshNtleas()

  #def save(self):
  #  self._saveJBeijing()
  #  self._saveQuickTime()

class LocaleLibraryTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(LocaleLibraryTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _LocaleLibraryTab(self)
    #self.setChildrenCollapsible(False)
    #self.setMinimumWidth(LIBRARY_MINIMUM_WIDTH)

  def load(self): self.__d.refresh()
  def save(self): pass #self.__d.save()
  def refresh(self): pass

# Translator Library

@Q_Q
class _TranslatorLibraryTab(object):
  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    blans = settings.global_().blockedLanguages()
    layout = QtWidgets.QVBoxLayout()
    if 'zh' not in blans:
      layout.addWidget(self.jbeijingGroup)
      layout.addWidget(self.fastaitGroup)
      layout.addWidget(self.dreyeGroup)
    if 'ko' not in blans:
      layout.addWidget(self.ezTransGroup)
    if 'en' not in blans:
      layout.addWidget(self.atlasGroup)
      layout.addWidget(self.lecGroup)
    layout.addStretch()
    q.setLayout(layout)

  ## JBeijing ##

  @memoizedproperty
  def jbeijingGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.jbeijingLocationEdit)
    editRow.addWidget(self.jbeijingLocationButton)
    editRow.addWidget(self.jbeijingLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.jbeijingInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("Kodensha JBeijing") + u" (高電社・J北京, 517MB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def jbeijingLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def jbeijingLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(mytr_("JBeijing7")))
    ret.clicked.connect(self._getJBeijingLocation)
    return ret

  @memoizedproperty
  def jbeijingLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearJBeijingLocation)
    return ret

  @memoizedproperty
  def jbeijingInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _clearJBeijingLocation(self):
    libman.jbeijing().setLocation('')
    self._refreshJBeijing()

  def _getJBeijingLocation(self):
    path = libman.jbeijing().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"JCT.exe"'),
        path, 0)
    if path:
      if not libman.jbeijing().verifyLocation(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format(mytr_("JBeijing7")))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        libman.jbeijing().setLocation(path)
        self._refreshJBeijing()

        if not skstr.isascii(path):
          growl.warn(my.tr("You have non-ascii characters in the path which might work as expected"))

  def _refreshJBeijing(self):
    libman.jbeijing().refresh()
    ok = libman.jbeijing().exists()
    if ok:
      path = libman.jbeijing().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)

    self.jbeijingLocationClearButton.setVisible(ok)

    #self.jbeijingLocationButton.setVisible(not ok)
    skqss.class_(self.jbeijingLocationEdit,
        'normal' if ok and skstr.isascii(path) else
        'error' if ok else
        'muted')
    self.jbeijingLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(mytr_("JBeijing7")))

    url = libman.JBeijing.URL
    self.jbeijingInfoEdit.setHtml((my.tr(
"""Kodensha JBeijing v7 is used by <span style="color:purple">offline Japanese-Chinese</span> translation.<br/>
JBeijing is detected on your system at the above location.""")
    if ok else my.tr(
"""Kodensha JBeijing v7 is needed by <span style="color:purple">offline Japanese-Chinese</span> translation.<br/>
JBeijing is <span style="color:purple">not free</span>, and you can purchase one here from Kodensha:
<center><a href="%s">%s</a></center>""") % (url, url))
+ "<br/>" + my.tr('<span style="color:red">Note: The path cannot contain non-English characters!</span>')
)

  #def _saveJBeijing(self):
  #  path = self.jbeijingLocationEdit.text().strip()
  #  if libman.jbeijing().verifyLocation(path):
  #    path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
  #    settings.global_().setJBeijingLocation(path)

  ## FastAIT ##

  @memoizedproperty
  def fastaitGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.fastaitLocationEdit)
    editRow.addWidget(self.fastaitLocationButton)
    editRow.addWidget(self.fastaitLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.fastaitInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("Kingsoft FastAIT") + u" (金山快譯, 534MB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def fastaitLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def fastaitLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(mytr_("FastAIT")))
    ret.clicked.connect(self._getFastaitLocation)
    return ret

  @memoizedproperty
  def fastaitLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearFastaitLocation)
    return ret

  @memoizedproperty
  def fastaitInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _getFastaitLocation(self):
    path = libman.fastait().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"NetTrans.exe"'),
        path, 0)
    if path:
      if not libman.fastait().verifyLocation(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format(mytr_("FastAIT")))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        libman.fastait().setLocation(path)
        self._refreshFastait()

        if not skstr.isascii(path):
          growl.warn(my.tr("You have non-ascii characters in the path which might work as expected"))

  def _clearFastaitLocation(self):
    libman.fastait().setLocation('')
    self._refreshFastait()

  def _refreshFastait(self):
    libman.fastait().refresh()
    ok = libman.fastait().exists()
    if ok:
      path = libman.fastait().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)
    self.fastaitLocationClearButton.setVisible(ok)
    #self.fastaitLocationButton.setVisible(not ok)
    skqss.class_(self.fastaitLocationEdit,
        'normal' if ok and skstr.isascii(path) else
        'error' if ok else
        'muted')
    self.fastaitLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(mytr_("FastAIT")))

    url = libman.FastAIT.URL
    self.fastaitInfoEdit.setHtml(my.tr(
"""Kingsoft FastAIT All Professional Edition is used by <span style="color:purple">offline Chinese-Japanese/English</span> translation.<br/>
FastAIT is detected on your system at the above location.""")
    if ok else my.tr(
"""Kingsoft FastAIT All Professional Edition is needed by <span style="color:purple">offline Chinese-Japanese/English</span> translation.<br/>
FastAIT is <span style="color:green">free</span>, and you can download one here from iCIBA:
<center><a href="%s">%s</a></center>""") % (url, url))

  ## Dr.eye ##

  @memoizedproperty
  def dreyeGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.dreyeLocationEdit)
    editRow.addWidget(self.dreyeLocationButton)
    editRow.addWidget(self.dreyeLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.dreyeInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("Inventec Dr.eye") + u" (英業達・譯典通, 1.35GB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def dreyeLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def dreyeLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(mytr_("Dr.eye")))
    ret.clicked.connect(self._getDreyeLocation)
    return ret

  @memoizedproperty
  def dreyeLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearDreyeLocation)
    return ret

  @memoizedproperty
  def dreyeInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _getDreyeLocation(self):
    path = libman.dreye().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"Dreye.exe"'),
        path, 0)
    if path:
      if not libman.dreye().verifyLocation(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format(mytr_("Dr.eye")))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        libman.dreye().setLocation(path)
        self._refreshDreye()

        if not skstr.isascii(path):
          growl.warn(my.tr("You have non-ascii characters in the path which might work as expected"))

  def _clearDreyeLocation(self):
    libman.dreye().setLocation('')
    self._refreshDreye()

  def _refreshDreye(self):
    libman.dreye().refresh()
    ok = libman.dreye().exists()
    if ok:
      path = libman.dreye().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)
    self.dreyeLocationClearButton.setVisible(ok)
    #self.dreyeLocationButton.setVisible(not ok)
    skqss.class_(self.dreyeLocationEdit,
        'normal' if ok and skstr.isascii(path) else
        'error' if ok else
        'muted')
    self.dreyeLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(mytr_("Dr.eye")))

    url = libman.Dreye.URL
    self.dreyeInfoEdit.setHtml(my.tr(
"""Inventec Dr.eye v9.0 is used by <span style="color:purple">offline Chinese-Japanese/English</span> translation.<br/>
Dr.eye is detected on your system at the above location.""")
    if ok else my.tr(
"""Inventec Dr.eye v9.0 is needed by <span style="color:purple">offline Chinese-Japanese/English</span> translation.<br/>
Dr.eye is <span style="color:purple">not free</span>, and you can purchase one here from Inventec:
<center><a href="%s">%s</a></center>""") % (url, url))

  ## ezTrans ##

  @memoizedproperty
  def ezTransGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.ezTransLocationEdit)
    editRow.addWidget(self.ezTransLocationButton)
    editRow.addWidget(self.ezTransLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.ezTransInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("ChangShin Soft - ezTrans XP (218MB)"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def ezTransLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def ezTransLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(mytr_("ezTrans XP")))
    ret.clicked.connect(self._getEzTransLocation)
    return ret

  @memoizedproperty
  def ezTransLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearEzTransLocation)
    return ret

  @memoizedproperty
  def ezTransInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _getEzTransLocation(self):
    path = libman.eztrans().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"ezTrans.exe"'),
        path, 0)
    if path:
      if not libman.eztrans().verifyLocation(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format(mytr_("ezTrans XP")))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        libman.eztrans().setLocation(path)
        self._refreshEzTrans()

        if not skstr.isascii(path):
          growl.warn(my.tr("You have non-ascii characters in the path which might work as expected"))

  def _clearEzTransLocation(self):
    libman.eztrans().setLocation('')
    self._refreshEzTrans()

  def _refreshEzTrans(self):
    libman.eztrans().refresh()
    ok = libman.eztrans().exists()
    if ok:
      path = libman.eztrans().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)
    self.ezTransLocationClearButton.setVisible(ok)
    #self.eztransLocationButton.setVisible(not ok)
    skqss.class_(self.ezTransLocationEdit,
        'normal' if ok and skstr.isascii(path) else
        'error' if ok else
        'muted')
    self.ezTransLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(mytr_("ezTrans XP")))

    url = libman.EzTrans.URL
    self.ezTransInfoEdit.setHtml(my.tr(
"""ChangShin Soft ezTrans XP is used by <span style="color:purple">offline Japanese-Korean</span> translation.<br/>
ezTrans is detected on your system at the above location.""")
    if ok else my.tr(
"""ChangShin Soft ezTrans XP is needed by <span style="color:purple">offline Japanese-Korean</span> translation.<br/>
ezTrans is <span style="color:purple">not free</span>, and you can purchase one here from ChangShin Soft:
<center><a href="%s">%s</a></center>""") % (url, url))

  ## Atlas ##

  @memoizedproperty
  def atlasGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.atlasLocationEdit)
    editRow.addWidget(self.atlasLocationButton)
    editRow.addWidget(self.atlasLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.atlasInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("Fujitsu Atlas") + u" (富士通・ATLAS, 597MB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def atlasLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def atlasLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(mytr_("ATLAS")))
    ret.clicked.connect(self._getAtlasLocation)
    return ret

  @memoizedproperty
  def atlasLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearAtlasLocation)
    return ret

  def _clearAtlasLocation(self):
    libman.atlas().setLocation('')
    self._refreshAtlas()

  def _getAtlasLocation(self):
    path = libman.atlas().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"Atlas.exe"'),
        path, 0)
    if path:
      if not libman.atlas().verifyLocation(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format(mytr_("ATLAS")))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        libman.atlas().setLocation(path)
        self._refreshAtlas()

        if not skstr.isascii(path):
          growl.warn(my.tr("You have non-ascii characters in the path which might work as expected"))

  @memoizedproperty
  def atlasInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _refreshAtlas(self):
    libman.atlas().refresh()
    ok = libman.atlas().exists()
    if ok:
      path = libman.atlas().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)
    self.atlasLocationClearButton.setVisible(ok)
    #self.atlasLocationButton.setVisible(not ok)
    skqss.class_(self.atlasLocationEdit,
        'normal' if ok and skstr.isascii(path) else
        'error' if ok else
        'muted')
    self.atlasLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(mytr_("ATLAS")))

    url = libman.Atlas.URL
    self.atlasInfoEdit.setHtml(my.tr(
"""Fujitsu Atlas v14 is used for <span style="color:purple">offline Japanese-English</span> translation.<br/>
Atlas is detected on your system at the above location.""")
    if ok else my.tr(
"""Fujitsu Atlas v14 is needed by <span style="color:purple">offline Japanese-English</span> translation.<br/>
You can get a free version of Atlas here from Fujitsu:
<center><a href="%s">%s</a></center>""") % (url, url))

  ## LEC ##

  @memoizedproperty
  def lecGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.lecLocationEdit)
    editRow.addWidget(self.lecLocationButton)
    editRow.addWidget(self.lecLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.lecInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("LEC Power Translator (1.74GB)"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def lecLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def lecLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(mytr_("LEC")))
    ret.clicked.connect(self._getLecLocation)
    return ret

  @memoizedproperty
  def lecLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearLecLocation)
    return ret

  def _clearLecLocation(self):
    libman.lec().setLocation('')
    self._refreshLec()

  def _getLecLocation(self):
    path = libman.lec().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"LogoMedia TranslateDotNet Server.exe"'),
        path, 0)
    if path:
      if not libman.lec().verifyLocation(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format(mytr_("LEC")))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        libman.lec().setLocation(path)
        self._refreshLec()

        if not skstr.isascii(path):
          growl.warn(my.tr("You have non-ascii characters in the path which might work as expected"))

  @memoizedproperty
  def lecInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _refreshLec(self):
    libman.lec().refresh()
    ok = libman.lec().exists()
    if ok:
      path = libman.lec().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)
    self.lecLocationClearButton.setVisible(ok)
    #self.lecLocationButton.setVisible(not ok)
    skqss.class_(self.lecLocationEdit,
        'normal' if ok and skstr.isascii(path) else
        'error' if ok else
        'muted')
    self.lecLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(mytr_("ATLAS")))

    url = libman.Lec.URL
    self.lecInfoEdit.setHtml(my.tr(
"""LEC Power Translator v15 is used for <span style="color:purple">offline Japanese-English</span> translation.<br/>
Power Translator is detected on your system at the above location.""")
    if ok else my.tr(
"""LEC Power Translator v15 is needed by <span style="color:purple">offline Japanese-English</span> translation.
It is <span style="color:purple">not free</span>, and you can purchase one here from LEC:
<center><a href="%s">%s</a></center>""") % (url, url))

  def refresh(self):
    self._refreshJBeijing()
    self._refreshFastait()
    self._refreshDreye()
    self._refreshEzTrans()
    self._refreshAtlas()
    self._refreshLec()

class TranslatorLibraryTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(TranslatorLibraryTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _TranslatorLibraryTab(self)
    #self.setChildrenCollapsible(False)
    #self.setMinimumWidth(LIBRARY_MINIMUM_WIDTH)

  def load(self): self.__d.refresh()
  def save(self): pass #self.__d.save()
  def refresh(self): pass

# Dictionary Library

@Q_Q
class _DictionaryLibraryTab(object):
  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.msimeGroup)
    blans = settings.global_().blockedLanguages()
    #if 'ja' not in blans:
    layout.addWidget(self.daijirinGroup)
    layout.addWidget(self.kojienGroup)
    if 'zh' not in blans:
      layout.addWidget(self.zhongriGroup)
    #if 'de' not in blans:
    #  layout.addWidget(self.wadokuGroup)
    layout.addStretch()
    q.setLayout(layout)

  ## Microsoft Japanese IME ##

  @memoizedproperty
  def msimeGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.msimeLocationEdit)
    layout.addWidget(self.msimeInfoEdit)
    ret = QtWidgets.QGroupBox(tr_("Microsoft Japanese IME"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def msimeLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def msimeInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  _MSIME_LOCATION = None # cached
  def _refreshMsime(self):
    ok = msime.ja_valid()
    path = None
    if ok:
      path = self._MSIME_LOCATION or msime.ja_ime_location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        self._MSIME_LOCATION = path
    skqss.class_(self.msimeLocationEdit, 'normal' if ok else 'muted')
    self.msimeLocationEdit.setText(path if path else tr_("Not found"))

    urls = (msime.JA_INSTALL_URL, msime.JA_INSTALL_URL,
            msime.JA_UPDATE_URL, msime.JA_UPDATE_URL)
    self.msimeInfoEdit.setHtml(my.tr(
"""Microsoft Japanese IME is detected on your system at the above location.<br/>
It could significantly improve the quality of Japanese analysis.<br/>
It is <span style="color:purple">free</span> for Office users, and you can get the latest version here from Microsoft:
<center><a href="%s">%s</a></center>
And here's an article illustrating how to update its dictionaries:
<center><a href="%s">%s</a></center>""") % urls
    if ok else my.tr(
"""Microsoft Japanese IME could significantly improve the quality of Japanese analysis.<br/>
It is <span style="color:purple">free</span> for Office users, and you can get the latest version here from Microsoft:
<center><a href="%s">%s</a></center>
And here's an article illustrating how to update its dictionaries:
<center><a href="%s">%s</a></center>""") % urls)

  ## Kojien ##

  @memoizedproperty
  def kojienGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.kojienLocationEdit)
    editRow.addWidget(self.kojienLocationButton)
    editRow.addWidget(self.kojienLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.kojienInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("Iwanami Kojien") + u" (岩波書店・広辞苑, 3.27GB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def kojienLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def kojienLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(mytr_("Kojien")))
    ret.clicked.connect(self._getKojienLocation)
    return ret

  @memoizedproperty
  def kojienLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearKojienLocation)
    return ret

  @memoizedproperty
  def kojienInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _clearKojienLocation(self):
    path = ''
    ebdict.kojien().setLocation(path)
    settings.global_().setKojienLocation(path)
    self._refreshKojien()

  def _getKojienLocation(self):
    path = ebdict.kojien().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"CATALOG(S)"'),
        path, 0)
    if path:
      if not ebdict.KojienDic.verifyLocation(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format(mytr_("Kojien")))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        ebdict.kojien().setLocation(path)
        if ebdict.kojien().exists():
          settings.global_().setKojienLocation(path)
        self._refreshKojien()

  def _refreshKojien(self):
    ok = ebdict.kojien().exists()
    if ok:
      path = ebdict.kojien().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)
    self.kojienLocationClearButton.setVisible(ok)
    #self.kojienLocationButton.setVisible(not ok)
    skqss.class_(self.kojienLocationEdit, 'normal' if ok else 'muted')
    self.kojienLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(mytr_("Kojien")))

    epwing_url = "http://ja.wikipedia.org/wiki/EPWING"
    url = ebdict.KojienDic.URL
    self.kojienInfoEdit.setHtml(my.tr(
"""Iwanami Kojien dictionary is used by <span style="color:purple">offline Japanese</span> word translation.<br/>
Kojien is detected on your system at the above location.""")
    if ok else my.tr(
"""Iwanami Kojien is needed by <span style="color:purple">offline Japanese</span> word translation.<br/>
It is a ja-ja dictionary distributed in <a href="%s">EPWING</a> format DVD.<br/>
Kojien is <span style="color:purple">not free</span>, and you can purchase one here from Iwanami:
<center><a href="%s">%s</a></center>""") % (epwing_url, url, url)
+ "<br/>" +  my.tr('<span style="color:red">Note: EPWING path cannot contain non-English characters!</span>')
)

  ## Daijirin ##

  @memoizedproperty
  def daijirinGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.daijirinLocationEdit)
    editRow.addWidget(self.daijirinLocationButton)
    editRow.addWidget(self.daijirinLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.daijirinInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("Sanseido Daijirin") + u" (三省堂・大辞林, 646MB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def daijirinLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def daijirinLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(mytr_("Daijirin")))
    ret.clicked.connect(self._getDaijirinLocation)
    return ret

  @memoizedproperty
  def daijirinLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearDaijirinLocation)
    return ret

  @memoizedproperty
  def daijirinInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _clearDaijirinLocation(self):
    path = ''
    ebdict.daijirin().setLocation(path)
    settings.global_().setDaijirinLocation(path)
    self._refreshDaijirin()

  def _getDaijirinLocation(self):
    path = ebdict.daijirin().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"CATALOG(S)"'),
        path, 0)
    if path:
      if not ebdict.DaijirinDic.verifyLocation(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format(mytr_("Daijirin")))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        ebdict.daijirin().setLocation(path)
        if ebdict.daijirin().exists():
          settings.global_().setDaijirinLocation(path)
        self._refreshDaijirin()

  def _refreshDaijirin(self):
    ok = ebdict.daijirin().exists()
    if ok:
      path = ebdict.daijirin().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)
    self.daijirinLocationClearButton.setVisible(ok)
    #self.daijirinLocationButton.setVisible(not ok)
    skqss.class_(self.daijirinLocationEdit, 'normal' if ok else 'muted')
    self.daijirinLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(mytr_("Daijirin")))

    epwing_url = "http://ja.wikipedia.org/wiki/EPWING"
    url = ebdict.DaijirinDic.URL
    self.daijirinInfoEdit.setHtml(my.tr(
"""Sanseido Daijirin dictionary is used by <span style="color:purple">offline Japanese and English</span> word translation.<br/>
Daijirin is detected on your system at the above location.""")
    if ok else my.tr(
"""Sanseido Daijirin is needed by <span style="color:purple">offline Japanese and English</span> word translation.<br/>
It is a ja-ja/ja-en dictionary distributed in <a href="%s">EPWING</a> format DVD.<br/>
Daijirin is <span style="color:purple">not free</span>, and you can purchase one here from Sanseido:
<center><a href="%s">%s</a></center>""") % (epwing_url, url, url)
+ "<br/>" +  my.tr('<span style="color:red">Note: EPWING path cannot contain non-English characters!</span>')
)

#  ## Wadoku ##
#
#  @memoizedproperty
#  def wadokuGroup(self):
#    layout = QtWidgets.QVBoxLayout()
#    editRow = QtWidgets.QHBoxLayout()
#    editRow.addWidget(self.wadokuLocationEdit)
#    editRow.addWidget(self.wadokuLocationButton)
#    layout.addLayout(editRow)
#    layout.addWidget(self.wadokuInfoEdit)
#    ret = QtWidgets.QGroupBox(notr_("Wadoku") + u" (和独辞書, 134MB)")
#    ret.setLayout(layout)
#    return ret
#
#  @memoizedproperty
#  def wadokuLocationEdit(self):
#    ret = QtWidgets.QLineEdit()
#    ret.setReadOnly(True)
#    ret.setToolTip(tr_("Location"))
#    return ret
#
#  @memoizedproperty
#  def wadokuLocationButton(self):
#    ret = QtWidgets.QPushButton(tr_("Browse"))
#    skqss.class_(ret, BROWSE_BTN_CLASS)
#    ret.setToolTip(my.tr("Select the location of {0}").format(mytr_("Wadoku")))
#    ret.clicked.connect(self._getWadokuLocation)
#    return ret
#
#  @memoizedproperty
#  def wadokuInfoEdit(self):
#    ret = QtWidgets.QTextBrowser()
#    skqss.class_(ret, 'texture')
#    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
#    ret.setAlignment(Qt.AlignCenter)
#    ret.setReadOnly(True)
#    ret.setOpenExternalLinks(True)
#    return ret
#
#  def _getWadokuLocation(self):
#    path = ebdict.wadoku().location() or skpaths.HOME
#    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
#        my.tr("Please select the folder containing {0}").format('"CATALOG(S)"'),
#        path, 0)
#    if path:
#      if not ebdict.WadokuDic.verifyLocation(path):
#        growl.error(my.tr("Couldn't find {0} from the specified location").format(mytr_("Wadoku")))
#      else:
#        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
#        ebdict.wadoku().setLocation(path)
#        if ebdict.wadoku().exists():
#          settings.global_().setWadokuLocation(path)
#        self._refreshWadoku()
#
#  def _refreshWadoku(self):
#    ok = ebdict.wadoku().exists()
#    if ok:
#      path = ebdict.wadoku().location()
#      if path:
#        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
#      ok = bool(path) and os.path.exists(path)
#    #self.wadokuLocationButton.setVisible(not ok)
#    skqss.class_(self.wadokuLocationEdit, 'normal' if ok else 'muted')
#    self.wadokuLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(mytr_("Wadoku")))
#
#    epwing_url = "http://ja.wikipedia.org/wiki/EPWING"
#    edict_url = "http://en.wikipedia.org/wiki/Edict"
#    url = ebdict.WadokuDic.URL
#    self.wadokuInfoEdit.setHtml(my.tr(
#"""Wadoku is used by <span style="color:purple">offline Japanese-German</span> word translation.<br/>
#Wadoku is detected on your system at the above location.""")
#    if ok else my.tr(
#"""Wadoku is needed by <span style="color:purple">offline Japanese-German</span> word translation.<br/>
#It is a ja-de dictionary similar to <a href="%s">EDICT</a> but distributed in <a href="%s">EPWING</a> format.<br/>
#Wadoku is <span style="color:purple">free</span>, and you can download here from its official website:
#<center><a href="%s">%s</a></center>""") % (edict_url, epwing_url, url, url)
#+ "<br/>" + my.tr('<span style="color:red">Note: EPWING path cannot contain non-English characters!</span>')
#)

  ## Zhongri ##

  @memoizedproperty
  def zhongriGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.zhongriLocationEdit)
    editRow.addWidget(self.zhongriLocationButton)
    editRow.addWidget(self.zhongriLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.zhongriInfoEdit)
    ret = QtWidgets.QGroupBox(notr_("Shogakukan Zhongri") + u" (小学館・日中統合辞典, 30MB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def zhongriLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def zhongriLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format(mytr_("Zhongri")))
    ret.clicked.connect(self._getZhongriLocation)
    return ret

  @memoizedproperty
  def zhongriLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearZhongriLocation)
    return ret

  @memoizedproperty
  def zhongriInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _clearZhongriLocation(self):
    path = ''
    ebdict.zhongri().setLocation(path)
    settings.global_().setZhongriLocation(path)
    self._refreshZhongri()

  def _getZhongriLocation(self):
    path = ebdict.zhongri().location() or skpaths.HOME
    path = QtWidgets.QFileDialog.getExistingDirectory(self.q,
        my.tr("Please select the folder containing {0}").format('"CATALOG(S)"'),
        path, 0)
    if path:
      if not ebdict.ZhongriDic.verifyLocation(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format(mytr_("Zhongri")))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        ebdict.zhongri().setLocation(path)
        if ebdict.zhongri().exists():
          settings.global_().setZhongriLocation(path)
        self._refreshZhongri()

  def _refreshZhongri(self):
    ok = ebdict.zhongri().exists()
    if ok:
      path = ebdict.zhongri().location()
      if path:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
      ok = bool(path) and os.path.exists(path)
    self.zhongriLocationClearButton.setVisible(ok)
    #self.zhongriLocationButton.setVisible(not ok)
    skqss.class_(self.zhongriLocationEdit, 'normal' if ok else 'muted')
    self.zhongriLocationEdit.setText(path if ok else my.tr("Not found, please specify the location of {0}").format(mytr_("Zhongri")))

    epwing_url = "http://ja.wikipedia.org/wiki/EPWING"
    url = ebdict.ZhongriDic.URL
    self.zhongriInfoEdit.setHtml(my.tr(
"""Shogakukan Zhongri is used by <span style="color:purple">offline Japanese-Chinese</span> word translation.<br/>
Zhongri is detected on your system at the above location.""")
    if ok else my.tr(
"""Shogakukan Zhongri is needed by <span style="color:purple">offline Japanese-Chinese</span> translation.<br/>
It is a ja-zh dictionary distributed in <a href="%s">EPWING</a> format DVD.<br/>
Zhongri is <span style="color:purple">not free</span>, and you can purchase one here from Amazon:
<center><a href="%s">%s</a></center>""") % (epwing_url, url, url)
+ "<br/>" + my.tr('<span style="color:red">Note: EPWING path cannot contain non-English characters!</span>')
)

  def refresh(self):
    self._refreshMsime()
    self._refreshDaijirin()
    self._refreshKojien()
    self._refreshZhongri()
    #self._refreshWadoku()

class DictionaryLibraryTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(DictionaryLibraryTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _DictionaryLibraryTab(self)
    #self.setChildrenCollapsible(False)
    #self.setMinimumWidth(LIBRARY_MINIMUM_WIDTH)

  def load(self): self.__d.refresh()
  def save(self): pass #self.__d.save()
  def refresh(self): pass

# TTS library tab

@Q_Q
class _TtsLibraryTab(object):
  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    # Google TTS
    #layout.addWidget(self.quickTimeGroup)
    layout.addWidget(self.wmpGroup)
    # Sapi
    layout.addWidget(self.showGroup)
    layout.addWidget(self.misakiGroup)
    # Voiceroid
    layout.addWidget(self.yukariGroup)
    layout.addWidget(self.zunkoGroup)
    layout.addStretch()
    q.setLayout(layout)

  ## Windows Media Player ##

  @memoizedproperty
  def wmpGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.wmpLocationEdit)
    layout.addWidget(self.wmpInfoEdit)
    #layout.addWidget(self.quickTimeEnableButton)
    ret = QtWidgets.QGroupBox(notr_("Windows Media Player (7MB)"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def wmpLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def wmpInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _refreshWmp(self):
    libman.wmp().refresh()
    ok = libman.wmp().exists()
    if ok:
      path = libman.wmp().location()
      path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
    skqss.class_(self.wmpLocationEdit, 'normal' if ok else 'muted')
    self.wmpLocationEdit.setText(path if ok else tr_("Not found"))

    url = libman.WindowsMediaPlayer.URL
    self.wmpInfoEdit.setHtml(my.tr(
"""Windows Media Player is needed by <span style="color:purple">Online TTS</span>.<br/>
Genuine Windows already have it installed by default, unless you have explicitly removed it by yourself.
It is detected on your system at the above location.""")
    if ok else my.tr(
"""Windows Media Player is needed by <span style="color:purple">Online TTS</span>.<br/>
Genuine Windows already have it installed by default, unless you have explicitly removed it by yourself.
You can reinistall Windows Media Player here from Microsoft for free:
<center><a href="{0}">{0}</a></center>""").format(url))

#  ## QuickTime ##
#
#  @memoizedproperty
#  def quickTimeGroup(self):
#    layout = QtWidgets.QVBoxLayout()
#    layout.addWidget(self.quickTimeLocationEdit)
#    layout.addWidget(self.quickTimeInfoEdit)
#    #layout.addWidget(self.quickTimeEnableButton)
#    ret = QtWidgets.QGroupBox(notr_("Apple QuickTime (74MB)"))
#    ret.setLayout(layout)
#    return ret
#
#  @memoizedproperty
#  def quickTimeLocationEdit(self):
#    ret = QtWidgets.QLineEdit()
#    ret.setReadOnly(True)
#    ret.setToolTip(tr_("Location"))
#    return ret
#
#  @memoizedproperty
#  def quickTimeInfoEdit(self):
#    ret = QtWidgets.QTextBrowser()
#    skqss.class_(ret, 'texture')
#    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
#    ret.setAlignment(Qt.AlignCenter)
#    ret.setReadOnly(True)
#    ret.setOpenExternalLinks(True)
#    return ret
#
#  def _refreshQuickTime(self):
#    libman.quicktime().refresh()
#    ok = libman.quicktime().exists()
#    if ok:
#      path = libman.quicktime().location()
#      path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
#    skqss.class_(self.quickTimeLocationEdit, 'normal' if ok else 'muted')
#    self.quickTimeLocationEdit.setText(path if ok else tr_("Not found"))
#
#    url = libman.QuickTime.URL
#    self.quickTimeInfoEdit.setHtml(my.tr(
#"""Apple QuickTime is needed by <span style="color:purple">Google TTS</span>.<br/>
#QuickTime is detected on your system at the above location.""")
#    if ok else my.tr(
#"""Apple QuickTime is needed by <span style="color:purple">Google TTS</span>.<br/>
#You can get a free version of QuickTime here from Apple:
#<center><a href="%s">%s</a></center>""") % (url, url))
#
#    #enabled = settings.global_().isQuickTimeEnabled()
#    #self.quickTimeEnableButton.setChecked(enabled)
#    #self.quickTimeLocationEdit.setEnabled(enabled)
#    #self.quickTimeInfoEdit.setEnabled(enabled)
#
#    #self._quickTimeEnableButton.setToolTip(
#    #  self.q.tr("QuickTime and TTS are enabled") if enabled else
#    #  self.q.tr("QuickTime and TTS are disabled")
#    #)
#    #self._quickTimeEnableButton.setVisible(not enabled or not ok)
#
#    #skqss.class_(self.quickTimeGroup, 'default' if ok else 'muted')
#
#    #self.quickTimeGroup.setStyleSheet("" if ok else
#    #  "QGroupBox { color:red }"
#    #)
#
#  #def _saveQuickTime(self):
#  #  settings.global_().setQuickTimeEnabled(
#  #      self.quickTimeEnableButton.isChecked())

  ## Misaki ##

  @memoizedproperty
  def misakiGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.misakiLocationEdit)
    layout.addWidget(self.misakiInfoEdit)
    ret = QtWidgets.QGroupBox(u"VoiceText Misaki (♀, SAPI, 755MB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def misakiLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def misakiInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _refreshMisaki(self):
    path = sapiman.location(sapiman.VW_MISAKI)
    ok = bool(path) and os.path.exists(path)
    if ok:
      path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
    skqss.class_(self.misakiLocationEdit, 'normal' if ok else 'muted')
    self.misakiLocationEdit.setText(path if ok else tr_("Not found"))

    url = "http://voicetext.jp"
    self.misakiInfoEdit.setHtml(my.tr(
"""VoiceText Misaki is needed by <span style="color:purple">offline TTS</span>.<br/>
Misaki is a Japanese female TTS SAPI library.<br/>
VoiceText is detected on your system at the above location.""")
    if ok else my.tr(
"""VoiceText Misaki is needed by <span style="color:purple">offline TTS</span>.<br/>
Misaki is a Japanese female TTS SAPI library.<br/>
Misaki is <span style="color:purple">not free</span>, and you can purchase one here from HOYA SERVICE:
<center><a href="%s">%s</a></center>""") % (url, url))

  ## Show ##

  @memoizedproperty
  def showGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.showLocationEdit)
    layout.addWidget(self.showInfoEdit)
    ret = QtWidgets.QGroupBox(u"VoiceText Show (♂, SAPI, 724MB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def showLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def showInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _refreshShow(self):
    path = sapiman.location(sapiman.VW_SHOW)
    ok = bool(path) and os.path.exists(path)
    if ok:
      path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
    skqss.class_(self.showLocationEdit, 'normal' if ok else 'muted')
    self.showLocationEdit.setText(path if ok else tr_("Not found"))

    url = "http://www.hoyasv.com/"
    self.showInfoEdit.setHtml(my.tr(
"""VoiceText Show is needed by <span style="color:purple">offline TTS</span>.<br/>
Show is a Japanese <span style="color:red">male</span> TTS SAPI library.<br/>
VoiceText is detected on your system at the above location.""")
    if ok else my.tr(
"""VoiceText Show is needed by <span style="color:purple">offline TTS</span>.<br/>
Show is a Japanese <span style="color:red">male</span> TTS SAPI library.<br/>
Show is <span style="color:purple">not free</span>, and you can purchase one here from HOYA SERVICE:
<center><a href="%s">%s</a></center>""") % (url, url))

  ## Yukari ##

  @memoizedproperty
  def yukariGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.yukariLocationEdit)
    editRow.addWidget(self.yukariLocationButton)
    editRow.addWidget(self.yukariLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.yukariInfoEdit)
    ret = QtWidgets.QGroupBox(u"VOICEROID+ 結月ゆかり (♀, 336MB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def yukariLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def yukariLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format('VOICEROID.exe'))
    ret.clicked.connect(self._getYukariLocation)
    return ret

  @memoizedproperty
  def yukariLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearYukariLocation)
    return ret

  @memoizedproperty
  def yukariInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _clearYukariLocation(self):
    settings.global_().setYukariLocation('')
    self._refreshYukari()

  def _getYukariLocation(self):
    FILTERS = "%s (%s)" % (tr_("Executable"), "VOICEROID.exe")
    path = settings.global_().yukariLocation() or ttsman.manager().yukariLocation()
    path = path if path and os.path.exists(path) else skpaths.HOME
    path, filter = QtWidgets.QFileDialog.getOpenFileName(self.q,
        my.tr("Please select the location of {0}").format("VOICEROID.exe"), path, FILTERS)
    if path:
      if not os.path.exists(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format('VOICEROID.exe'))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        settings.global_().setYukariLocation(path)
        self._refreshYukari()

  def _refreshYukari(self):
    path = settings.global_().yukariLocation() or ttsman.manager().yukariLocation()
    if path:
      path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
    ok = bool(path) and os.path.exists(path)
    self.yukariLocationClearButton.setVisible(ok)
    #self.yukariLocationButton.setVisible(not ok)
    skqss.class_(self.yukariLocationEdit, 'normal' if ok else 'muted')
    self.yukariLocationEdit.setText(path if path else my.tr("Not found, please specify the location of {0}").format('VOICEROID.exe'))

    url = "http://www.ah-soft.com/voiceroid/yukari"
    self.yukariInfoEdit.setHtml(my.tr(
"""Voiceroid+ Yukari from AHS is used by <span style="color:purple">offline TTS</span>.<br/>
Yukari is a Japanese female TTS app.<br/>
Voiceroid is detected on your system at the above location.""")
    if ok else my.tr(
"""Voiceroid+ Yukari could be used for <span style="color:purple">offline TTS</span>.<br/>
Yukari is a Japanese female TTS app.<br/>
Yukari is <span style="color:purple">not free</span>, and you can purchase one here from AHS:
<center><a href="%s">%s</a></center>""") % (url, url))

  ## Zunko ##

  @memoizedproperty
  def zunkoGroup(self):
    layout = QtWidgets.QVBoxLayout()
    editRow = QtWidgets.QHBoxLayout()
    editRow.addWidget(self.zunkoLocationEdit)
    editRow.addWidget(self.zunkoLocationButton)
    editRow.addWidget(self.zunkoLocationClearButton)
    layout.addLayout(editRow)
    layout.addWidget(self.zunkoInfoEdit)
    ret = QtWidgets.QGroupBox(u"VOICEROID+ 東北ずん子 (♀, 332MB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def zunkoLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def zunkoLocationButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, BROWSE_BTN_CLASS)
    ret.setToolTip(my.tr("Select the location of {0}").format('VOICEROID.exe'))
    ret.clicked.connect(self._getZunkoLocation)
    return ret

  @memoizedproperty
  def zunkoLocationClearButton(self):
    ret = QtWidgets.QPushButton(tr_("Clear"))
    skqss.class_(ret, CLEAR_BTN_CLASS)
    ret.setToolTip(my.tr("Clear the specified location"))
    ret.clicked.connect(self._clearZunkoLocation)
    return ret

  @memoizedproperty
  def zunkoInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _clearZunkoLocation(self):
    settings.global_().setZunkoLocation('')
    self._refreshZunko()

  def _getZunkoLocation(self):
    FILTERS = "%s (%s)" % (tr_("Executable"), "VOICEROID.exe")
    path = settings.global_().zunkoLocation() or ttsman.manager().zunkoLocation()
    path = path if path and os.path.exists(path) else skpaths.HOME
    path, filter = QtWidgets.QFileDialog.getOpenFileName(self.q,
        my.tr("Please select the location of {0}").format("VOICEROID.exe"), path, FILTERS)
    if path:
      if not os.path.exists(path):
        growl.error(my.tr("Couldn't find {0} from the specified location").format('VOICEROID.exe'))
      else:
        path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
        settings.global_().setZunkoLocation(path)
        self._refreshZunko()

  def _refreshZunko(self):
    path = settings.global_().zunkoLocation() or ttsman.manager().zunkoLocation()
    if path:
      path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
    ok = bool(path) and os.path.exists(path)
    self.zunkoLocationClearButton.setVisible(ok)
    #self.zunkoLocationButton.setVisible(not ok)
    skqss.class_(self.zunkoLocationEdit, 'normal' if ok else 'muted')
    self.zunkoLocationEdit.setText(path if path else my.tr("Not found, please specify the location of {0}").format('VOICEROID.exe'))

    url = "http://www.ah-soft.com/voiceroid/zunko"
    self.zunkoInfoEdit.setHtml(my.tr(
"""Voiceroid+ Zunko from AHS is used by <span style="color:purple">offline text-to-speech</span>.<br/>
Yukari is a Japanese female TTS app.<br/>
Voiceroid is detected on your system at the above location.""")
    if ok else my.tr(
"""Voiceroid+ Zunko could be used for <span style="color:purple">offline text-to-speech</span>.<br/>
Zunko is a Japanese female TTS app.<br/>
Zunko is <span style="color:purple">not free</span>, and you can purchase one here from AHS:
<center><a href="%s">%s</a></center>""") % (url, url))

  def refresh(self):
    self._refreshWmp()
    #self._refreshQuickTime()
    self._refreshYukari()
    self._refreshZunko()
    self._refreshMisaki()
    self._refreshShow()

class TtsLibraryTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(TtsLibraryTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _TtsLibraryTab(self)
    #self.setChildrenCollapsible(False)
    #self.setMinimumWidth(LIBRARY_MINIMUM_WIDTH)

  def load(self): self.__d.refresh()
  def save(self): pass #self.__d.save()
  def refresh(self): pass

# OCR library tab

@Q_Q
class _OcrLibraryTab(object):
  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()
    # Google TTS
    layout.addWidget(self.modiOcrGroup)
    layout.addStretch()
    q.setLayout(layout)

  ## MODI OCR ##

  @memoizedproperty
  def modiOcrGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.modiOcrLocationEdit)
    layout.addWidget(self.modiOcrInfoEdit)
    #layout.addWidget(self.modiOcrEnableButton)

    layout.addStretch()
    ret = QtWidgets.QGroupBox("MODI OCR (61MB)")
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def modiOcrLocationEdit(self):
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setToolTip(tr_("Location"))
    return ret

  @memoizedproperty
  def modiOcrInfoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    ret.setMaximumHeight(LIBRARY_TEXTEDIT_MAXIMUM_HEIGHT)
    ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    return ret

  def _refreshModiOcr(self):
    libman.modiocr().refresh()
    ok = libman.modiocr().exists()
    if ok:
      path = libman.modiocr().location()
      path = QtCore.QDir.toNativeSeparators(path).rstrip(os.path.sep)
    skqss.class_(self.modiOcrLocationEdit, 'normal' if ok else 'muted')
    self.modiOcrLocationEdit.setText(path if ok else tr_("Not found"))

    #ms_url = libman.ModiOcr.URL
    url = libman.ModiOcr.DOWNLOAD_URL
    self.modiOcrInfoEdit.setHtml(my.tr(
"""Microsoft MODI OCR from Office 2007 is needed by VNR's <span style="color:purple">optical character recognition</span>.
You can either purchase Microsoft Office 2007 and enable Japanese MODI OCR, or download MODI from the following page:
<center><a href="{0}">{0}</a></center>""").format(url))

  def refresh(self):
    self._refreshModiOcr()

class OcrLibraryTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(OcrLibraryTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _OcrLibraryTab(self)
    #self.setChildrenCollapsible(False)
    #self.setMinimumWidth(LIBRARY_MINIMUM_WIDTH)

  def load(self): self.__d.refresh()
  def save(self): pass #self.__d.save()
  def refresh(self): pass

# Embedded subtitle

#@Q_Q
class _EngineTab(object):
  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    ss = settings.global_()
    blans = ss.blockedLanguages()

    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.agentGroup)
    layout.addWidget(self.textGroup)
    layout.addWidget(self.optionGroup)

    if 'zh' not in blans:
      layout.addWidget(self.chineseGroup)

    layout.addWidget(self.infoEdit)
    layout.addStretch()
    q.setLayout(layout)

    b = self.agentEnableButton
    l = [self.textGroup, self.optionGroup]
    if 'zh' not in blans:
      l.append(self.chineseGroup)
    for w in l:
      w.setEnabled(b.isChecked())
      b.toggled.connect(w.setEnabled)

    l = [self.translationWaitTimeButton]
    if 'zh' not in blans:
      l.append(self.chineseGroup)
    for w in l:
      slot = partial(lambda w, value: w.setEnabled(ss.isGameAgentLauncherNeeded()), w)
      slot(None)
      ss.gameAgentEnabledChanged.connect(slot)
      for key in 'Scenario', 'Name', 'Other':
        sig = getattr(ss, 'embedded{0}TranslationEnabledChanged'.format(key))
        sig.connect(slot)

  @memoizedproperty
  def infoEdit(self):
    ret = QtWidgets.QTextBrowser()
    skqss.class_(ret, 'texture')
    #ret.setAlignment(Qt.AlignCenter)
    ret.setReadOnly(True)
    ret.setOpenExternalLinks(True)
    #ret.setMaximumHeight(145)
    ret.setHtml(info.renderGameAgentHelp())
    return ret

  ## Agent ##

  @memoizedproperty
  def agentGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.agentEnableButton)
    layout.addWidget(self.agentInfoLabel)
    ret = QtWidgets.QGroupBox(my.tr("Preferred game text extraction method"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def agentEnableButton(self):
    ss = settings.global_()
    ret = QtWidgets.QCheckBox(my.tr(
      "Use VNR's built-in hook instead of ITH if possible"
    ))
    ret.setChecked(ss.isGameAgentEnabled())
    ret.toggled.connect(ss.setGameAgentEnabled)
    return ret

  @memoizedproperty
  def agentInfoLabel(self):
    ret = QtWidgets.QLabel('\n'.join((
      my.tr("Changing the text extraction method requires restarting the game."),
      my.tr("This feature is currently under development, and only supports a small portion of the games that ITH supports."),
      my.tr("The current supported game engines are: {0}").format(', '.join(config.EMBEDDED_GAME_ENGINES)),
    )))
    skqss.class_(ret, 'text-error')
    ret.setWordWrap(True)
    #ret.setOpenExternalLinks(True)

    #import main
    #m = main.manager()
    #ret.linkActivated.connect(partial(m.openWiki, 'VNR/Game Settings'))
    return ret

  ## Chinese ##

  @memoizedproperty
  def chineseGroup(self):
    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.chineseEnableButton)

    info =  QtWidgets.QLabel(my.tr(
      "Convert Simplified Chinese to Traditional Chinese or Japanese kanji in the embedded translation. Otherwise, Chinese characters might be shown as question marks."
    ))
    info.setWordWrap(True)
    layout.addWidget(info)

    ret = QtWidgets.QGroupBox(my.tr("Preferred Chinese characters"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def chineseEnableButton(self):
    ss = settings.global_()
    ret = QtWidgets.QCheckBox(my.tr(
      "Use Traditional Chinese or Japanese kanji"
    ))
    ret.setChecked(ss.gameAgentConvertsKanji())
    ret.toggled.connect(ss.setGameAgentConvertsKanji)
    return ret

  ## Launcher ##

  #@memoizedproperty
  #def launchGroup(self):
  #  layout = QtWidgets.QVBoxLayout()
  #  layout.addWidget(self.launchEnableButton)
  #  layout.addWidget(self.launchInfoLabel)
  #  ret = QtWidgets.QGroupBox(my.tr("Preferred game launch method"))
  #  ret.setLayout(layout)
  #  return ret

  #@memoizedproperty
  #def launchEnableButton(self):
  #  ss = settings.global_()
  #  ret = QtWidgets.QCheckBox(my.tr(
  #    "Use VNR's built-in game launcher instead of others if have to"
  #  ))
  #  ret.setChecked(ss.isGameAgentLauncherEnabled())
  #  ret.toggled.connect(ss.setGameAgentLauncherEnabled)
  #  return ret

  #@memoizedproperty
  #def launchInfoLabel(self):
  #  ret = QtWidgets.QLabel(' '.join((
  #    my.tr("This is indispensable for SHIFT-JIS games when your language is NOT Latin-based."),
  #    my.tr("It is only needed when embedding translation is enabled."),
  #    my.tr("The current implementation is buggy. It is only guaranteed to work well on Japanese Windows."),
  #  )))
  #  ret.setWordWrap(True)
  #  return ret

  ## Options ##

  @memoizedproperty
  def optionGroup(self):
    layout = QtWidgets.QVBoxLayout()

    # CTRL detection
    layout.addWidget(self.ctrlButton)

    # Translation wait time
    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.translationWaitTimeButton)
    row.addWidget(QtWidgets.QLabel("<= %s" % my.tr("Translation wait time")))
    row.addStretch()
    layout.addLayout(row)

    layout.addWidget(self.optionInfoLabel)
    ret = QtWidgets.QGroupBox(my.tr("Embedding options"))
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def translationWaitTimeButton(self):
    #ret = QtWidgets.QSpinBox()
    #ret.setToolTip("%s: %s msec" % (tr_("Default"), 1000))
    #ret.setRange(100, 10000)
    #ret.setSingleStep(10)
    #ss = settings.global_()
    #ret.setValue(ss.embeddedTranslationWaitTime())
    #ret.valueChanged[int].connect(ss.setEmbeddedTranslationWaitTime)

    ret = QtWidgets.QDoubleSpinBox()
    ret.setToolTip("%s: %s sec." % (tr_("Default"), 2))
    ret.setRange(0.1, 10.0)
    ret.setSingleStep(0.1)
    ret.setDecimals(1) # 0.1
    ss = settings.global_()
    ret.setValue(ss.embeddedTranslationWaitTime() / 1000.0)
    ret.valueChanged[float].connect(lambda v: ss.setEmbeddedTranslationWaitTime(int(v * 1000)))
    return ret

  @memoizedproperty
  def optionInfoLabel(self):
    ret = QtWidgets.QLabel("%s: %s" % (
      tr_("Note"),
      my.tr("A large wait time might also slow down the game when your machine translator is slow."),
    ))
    ret.setWordWrap(True)
    return ret

  @memoizedproperty
  def ctrlButton(self):
    ret = QtWidgets.QCheckBox(my.tr("Disable text extraction when Ctrl is pressed"))
    ss = settings.global_()
    ret.setChecked(ss.isEmbeddedTextCancellableByControl())
    ret.toggled.connect(ss.setEmbeddedTextCancellableByControl)
    return ret

  ## Text ##

  @memoizedproperty
  def textGroup(self):
    ret = QtWidgets.QGroupBox(my.tr("Preferred text to embed in the game"))
    cols = QtWidgets.QVBoxLayout()

    for key,label in (
        ('scenario', mytr_("Dialog")),
        ('name', tr_("Name")),
        ('other', tr_("Other")),
        ('window', tr_("Window")),
      ):
      row = QtWidgets.QHBoxLayout()
      row.addWidget(QtWidgets.QLabel(label + ":"))
      group = getattr(self, key + 'TextGroup')
      group.setParent(ret)
      l = group.buttons()
      map(row.addWidget, l)
      row.addStretch()
      cols.addLayout(row)

    #cols.addWidget(self.textInfoEdit)

    ret.setLayout(cols)
    return ret

  # Scenario

  @memoizedproperty
  def scenarioTextGroup(self):
    ret = QtWidgets.QButtonGroup()

    self.scenarioDisableButton = QtWidgets.QRadioButton(tr_("Disable"))
    ret.addButton(self.scenarioDisableButton)

    self.scenarioTranscodeButton = QtWidgets.QRadioButton(tr_("Transcode"))
    ret.addButton(self.scenarioTranscodeButton)

    self.scenarioTranslateButton = QtWidgets.QRadioButton(tr_("Translate"))
    ret.addButton(self.scenarioTranslateButton)

    self.scenarioHideButton = QtWidgets.QRadioButton(tr_("Hide"))
    ret.addButton(self.scenarioHideButton)

    self._loadScenarioTextGroup()
    ret.buttonClicked.connect(self._saveScenarioTextGroup)
    return ret

  def _loadScenarioTextGroup(self):
    ss = settings.global_()
    if not ss.isEmbeddedScenarioVisible():
      self.scenarioHideButton.setChecked(True)
    elif ss.isEmbeddedScenarioTranslationEnabled():
      self.scenarioTranslateButton.setChecked(True)
    elif ss.isEmbeddedScenarioTranscodingEnabled():
      self.scenarioTranscodeButton.setChecked(True)
    else:
      self.scenarioDisableButton.setChecked(True)

  def _saveScenarioTextGroup(self):
    ss = settings.global_()
    if self.scenarioTranslateButton.isChecked():
      ss.setEmbeddedScenarioVisible(True)
      ss.setEmbeddedScenarioTranscodingEnabled(True)
      ss.setEmbeddedScenarioTranslationEnabled(True)
    elif self.scenarioTranscodeButton.isChecked():
      ss.setEmbeddedScenarioVisible(True)
      ss.setEmbeddedScenarioTranscodingEnabled(True)
      ss.setEmbeddedScenarioTranslationEnabled(False)
    elif self.scenarioDisableButton.isChecked():
      ss.setEmbeddedScenarioVisible(True)
      ss.setEmbeddedScenarioTranscodingEnabled(False)
      ss.setEmbeddedScenarioTranslationEnabled(False)
    elif self.scenarioHideButton.isChecked():
      ss.setEmbeddedScenarioVisible(False)
      ss.setEmbeddedScenarioTranscodingEnabled(False)
      ss.setEmbeddedScenarioTranslationEnabled(False)

  # Name

  @memoizedproperty
  def nameTextGroup(self):
    ret = QtWidgets.QButtonGroup()

    self.nameDisableButton = QtWidgets.QRadioButton(tr_("Disable"))
    ret.addButton(self.nameDisableButton)

    self.nameTranscodeButton = QtWidgets.QRadioButton(tr_("Transcode"))
    ret.addButton(self.nameTranscodeButton)

    self.nameTranslateButton = QtWidgets.QRadioButton(tr_("Translate"))
    ret.addButton(self.nameTranslateButton)

    self.nameVisibleButton = QtWidgets.QRadioButton(tr_("Both"))
    ret.addButton(self.nameVisibleButton)

    self.nameHideButton = QtWidgets.QRadioButton(tr_("Hide"))
    ret.addButton(self.nameHideButton)

    self._loadNameTextGroup()
    ret.buttonClicked.connect(self._saveNameTextGroup)
    return ret

  def _loadNameTextGroup(self):
    ss = settings.global_()
    if not ss.isEmbeddedNameVisible():
      self.nameHideButton.setChecked(True)
    elif ss.isEmbeddedNameTextVisible():
      self.nameVisibleButton.setChecked(True)
    elif ss.isEmbeddedNameTranslationEnabled():
      self.nameTranslateButton.setChecked(True)
    elif ss.isEmbeddedNameTranscodingEnabled():
      self.nameTranscodeButton.setChecked(True)
    else:
      self.nameDisableButton.setChecked(True)

  def _saveNameTextGroup(self):
    ss = settings.global_()
    if self.nameVisibleButton.isChecked():
      ss.setEmbeddedNameVisible(True)
      ss.setEmbeddedNameTranscodingEnabled(True)
      ss.setEmbeddedNameTranslationEnabled(True)
      ss.setEmbeddedNameTextVisible(True)
    elif self.nameTranslateButton.isChecked():
      ss.setEmbeddedNameVisible(True)
      ss.setEmbeddedNameTranscodingEnabled(True)
      ss.setEmbeddedNameTranslationEnabled(True)
      ss.setEmbeddedNameTextVisible(False)
    elif self.nameTranscodeButton.isChecked():
      ss.setEmbeddedNameVisible(True)
      ss.setEmbeddedNameTranscodingEnabled(True)
      ss.setEmbeddedNameTranslationEnabled(False)
      ss.setEmbeddedNameTextVisible(False)
    elif self.nameDisableButton.isChecked():
      ss.setEmbeddedNameVisible(True)
      ss.setEmbeddedNameTranscodingEnabled(False)
      ss.setEmbeddedNameTranslationEnabled(False)
      ss.setEmbeddedNameTextVisible(False)
    elif self.nameHideButton.isChecked():
      ss.setEmbeddedNameVisible(False)
      ss.setEmbeddedNameTranscodingEnabled(False)
      ss.setEmbeddedNameTranslationEnabled(False)
      ss.setEmbeddedNameTextVisible(False)

  # Other

  @memoizedproperty
  def otherTextGroup(self):
    ret = QtWidgets.QButtonGroup()

    self.otherDisableButton = QtWidgets.QRadioButton(tr_("Disable"))
    ret.addButton(self.otherDisableButton)

    self.otherTranscodeButton = QtWidgets.QRadioButton(tr_("Transcode"))
    ret.addButton(self.otherTranscodeButton)

    self.otherTranslateButton = QtWidgets.QRadioButton(
        "%s (%s)" % (tr_("Translate"), tr_("slow")))
    ret.addButton(self.otherTranslateButton)

    #self.otherHideButton = QtWidgets.QRadioButton(tr_("Hide"))
    #ret.addButton(self.otherHideButton)

    self._loadOtherTextGroup()
    ret.buttonClicked.connect(self._saveOtherTextGroup)
    return ret

  def _loadOtherTextGroup(self):
    ss = settings.global_()
    #if not ss.isEmbeddedOtherVisible():
    #  self.otherHideButton.setChecked(True)
    if ss.isEmbeddedOtherTranslationEnabled():
      self.otherTranslateButton.setChecked(True)
    elif ss.isEmbeddedOtherTranscodingEnabled():
      self.otherTranscodeButton.setChecked(True)
    else:
      self.otherDisableButton.setChecked(True)

  def _saveOtherTextGroup(self):
    ss = settings.global_()
    if self.otherTranslateButton.isChecked():
      ss.setEmbeddedOtherVisible(True)
      ss.setEmbeddedOtherTranscodingEnabled(True)
      ss.setEmbeddedOtherTranslationEnabled(True)
    elif self.otherTranscodeButton.isChecked():
      ss.setEmbeddedOtherVisible(True)
      ss.setEmbeddedOtherTranscodingEnabled(True)
      ss.setEmbeddedOtherTranslationEnabled(False)
    elif self.otherDisableButton.isChecked():
      ss.setEmbeddedOtherVisible(True)
      ss.setEmbeddedOtherTranscodingEnabled(False)
      ss.setEmbeddedOtherTranslationEnabled(False)
    #elif self.otherHideButton.isChecked():
    #  ss.setEmbeddedOtherVisible(False)
    #  ss.setEmbeddedOtherTranscodingEnabled(False)
    #  ss.setEmbeddedOtherTranslationEnabled(False)

  # Window

  @memoizedproperty
  def windowTextGroup(self):
    ret = QtWidgets.QButtonGroup()

    self.windowDisableButton = QtWidgets.QRadioButton(tr_("Disable"))
    ret.addButton(self.windowDisableButton)

    self.windowTranscodeButton = QtWidgets.QRadioButton(tr_("Transcode"))
    ret.addButton(self.windowTranscodeButton)

    self.windowTranslateButton = QtWidgets.QRadioButton(
        "%s (%s)" % (tr_("Translate"), tr_("slow")))
    ret.addButton(self.windowTranslateButton)

    self.windowVisibleButton = QtWidgets.QRadioButton(
        "%s (%s)" % (tr_("Both"), tr_("slow")))
    ret.addButton(self.windowVisibleButton)

    #self.windowHideButton = QtWidgets.QRadioButton(tr_("Hide"))
    #ret.addButton(self.windowHideButton)

    self._loadWindowTextGroup()
    ret.buttonClicked.connect(self._saveWindowTextGroup)
    return ret

  def _loadWindowTextGroup(self):
    ss = settings.global_()
    #if not ss.isWindowVisible():
    #  self.windowHideButton.setChecked(True)
    if ss.isWindowTextVisible():
      self.windowVisibleButton.setChecked(True)
    elif ss.isWindowTranslationEnabled():
      self.windowTranslateButton.setChecked(True)
    elif ss.isWindowTranscodingEnabled():
      self.windowTranscodeButton.setChecked(True)
    else:
      self.windowDisableButton.setChecked(True)

  def _saveWindowTextGroup(self):
    ss = settings.global_()
    if self.windowVisibleButton.isChecked():
      #ss.setWindowVisible(True)
      ss.setWindowTranscodingEnabled(True)
      ss.setWindowTranslationEnabled(True)
      ss.setWindowTextVisible(True)
    elif self.windowTranslateButton.isChecked():
      #ss.setWindowVisible(True)
      ss.setWindowTranscodingEnabled(True)
      ss.setWindowTranslationEnabled(True)
      ss.setWindowTextVisible(False)
    elif self.windowTranscodeButton.isChecked():
      #ss.setWindowVisible(True)
      ss.setWindowTranscodingEnabled(True)
      ss.setWindowTranslationEnabled(False)
      ss.setWindowTextVisible(False)
    elif self.windowDisableButton.isChecked():
      #ss.setWindowVisible(True)
      ss.setWindowTranscodingEnabled(False)
      ss.setWindowTranslationEnabled(False)
      ss.setWindowTextVisible(False)
    #elif self.windowHideButton.isChecked():
    #  ss.setWindowVisible(False)
    #  ss.setWindowTranscodingEnabled(False)
    #  ss.setWindowTranslationEnabled(False)
    #  ss.setWindowTextVisible(False)

class EngineTab(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(EngineTab, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.__d = _EngineTab(self)
    #self.setChildrenCollapsible(False)
    #self.setMinimumWidth(LIBRARY_MINIMUM_WIDTH)

  def load(self): pass
  def save(self): pass
  def refresh(self): pass

# EOF

#    layout = QtWidgets.QVBoxLayout()
#    layout.addWidget(self.categoryGroup)
#    layout.addWidget(self.testGroup)
#    layout.addWidget(self.engineGroup)
#    layout.addWidget(self.genderGroup)
#
#    infoEdit = QtWidgets.QTextBrowser()
#    #infoEdit.setAlignment(Qt.AlignCenter)
#    infoEdit.setReadOnly(True)
#    infoEdit.setOpenExternalLinks(True)
#
#    hoya_url = "http://www.hoyasv.com/"
#    quicktime_url = libman.QuickTime.URL
#    #voicetext_url = "http://voicetext.jp/samplevoice.html"
#    #mstts_url = "http://mahoro-ba.net/e1542.html"
#    #mstts_setup_url = "http://mahoro-ba.net/e1542.html"
#    #infoEdit.setMaximumHeight(145)
#    infoEdit.setHtml(myss.render(my.tr(
#"""<h4>About online TTS</h4>
#Google TTS requires <a href="%s">Apple QuickTime</a> to be installed on your system.
#<span style="color:red">If you are living in Mainland China,
#don't forget to turn on the Mainland China option in Features.</span>
#
#<h4>About offline TTS</h4>
#You can install additional Japanese TTS soft to enable offline TTS,
#such as <span style="color:purple">VoiceText Misaki (female) and Show (male)</span>
#from <a href="%s">HOYA SERVICE</a>.""") % (quicktime_url, hoya_url)))
##<br/><br/>
##
##You can also get a free TTS from Microsoft online (<a href="%s">MS Haruka</a>).
##But it requires a bunch of runtime libraries to be installed first.
##Here's an one-click installer for MS Haruka that might be helpful:
##<center><a href="%s">%s</a></center>""")
##% (hoya_url, voicetext_url, voicetext_url, mstts_url, mstts_setup_url, mstts_setup_url))
#    layout.addWidget(infoEdit)
#    q.setLayout(layout)
#
#    self._loadCategory()
#    self._loadEngine()
#
#  ## Category ##
#
#  @memoizedproperty
#  def categoryGroup(self):
#    layout = QtWidgets.QVBoxLayout()
#    layout.addWidget(self.disableButton)
#    layout.addWidget(self.onlineButton)
#    layout.addWidget(self.offlineButton)
#    ret = QtWidgets.QGroupBox(my.tr("Speak clicked Japanese using text-to-speech") + " (TTS)")
#    ret.setLayout(layout)
#    return ret
#
#  @memoizedproperty
#  def disableButton(self):
#    ret = QtWidgets.QRadioButton(my.tr("Disable TTS"))
#    ret.toggled.connect(self._saveCategory)
#    ret.toggled.connect(self._loadEngine)
#    return ret
#
#  @memoizedproperty
#  def onlineButton(self):
#    ret = QtWidgets.QRadioButton("(%s) %s" % (
#        tr_("Online"), my.tr("Use Google's TTS service")))
#    ret.toggled.connect(self._saveCategory)
#    ret.toggled.connect(self._loadEngine)
#    return ret
#
#  @memoizedproperty
#  def offlineButton(self):
#    ret = QtWidgets.QRadioButton("(%s) %s" % (
#        tr_("Offline"), my.tr("Use TTS voices installed on Windows")))
#    if features.ADMIN == True:
#      skqss.class_(ret, 'warning')
#    ret.toggled.connect(self._saveCategory)
#    ret.toggled.connect(self._loadEngine)
#    return ret
#
#  def _loadCategory(self):
#    ss = settings.global_()
#    b = (self.onlineButton if ss.isGoogleTtsEnabled() else
#         self.offlineButton if ss.isWindowsTtsEnabled() else
#         self.disableButton)
#    if not b.isChecked():
#      b.setChecked(True)
#
#  def _saveCategory(self):
#    ss = settings.global_()
#    ss.setGoogleTtsEnabled(self.onlineButton.isChecked())
#    ss.setWindowsTtsEnabled(self.offlineButton.isChecked())
#
#  ## Test ##
#
#  @memoizedproperty
#  def testGroup(self):
#    layout = QtWidgets.QVBoxLayout()
#
#    row = QtWidgets.QHBoxLayout()
#    row.addWidget(self.testButton)
#    row.addWidget(self.testEdit)
#    layout.addLayout(row)
#
#    ret = QtWidgets.QGroupBox() #tr_("Test"))
#    self.disableButton.toggled.connect(lambda value:
#        ret.setEnabled(not value))
#    ret.setLayout(layout)
#    return ret
#
#  @memoizedproperty
#  def testEdit(self):
#    ret = QtWidgets.QLineEdit()
#    ret.setText(u"お花の匂い")
#    skqss.class_(ret, 'normal')
#    ret.textChanged.connect(lambda text:
#        self.testButton.setEnabled(bool(text.strip())))
#    return ret
#
#  @memoizedproperty
#  def testButton(self):
#    ret = QtWidgets.QPushButton()
#    ret.setText(tr_("Test"))
#    skqss.class_(ret, 'btn btn-success')
#    ret.clicked.connect(self._test)
#    return ret
#
#  def _test(self):
#    import ttsman
#    t = self.testEdit.text().strip()
#    if t:
#      ttsman.speak(t)
#
#  ## Gender ##
#
#  @memoizedproperty
#  def genderGroup(self):
#    grid = QtWidgets.QGridLayout()
#
#    grid.addWidget(QtWidgets.QLabel(tr_("Female") + " (ja):"), 0, 0)
#    grid.addWidget(self.femaleEditJa, 0, 1)
#
#    grid.addWidget(QtWidgets.QLabel(tr_("Male") + " (ja):"))
#    grid.addWidget(self.maleEditJa)
#
#    grid.addWidget(QtWidgets.QLabel(tr_("Female") + " (%s):" % tr_("other")))
#    grid.addWidget(self.femaleEdit)
#
#    grid.addWidget(QtWidgets.QLabel(tr_("Male") + " (%s):" % tr_("other")))
#    grid.addWidget(self.maleEdit)
#
#    ret = QtWidgets.QGroupBox(my.tr("Multilingual TTS voices"))
#    ret.setLayout(grid)
#
#    #self.onlineButton.toggled.connect(lambda value:
#    #    ret.setEnabled(value))
#    return ret
#
#  @memoizedproperty
#  def femaleEdit(self):
#    engine = settings.global_().femaleVoice()
#    ret = self._createWindowsEngineEdit(engine, ttsman.voices_nonja())
#    ret.currentIndexChanged.connect(self._saveFemaleEngine)
#    return ret
#
#  @memoizedproperty
#  def maleEdit(self):
#    engine = settings.global_().maleVoice()
#    ret = self._createWindowsEngineEdit(engine, ttsman.voices_nonja())
#    ret.currentIndexChanged.connect(self._saveMaleEngine)
#    return ret
#
#  def _saveFemaleEngine(self):
#    ss = settings.global_()
#    if ss.isWindowsTtsEnabled():
#      voices = ttsman.voices_nonja()
#      index = self.femaleEdit.currentIndex()
#      if index >= 0 and index < len(voices):
#        ss.setFemaleVoice(voices[index].key)
#
#  def _saveMaleEngine(self):
#    ss = settings.global_()
#    if ss.isWindowsTtsEnabled():
#      voices = ttsman.voices_nonja()
#      index = self.maleEdit.currentIndex()
#      if index >= 0 and index < len(voices):
#        ss.setMaleVoice(voices[index].key)
#
#  @memoizedproperty
#  def femaleEditJa(self):
#    engine = settings.global_().femaleVoiceJa()
#    ret = self._createWindowsEngineEdit(engine, ttsman.voices_ja())
#    ret.currentIndexChanged.connect(self._saveFemaleEngineJa)
#    return ret
#
#  @memoizedproperty
#  def maleEditJa(self):
#    engine = settings.global_().maleVoiceJa()
#    ret = self._createWindowsEngineEdit(engine, ttsman.voices_ja())
#    ret.currentIndexChanged.connect(self._saveMaleEngineJa)
#    return ret
#
#  def _saveFemaleEngineJa(self):
#    ss = settings.global_()
#    if ss.isWindowsTtsEnabled():
#      voices = ttsman.voices_ja()
#      index = self.femaleEditJa.currentIndex()
#      if index >= 0 and index < len(voices):
#        ss.setFemaleVoiceJa(voices[index].key)
#
#  def _saveMaleEngineJa(self):
#    ss = settings.global_()
#    if ss.isWindowsTtsEnabled():
#      voices = ttsman.voices_ja()
#      index = self.maleEditJa.currentIndex()
#      if index >= 0 and index < len(voices):
#        ss.setMaleVoiceJa(voices[index].key)
#
#  ## Engine ##
#
#  @memoizedproperty
#  def engineGroup(self):
#    layout = QtWidgets.QVBoxLayout()
#
#    row = QtWidgets.QHBoxLayout()
#    row.addWidget(QtWidgets.QLabel(mytr_("Voice") + ":"))
#    row.addWidget(self.engineEdit)
#    row.addStretch()
#    layout.addLayout(row)
#
#    row = QtWidgets.QHBoxLayout()
#    row.addWidget(QtWidgets.QLabel(tr_("Speed") + ":"))
#    row.addWidget(self.speedSlider)
#    row.addWidget(self.speedEdit)
#    row.addWidget(QtWidgets.QLabel("[-10, +10]"))
#    row.addStretch()
#    layout.addLayout(row)
#
#    infoLabel = QtWidgets.QLabel(my.tr(
#      "TTS might NOT work when VNR is launched as admin!"
#    ))
#    if features.ADMIN == True:
#      skqss.class_(infoLabel, 'warning')
#    layout.addWidget(infoLabel)
#
#    ret = QtWidgets.QGroupBox(my.tr("Preferred Japanese Windows TTS voice"))
#    ret.setLayout(layout)
#    return ret
#
#  @staticmethod
#  def _createWindowsEngineEdit(engine, voices):
#    """
#    @param  engine  unicode
#    @param  voices  [wintts.WinTtsVocie]
#    @return  QComboBox
#    """
#    ret = QtWidgets.QComboBox()
#    ret.setEditable(False)
#
#    ret.addItems([
#      "%s (%s, %s) < %s" %
#      (it.name, it.language(), it.gender.lower(), it.vendor)
#      for it in voices
#    ])
#
#    voiceNames = [it.name for it in voices]
#    if engine and engine in voiceNames:
#      ret.setCurrentIndex(voiceNames.index(engine))
#    return ret
#
#  @memoizedproperty
#  def engineEdit(self):
#    engine = settings.global_().windowsTtsEngine()
#    ret = self._createWindowsEngineEdit(engine, ttsman.voices_ja())
#    ret.currentIndexChanged.connect(self._saveEngine)
#    return ret
#
#  @memoizedproperty
#  def speedEdit(self):
#    ret = QtWidgets.QSpinBox()
#    ret.setRange(-10, 10)
#    ret.setSingleStep(1)
#    ss = settings.global_()
#    ret.setValue(ss.windowsTtsSpeed())
#    ret.valueChanged.connect(ss.setWindowsTtsSpeed)
#    ss.windowsTtsSpeedChanged.connect(ret.setValue)
#    return ret
#
#  @memoizedproperty
#  def speedSlider(self):
#    ret = QtWidgets.QSlider(Qt.Horizontal)
#    ret.setRange(-10, 10)
#    ret.setSingleStep(1)
#    ret.setTickInterval(1)
#    ret.setTickPosition(QtWidgets.QSlider.TicksBelow)
#    ss = settings.global_()
#    ret.setValue(ss.windowsTtsSpeed())
#    ret.valueChanged.connect(ss.setWindowsTtsSpeed)
#    ss.windowsTtsSpeedChanged.connect(ret.setValue)
#    return ret
#
#  def _loadEngine(self):
#    enabled = settings.global_().isWindowsTtsEnabled()
#    for w in (
#        self.engineEdit,
#        self.speedEdit, self.speedSlider,
#        self.femaleEdit, self.maleEdit, self.femaleEditJa, self.maleEditJa):
#      w.setEnabled(enabled)
#
#    ok = bool(ttsman.voices_ja())
#    self.engineGroup.setEnabled(ok)
#    skqss.class_(self.engineGroup, 'default' if ok else 'muted')
#
#
#    ok = bool(ttsman.voices())
#    self.genderGroup.setEnabled(ok)
#    skqss.class_(self.genderGroup, 'default' if ok else 'muted')
#
#    if enabled and ok:
#      ok = bool(ttsman.voices_ja())
#      self.maleEditJa.setEnabled(ok)
#      self.femaleEditJa.setEnabled(ok)
#
#      ok = bool(ttsman.voices_nonja())
#      self.maleEdit.setEnabled(ok)
#      self.femaleEdit.setEnabled(ok)
#
#  def _saveEngine(self):
#    ss = settings.global_()
#    if ss.isWindowsTtsEnabled():
#      voices = ttsman.voices_ja()
#      index = self.engineEdit.currentIndex()
#      if index >= 0 and index < len(voices):
#        ss.setWindowsTtsEngine(voices[index].key)
#
#  def save(self):
#    self._saveCategory()
#    self._saveEngine()
#    self._saveMaleEngine()
#    self._saveFemaleEngine()
#    self._saveMaleEngineJa()
#    self._saveFemaleEngineJa()
