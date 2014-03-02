# coding: utf8
# terminput.py
# 2/12/2014 jichi

__all__ = ['TermInput']

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

from functools import partial
from PySide.QtCore import Qt #, Signal
from Qt5 import QtWidgets
from sakurakit import skdatetime, skevents, skqss
from sakurakit.skclass import Q_Q, memoizedproperty
from sakurakit.sktr import tr_
from mytr import my, mytr_
import config, dataman, growl, i18n, rc

COMBOBOX_MAXWIDTH = 100

@Q_Q
class _TermInput(object):
  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()

    # Options
    grid = QtWidgets.QGridLayout()
    r = 0
    grid.addWidget(QtWidgets.QLabel(tr_("Language") + ":"), r, 0)
    grid.addWidget(self.languageEdit, r, 1)
    r += 1

    grid.addWidget(QtWidgets.QLabel(tr_("Target") + ":"), r, 0)
    grid.addWidget(self.typeEdit, r, 1)
    r += 1

    grid.addWidget(QtWidgets.QLabel(tr_("Options") + ":"), r, 0)
    grid.addWidget(self.specialButton, r, 1)
    r += 1

    grid.addWidget(self.regexButton, r, 1)
    r += 1

    grid.addWidget(QtWidgets.QLabel(tr_("Pattern") + ":"), r, 0)
    grid.addWidget(self.patternEdit, r, 1)
    r += 1

    grid.addWidget(QtWidgets.QLabel(tr_("Translation") + ":"), r, 0)
    grid.addWidget(self.textEdit, r, 1)
    r += 1

    grid.addWidget(QtWidgets.QLabel(tr_("Comment") + ":"), r, 0)
    grid.addWidget(self.commentEdit, r, 1)
    r += 1

    #grid.addWidget(QtWidgets.QLabel(tr_("Status") + ":"), r, 0)
    #grid.addWidget(self.statusLabel, r, 1, 2, 1)
    #r += 1

    layout.addLayout(grid)

    # Footer
    layout.addWidget(self.statusLabel)

    row = QtWidgets.QHBoxLayout()
    row.addStretch()
    row.addWidget(self.helpButton)
    row.addWidget(self.cancelButton)
    row.addWidget(self.saveButton)
    layout.addLayout(row)

    q.setLayout(layout)

  @memoizedproperty
  def helpButton(self):
    ret = QtWidgets.QPushButton(tr_("Help"))
    ret.setToolTip(tr_("Help"))
    skqss.class_(ret, 'btn btn-default')

    import main
    ret.clicked.connect(partial(main.manager().openWiki,
        "VNR/Shared Dictionary"))
    return ret

  @memoizedproperty
  def cancelButton(self):
    ret = QtWidgets.QPushButton(tr_("Cancel"))
    ret.setToolTip(tr_("Cancel"))
    skqss.class_(ret, 'btn btn-default')
    ret.setDefault(True)
    ret.clicked.connect(self.q.hide)
    return ret

  @memoizedproperty
  def saveButton(self):
    ret = QtWidgets.QPushButton(tr_("Submit"))
    ret.setToolTip(tr_("Submit"))
    skqss.class_(ret, 'btn btn-primary')
    ret.clicked.connect(self.save)
    return ret

  @memoizedproperty
  def typeEdit(self):
    ret = QtWidgets.QComboBox()
    ret.setEditable(False)
    ret.addItems(dataman.Term.TR_TYPES)
    ret.setMaxVisibleItems(ret.count())
    ret.setMaximumWidth(COMBOBOX_MAXWIDTH)
    return ret

  @memoizedproperty
  def languageEdit(self):
    ret = QtWidgets.QComboBox()
    ret.setEditable(False)
    ret.addItems(map(i18n.language_name2, config.LANGUAGES))
    ret.setMaxVisibleItems(ret.count())
    ret.setMaximumWidth(COMBOBOX_MAXWIDTH)

    index = config.LANGUAGES.index(dataman.manager().user().language)
    ret.setCurrentIndex(index)
    return ret

  @memoizedproperty
  def regexButton(self):
    return QtWidgets.QCheckBox(tr_("Regular expression"))

  @memoizedproperty
  def specialButton(self):
    ret = QtWidgets.QCheckBox(mytr_("Series-specific"))
    ret.setChecked(True) # enable series-specific by default
    ret.toggled.connect(self._refreshStatus)
    return ret

  @memoizedproperty
  def statusLabel(self):
    ret = QtWidgets.QLabel()
    ret.setToolTip(tr_("Status"))
    ret.setWordWrap(True)
    return ret

  @memoizedproperty
  def patternEdit(self):
    ret = QtWidgets.QLineEdit()
    skqss.class_(ret, 'normal')
    ret.setPlaceholderText(mytr_("Matched text"))
    ret.setToolTip(ret.placeholderText())
    ret.textChanged.connect(self.refresh)
    return ret

  @memoizedproperty
  def textEdit(self):
    ret = QtWidgets.QLineEdit()
    skqss.class_(ret, 'normal')
    ret.setPlaceholderText(mytr_("Replaced text"))
    ret.setToolTip(ret.placeholderText())
    ret.textChanged.connect(self._refreshStatus)
    return ret

  @memoizedproperty
  def commentEdit(self):
    ret = QtWidgets.QLineEdit()
    #skqss.class_(ret, 'normal')
    ret.setPlaceholderText(tr_("Comment"))
    ret.setToolTip(ret.placeholderText())
    #ret.textChanged.connect(self.refresh)
    return ret

  def _canSave(self):
    return bool(self.patternEdit.text().strip())

  def _getLanguage(self): # -> str
    return config.LANGUAGES[self.languageEdit.currentIndex()]

  def _setLanguage(self, v): # str
    index = config.LANGUAGES.index(v)
    self.languageEdit.setCurrentIndex(index)

  def _getType(self): # -> str
    return dataman.Term.TYPES[self.typeEdit.currentIndex()]

  def _setType(self, v): # str
    index = dataman.Term.TYPES.index(v)
    self.typeEdit.setCurrentIndex(index)

  def save(self):
    if self._canSave():
      dm = dataman.manager()
      user = dm.user()
      if not user.name:
        return
      gameId = dm.currentGameId()
      md5 = dm.currentGameMd5()
      #if not gameId and not md5:
      #  return
      lang = self._getLanguage()
      type = self._getType()
      pattern = self.patternEdit.text().strip()
      text = self.textEdit.text().strip()
      regex = self.regexButton.isChecked()
      special = self.specialButton.isChecked() and bool(gameId or md5)
      ret = dataman.Term(gameId=gameId, gameMd5=md5,
          userId=user.id,
          language=lang, type=type,
          special=special, regex=regex,
          timestamp=skdatetime.current_unixtime(),
          pattern=pattern, text=text)

      self.clear()
      self.q.hide()

      skevents.runlater(partial(dm.submitTerm, ret), 200)

      from sakurakit.skstr import escapehtml
      growl.msg('<br/>'.join((
        my.tr("Add new term"),
        "%s = %s" % (escapehtml(pattern), escapehtml(text) or "(%s)" % tr_('empty')),
      )))

  def clear(self):
    for it in self.patternEdit, self.textEdit: #, self.commentEdit:
      it.clear()

  def autofill(self):
    lang = self._getLanguage()
    type = 'escape' if config.is_kanji_language(lang) else 'source'
    self._setType(type)

  def refresh(self):
    self.saveButton.setEnabled(self._canSave())
    self._refreshStatus()

  def _refreshStatus(self):
    w = self.statusLabel
    pattern = self.patternEdit.text().strip()
    if not pattern:
      skqss.class_(w, 'text-info')
      w.setText("%s: %s" % (tr_("Note"), my.tr("Missing pattern")))
    elif len(pattern.strip()) < 3 and not self.specialButton.isChecked():
      skqss.class_(w, 'text-error')
      w.setText("%s: %s" % (tr_("Warning"), my.tr("The pattern is kind of short. You might want to turn on the series-specific option.")))
    elif not self.textEdit.text().strip():
      skqss.class_(w, 'text-error')
      w.setText("%s: %s" % (tr_("Warning"), my.tr("The translation is empty. VNR will delete the text matched with the pattern.")))
    else:
      skqss.class_(w, 'text-success')
      w.setText("%s: %s" % (tr_("Note"), my.tr("Everything looks OK")))

class TermInput(QtWidgets.QDialog):
  #termEntered = Signal(QtCore.QObject) # Term

  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog | Qt.WindowMinMaxButtonsHint
    super(TermInput, self).__init__(parent, WINDOW_FLAGS)
    skqss.class_(self, 'texture')
    self.setWindowTitle(my.tr("Create dictionary entry"))
    self.setWindowIcon(rc.icon('window-dict'))
    self.__d = _TermInput(self)
    self.__d.autofill()
    self.resize(300, 250)
    #self.statusBar() # show status bar

    import netman
    netman.manager().onlineChanged.connect(lambda t: t or self.hide())
    dataman.manager().loginChanged.connect(lambda t: t or self.hide())

  def setPattern(self, v): # unicode
    self.__d.patternEdit.setText(v)

  def setText(self, v): # unicode
    self.__d.textEdit.setText(v)

  #def autofill(self): self.__d.autofill()

  def setVisible(self, value):
    """@reimp @public"""
    if value and not self.isVisible():
      self.__d.refresh()
    super(TermInput, self).setVisible(value)

if __name__ == '__main__':
  a = debug.app()
  w = TermInput()
  w.show()
  a.exec_()

# EOF
