# coding: utf8
# mttest.py
# 11/20/2013 jichi
# Machine translation tester.

__all__ = 'MTTester',

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

from functools import partial
from PySide.QtCore import Qt
from Qt5 import QtWidgets
from sakurakit import skqss
from sakurakit.skclass import memoizedproperty
from sakurakit.skdebug import dprint
from sakurakit.sktr import tr_
from mytr import my, mytr_
import config, i18n, rc, settings, textutil, termman, trman, trtraits

# http://www.alanwood.net/unicode/arrows.html
_LEFTARROW = u'<span style="color:blue">←</span>'
_RIGHTARROW = u'<span style="color:blue">→</span>'
_UPARROW = u'<span style="color:blue">↑</span>'
_DOWNARROW = u'<span style="color:blue">↓</span>'

_EQ_LABEL = u'<span style="color:blue">＝</span>'
_NE_LABEL = u'<span style="color:red">≠</span>'

_EMPTY_TEXT = "(%s)" % tr_("Not changed")
_DISABLED_TEXT = "(%s)" % tr_("Disabled")
_LANGUAGE_STAR = '<span style="color:green">+</span>' # plus
_TERM_STAR = '<span style="color:red">*</span>' # star

_TEXTEDIT_MINWIDTH = 150
_TEXTEDIT_MINHEIGHT = 200

class _MTTester(object):

  def __init__(self, q):
    self._createUi(q)

    tm = trman.manager()

    tm.languagesReceived.connect(lambda fr, to: (
        self.setFromLanguageLabelText(fr),
        self.setToLanguageLabelText(to)))
    tm.normalizedTextReceived.connect(lambda t:
        self.normalizedTextEdit.setPlainText(t or _EMPTY_TEXT))
    tm.sourceTextReceived.connect(lambda t:
        self.sourceTextEdit.setPlainText(t or _EMPTY_TEXT))
    tm.escapedTextReceived.connect(lambda t:
        self.escapedTextEdit.setPlainText(t or _EMPTY_TEXT))
    tm.jointTranslationReceived.connect(lambda t:
        self.jointTranslationEdit.setPlainText(t or _EMPTY_TEXT))
    tm.escapedTranslationReceived.connect(lambda t:
        self.escapedTranslationEdit.setHtml(t or _EMPTY_TEXT))
    tm.targetTranslationReceived.connect(lambda t:
        self.targetTranslationEdit.setHtml(t or _EMPTY_TEXT))

    tm.splitTextsReceived.connect(lambda l:
        self.splitTextEdit.setPlainText('\n--------\n'.join(l) if l else _EMPTY_TEXT))
    tm.splitTranslationsReceived.connect(lambda l:
        self.splitTranslationEdit.setPlainText('\n--------\n'.join(l) if l else _EMPTY_TEXT))

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()

    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.translateButton)
    row.addWidget(self.pasteButton)
    row.addWidget(self.speakButton)
    row.addWidget(self.translatorEdit)
    row.addWidget(self.fromLanguageEdit)
    row.addWidget(QtWidgets.QLabel(_RIGHTARROW))
    row.addWidget(self.toLanguageEdit)

    row.addWidget(QtWidgets.QLabel("("))
    row.addWidget(self.translatorLabel)
    #row.addWidget(QtWidgets.QLabel(":"))
    row.addWidget(self.fromLanguageLabel)
    row.addWidget(QtWidgets.QLabel(_RIGHTARROW))
    row.addWidget(self.toLanguageLabel)
    row.addWidget(QtWidgets.QLabel(")"))
    #row.addWidget(QtWidgets.QLabel(my.tr("Current game") + ":"))
    #row.addWidget(QtWidgets.QLabel("#"))
    row.addWidget(self.gameLabel)
    row.addStretch()
    layout.addLayout(row)

    # First row
    grid = QtWidgets.QGridLayout()
    r = 0
    c = 0
    cell = QtWidgets.QVBoxLayout()
    cell.addWidget(self.textLabel)
    cell.addWidget(self.textEdit)
    grid.addLayout(cell, r, c)

    c += 1
    grid.addWidget(QtWidgets.QLabel(_RIGHTARROW), r, c)
    c += 1
    cell = QtWidgets.QVBoxLayout()
    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.originTextButton)
    row.addWidget(self.originTextLabel)
    row.addStretch()
    cell.addLayout(row)
    cell.addWidget(self.originTextEdit)
    grid.addLayout(cell, r, c)

    c += 1
    grid.addWidget(QtWidgets.QLabel(_RIGHTARROW), r, c)
    c += 1
    cell = QtWidgets.QVBoxLayout()
    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.normalizedTextButton)
    row.addWidget(self.normalizedTextLabel)
    row.addStretch()
    cell.addLayout(row)
    cell.addWidget(self.normalizedTextEdit)
    grid.addLayout(cell, r, c)

    c += 1
    grid.addWidget(QtWidgets.QLabel(_RIGHTARROW), r, c)
    c += 1
    cell = QtWidgets.QVBoxLayout()
    cell.addWidget(self.sourceTextLabel)
    cell.addWidget(self.sourceTextEdit)
    grid.addLayout(cell, r, c)

    c += 1
    grid.addWidget(QtWidgets.QLabel(_RIGHTARROW), r, c)
    c += 1
    cell = QtWidgets.QVBoxLayout()
    cell.addWidget(self.escapedTextLabel)
    cell.addWidget(self.escapedTextEdit)
    grid.addLayout(cell, r, c)

    c += 1
    grid.addWidget(QtWidgets.QLabel(_RIGHTARROW), r, c)
    c += 1
    cell = QtWidgets.QVBoxLayout()
    cell.addWidget(self.splitTextLabel)
    cell.addWidget(self.splitTextEdit)
    grid.addLayout(cell, r, c)

    # Second row

    r += 1
    c = 0
    label = QtWidgets.QLabel(_DOWNARROW)
    label.setAlignment(Qt.AlignCenter)
    grid.addWidget(label, r, c)

    c = 6 * 2 - 2 # totally six columnes
    label = QtWidgets.QLabel(_DOWNARROW)
    label.setAlignment(Qt.AlignCenter)
    grid.addWidget(label, r, c)

    # Third row
    r += 1
    c = 0
    cell = QtWidgets.QVBoxLayout()
    cell.addWidget(self.directTranslationLabel)
    cell.addWidget(self.directTranslationEdit)
    grid.addLayout(cell, r, c)

    c += 1
    grid.addWidget(self.equalLabel, r, c)

    c += 1
    cell = QtWidgets.QVBoxLayout()
    cell.addWidget(self.finalTranslationLabel)
    cell.addWidget(self.finalTranslationEdit)
    grid.addLayout(cell, r, c)

    c += 1
    grid.addWidget(QtWidgets.QLabel(_LEFTARROW), r, c)
    c += 1
    cell = QtWidgets.QVBoxLayout()
    cell.addWidget(self.targetTranslationLabel)
    cell.addWidget(self.targetTranslationEdit)
    grid.addLayout(cell, r, c)

    c += 1
    grid.addWidget(QtWidgets.QLabel(_LEFTARROW), r, c)
    c += 1
    cell = QtWidgets.QVBoxLayout()
    cell.addWidget(self.escapedTranslationLabel)
    cell.addWidget(self.escapedTranslationEdit)
    grid.addLayout(cell, r, c)

    c += 1
    grid.addWidget(QtWidgets.QLabel(_LEFTARROW), r, c)
    c += 1
    cell = QtWidgets.QVBoxLayout()
    cell.addWidget(self.jointTranslationLabel)
    cell.addWidget(self.jointTranslationEdit)
    grid.addLayout(cell, r, c)

    c += 1
    grid.addWidget(QtWidgets.QLabel(_LEFTARROW), r, c)
    c += 1
    cell = QtWidgets.QVBoxLayout()
    cell.addWidget(self.splitTranslationLabel)
    cell.addWidget(self.splitTranslationEdit)
    grid.addLayout(cell, r, c)

    layout.addLayout(grid)
    # Footer
    layout.addWidget(QtWidgets.QLabel("%s: %s" % (tr_("Note"), " ".join((
        my.tr("Procedures marked as {0} would utilize Shared Dictionary.").format(_TERM_STAR),
        my.tr("Procedures marked as {0} behave differently for different user languages.").format(_LANGUAGE_STAR))))))

    q.setLayout(layout)

  def _clearTranslations(self):
    for it in (
        #self.textEdit,
        self.directTranslationEdit,
        self.originTextEdit,
        self.normalizedTextEdit,
        self.sourceTextEdit,
        self.escapedTextEdit,
        self.splitTextEdit,
        self.splitTranslationEdit,
        self.jointTranslationEdit,
        self.escapedTranslationEdit,
        self.targetTranslationEdit,
        self.finalTranslationEdit,
      ):
      it.setPlainText(_EMPTY_TEXT)

  def _currentText(self):
    return self.textEdit.toPlainText().strip()
  def _currentFromLanguage(self):
    return config.LANGUAGES[self.fromLanguageEdit.currentIndex()]
  def _currentToLanguage(self):
    return config.LANGUAGES[self.toLanguageEdit.currentIndex()]
  def _currentTranslator(self):
    return trtraits.TRAITS[self.translatorEdit.currentIndex()]['key']

  def _isOriginTermsEnabled(self): return self.originTextButton.isChecked()
  def _isTranslationScriptEnabled(self): return self.normalizedTextButton.isChecked()

  def _speak(self):
    t = self._currentText()
    if t:
      lang = self._currentFromLanguage()
      import ttsman
      ttsman.speak(t, language=lang)

  def _paste(self):
    from sakurakit import skclip
    t = skclip.gettext().strip()
    if t:
      self.textEdit.setPlainText(t)

  def _translate(self):
    t = self._currentText()
    if t:
      dprint("enter")
      self._clearTranslations()
      self.translatorLabel.setText(self.translatorEdit.currentText())
      lang = self._currentFromLanguage()
      params = {
        'fr': lang,
        'engine': self._currentTranslator(),
      }
      raw = trman.manager().translateDirect(t, **params)
      if raw:
        self.directTranslationEdit.setPlainText(raw)

      if self._isOriginTermsEnabled():
        tt = textutil.normalize_punct(t)
        tt = termman.manager().applyOriginTerms(tt, lang)
        if tt != t:
          t = tt
          self.setOriginTextEditText(t or _EMPTY_TEXT)
        else:
          self.setOriginTextEditText(_EMPTY_TEXT)
      else:
        self.originTextEdit.setPlainText(_DISABLED_TEXT)
      if t:
        t = trman.manager().translate(t, emit=True,
            scriptEnabled=self._isTranslationScriptEnabled(),
            **params)
        if t:
          self.finalTranslationEdit.setHtml(t)
      dprint("leave")

  @memoizedproperty
  def equalLabel(self):
    ret = QtWidgets.QLabel(_EQ_LABEL)
    ret.setToolTip(my.tr("Equal"))
    return ret

  def _refreshEqualLabel(self):
    eq = self.directTranslationEdit.toPlainText() == self.finalTranslationEdit.toPlainText()
    self.equalLabel.setText(_EQ_LABEL if eq else _NE_LABEL)
    self.equalLabel.setToolTip(my.tr("Equal") if eq else my.tr("Not equal"))

  @memoizedproperty
  def gameLabel(self):
    import dataman
    dm = dataman.manager()
    ret = QtWidgets.QLabel()
    ret.setToolTip(my.tr("Current game for game-specific terms in the Shared Dictionary"))
    def _refresh():
      ret.setText('#' + (dm.currentGameName() or my.tr("Unknown game")))
      skqss.class_(ret, 'text-error' if dm.currentGame() else 'text-info')
    _refresh()
    dm.currentGameChanged.connect(_refresh)
    return ret

  @memoizedproperty
  def pasteButton(self):
    ret = QtWidgets.QPushButton(tr_("Paste"))
    skqss.class_(ret, 'btn btn-success')
    ret.setToolTip(tr_("Paste"))
    ret.clicked.connect(self._paste)
    return ret

  @memoizedproperty
  def speakButton(self):
    ret = QtWidgets.QPushButton(mytr_("Speak"))
    skqss.class_(ret, 'btn btn-default')
    ret.setToolTip(mytr_("Speak"))
    ret.clicked.connect(self._speak)
    return ret

  @memoizedproperty
  def translateButton(self):
    ret = QtWidgets.QPushButton(tr_("Translate"))
    skqss.class_(ret, 'btn btn-primary')
    ret.setToolTip(tr_("Translate"))
    ret.clicked.connect(self._translate)
    return ret

  @memoizedproperty
  def translatorEdit(self):
    ret = QtWidgets.QComboBox()
    ret.setEditable(False)
    ret.addItems([it['name'] for it in trtraits.TRAITS])
    ret.setMaxVisibleItems(ret.count())
    ret.currentIndexChanged.connect(self._onTranslatorChanged)
    return ret

  def _onTranslatorChanged(self):
    tah = self._currentTranslator() in ('atlas', 'lec', 'lecol')
    self.normalizedTextButton.setEnabled(tah)
    self.normalizedTextLabel.setEnabled(tah)
    self.normalizedTextEdit.setEnabled(tah)

  @memoizedproperty
  def fromLanguageEdit(self):
    ret = QtWidgets.QComboBox()
    ret.setToolTip(my.tr("Text language"))
    ret.setEditable(False)
    ret.addItems(map(i18n.language_name2, config.LANGUAGES))
    ret.setMaxVisibleItems(ret.count())
    return ret

  @memoizedproperty
  def toLanguageEdit(self):
    ret = QtWidgets.QLabel()
    ret.setToolTip(my.tr("User language"))
    ss = settings.global_()
    ret.setText(i18n.language_name2(ss.userLanguage()))
    ss.userLanguageChanged.connect(lambda lang:
        ret.setText(i18n.language_name2(lang)))
    return ret

  @memoizedproperty
  def translatorLabel(self):
    ret = QtWidgets.QLabel()
    ret.setText(self.translatorEdit.currentText())
    ret.setToolTip(my.tr("Current translator"))
    skqss.class_(ret, 'text-info')
    return ret

  @memoizedproperty
  def fromLanguageLabel(self):
    ret = QtWidgets.QLabel()
    ret.setText(self.fromLanguageEdit.currentText())
    ret.setToolTip(my.tr("Language adjusted for the translator"))
    skqss.class_(ret, 'text-info')
    return ret

  @memoizedproperty
  def toLanguageLabel(self):
    ret = QtWidgets.QLabel()
    ret.setText(self.toLanguageEdit.text() + _LANGUAGE_STAR)
    ret.setToolTip(my.tr("Language adjusted for the translator"))
    skqss.class_(ret, 'text-info')
    return ret

  def setFromLanguageLabelText(self, lang):
    """
    @param  lang  unicode
    """
    label = self.fromLanguageLabel
    edit = self.fromLanguageEdit
    t = i18n.language_name2(lang) #+ _LANGUAGE_STAR
    label.setText(t)
    skqss.class_(label, 'text-info' if t == edit.currentText() else 'text-error')

  def setToLanguageLabelText(self, lang):
    """
    @param  lang  unicode
    """
    label = self.toLanguageLabel
    edit = self.toLanguageEdit
    t = i18n.language_name2(lang)
    label.setText(t + _LANGUAGE_STAR)
    skqss.class_(label, 'text-info' if t == edit.text() else 'text-error')

  # Text edits

  @classmethod
  def _createTextLabel(cls, edit, text='', tip=''):
    """
    @param  edit  QTextEdit
    @param* text  unicode
    @param* tip  unicode
    @return  QLabel
    """
    text = text or edit.toolTip()
    tip = tip or edit.toolTip()
    ret = QtWidgets.QLabel()
    ret.setText(text)
    ret.setToolTip(tip)
    edit.textChanged.connect(partial(cls._refreshTextLabel, ret, edit, text))
    return ret

  @staticmethod
  def _refreshTextLabel(label, edit, text):
    """
    @param  label  QLabel
    @param  edit  QTextEdit
    @param* text  unicode
    """
    t = edit.toPlainText().strip()
    if not t or t in (_EMPTY_TEXT, _DISABLED_TEXT):
      labelText = text
      skqss.class_(edit, 'text-muted')
    else:
      labelText = '%s (<span style="color:purple">%s</span>)' % (text, len(t))
      #skqss.class_(edit, 'readonly')
      skqss.class_(edit, '')
    label.setText(labelText)

  @staticmethod
  def _createTextView(tip='', rich=False):
    """
    @param* tip  unicode
    @param* rich  enable richText
    @return  QTextEdit
    """
    if rich:
      ret = QtWidgets.QTextEdit(_EMPTY_TEXT)
    else:
      ret = QtWidgets.QPlainTextEdit(_EMPTY_TEXT)
    ret.setMinimumWidth(_TEXTEDIT_MINWIDTH)
    ret.setMinimumHeight(_TEXTEDIT_MINHEIGHT)
    if tip:
      ret.setToolTip(tip)
    #ret.setAcceptRichText(False)
    #ret.setReadOnly(True)
    skqss.class_(ret, 'text-muted')
    #ret.resize(300, 200)
    return ret

  @memoizedproperty
  def textLabel(self):
    text = my.tr("Original text")
    ret = QtWidgets.QLabel()
    ret.setText('<span style="color:blue">%s</span>' % text)
    ret.setToolTip(my.tr("Original text to translate"))
    self.textEdit.textChanged.connect(lambda:
        ret.setText(
          '<span style="color:blue">%s</span> (<span style="color:purple">%s</span>)' % (
          text,
          len(self.textEdit.toPlainText().strip()),
        )))
    return ret
  @memoizedproperty
  def textEdit(self):
    ret = QtWidgets.QPlainTextEdit()
    ret.setMinimumWidth(_TEXTEDIT_MINWIDTH)
    ret.setMinimumHeight(_TEXTEDIT_MINHEIGHT)
    ret.setToolTip(my.tr("Original text to translate"))
    #ret.setAcceptRichText(False)
    skqss.class_(ret, 'normal')
    ret.setPlainText(
      u"「ごめんなさい。こう言う時どんな顔すればいいのか分からないの。」【綾波レイ】"
    )
    #ret.resize(300, 200)
    return ret

  @memoizedproperty
  def originTextButton(self):
    ret = QtWidgets.QCheckBox()
    ret.setChecked(True)
    return ret
  @memoizedproperty
  def originTextLabel(self):
    return self._createTextLabel(self.originTextEdit, my.tr("Apply game terms") + _TERM_STAR)
  @memoizedproperty
  def originTextEdit(self):
    return self._createTextView(my.tr("Apply game terms in the Shared Dictionary to correct game text"))
  def setOriginTextEditText(self, t):
    e = self.originTextEdit
    e.setPlainText(t)
    skqss.class_(e, 'text-muted' if t in (_EMPTY_TEXT, _DISABLED_TEXT) else '')

  @memoizedproperty
  def directTranslationLabel(self):
    return self._createTextLabel(self.directTranslationEdit, my.tr("Direct translation"))
  @memoizedproperty
  def directTranslationEdit(self):
    ret = self._createTextView(my.tr("Direct translation without modifications by VNR"))
    ret.textChanged.connect(self._refreshEqualLabel)
    return ret

  @memoizedproperty
  def jointTranslationLabel(self):
    return self._createTextLabel(self.jointTranslationEdit, my.tr("Concatenated translation"))
  @memoizedproperty
  def jointTranslationEdit(self):
    return self._createTextView(my.tr("Join split translations"))

  @memoizedproperty
  def finalTranslationLabel(self):
    return self._createTextLabel(self.finalTranslationEdit, my.tr("Final translation"))
  @memoizedproperty
  def finalTranslationEdit(self):
    ret = self._createTextView(my.tr("Actual translation used by VNR"), rich=True)
    ret.textChanged.connect(self._refreshEqualLabel)
    return ret

  @memoizedproperty
  def normalizedTextButton(self):
    ret = QtWidgets.QCheckBox();
    ret.setChecked(True)
    ret.setEnabled(False)
    return ret
  @memoizedproperty
  def normalizedTextLabel(self):
    ret = self._createTextLabel(self.normalizedTextEdit, my.tr("Apply TAH script"))
    ret.setEnabled(False)
    return ret
  @memoizedproperty
  def normalizedTextEdit(self):
    ret = self._createTextView(my.tr("Rewrite Japanese according to the rules in TAH script"))
    ret.setEnabled(False)
    return ret

  @memoizedproperty
  def sourceTextLabel(self):
    return self._createTextLabel(self.sourceTextEdit, my.tr("Apply input terms and names") + _TERM_STAR + _LANGUAGE_STAR)
  @memoizedproperty
  def sourceTextEdit(self):
    return self._createTextView(my.tr("Character names in Shared Dictionary/Game Information will be applied only for Latin-charactered languages"))

  @memoizedproperty
  def escapedTextLabel(self):
    return self._createTextLabel(self.escapedTextEdit, my.tr("Prepare escaped terms and names") + _TERM_STAR + _LANGUAGE_STAR)
  @memoizedproperty
  def escapedTextEdit(self):
    return self._createTextView(my.tr("Character names in Shared Dictionary/Game Information will be applied only for Kanji-based languages"))

  @memoizedproperty
  def escapedTranslationLabel(self):
    return self._createTextLabel(self.escapedTranslationEdit,
        my.tr("Unescape terms and names") + _TERM_STAR + _LANGUAGE_STAR)
  @memoizedproperty
  def escapedTranslationEdit(self):
    return self._createTextView(my.tr("Character names in Shared Dictionary/Game Information will be applied only for Kanji-based languages"),
        rich=True)

  @memoizedproperty
  def targetTranslationLabel(self):
    return self._createTextLabel(self.targetTranslationEdit, my.tr("Apply output terms") + _TERM_STAR)
  @memoizedproperty
  def targetTranslationEdit(self):
    return self._createTextView(my.tr("Apply output terms in the Shared Dictionary to correct translations from the machine translator"),
        rich=True)

  @memoizedproperty
  def splitTextLabel(self):
    return self._createTextLabel(self.splitTextEdit, my.tr("Split by punctuations") + _LANGUAGE_STAR)
  @memoizedproperty
  def splitTextEdit(self):
    return self._createTextView(my.tr("Split either by sentences for offline Latin languages or by paragraphs otherwise"))

  @memoizedproperty
  def splitTranslationLabel(self):
    return self._createTextLabel(self.splitTranslationEdit, my.tr("Separated translations"))
  @memoizedproperty
  def splitTranslationEdit(self):
    return self._createTextView(my.tr("Translations for split texts"))

class MTTester(QtWidgets.QDialog):

  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog | Qt.WindowMinMaxButtonsHint
    super(MTTester, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.setWindowFlags(WINDOW_FLAGS)
    self.setWindowTitle(mytr_("Test Machine Translation"))
    self.setWindowIcon(rc.icon('window-mttest'))
    self.__d = _MTTester(self)
    #self.setContentsMargins(9, 9, 9, 9)
    self.resize(800, 300)
    dprint("pass")

if __name__ == '__main__':
  a = debug.app()
  w = MTTester()
  w.show()
  a.exec_()

# EOF
