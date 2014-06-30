# coding: utf8
# qmlplugin.py
# 10/7/2012 jichi
#
# Categorization of QML plugins
# - Standalone: should be avoid!
# - Associative
#   - Proxy: QML => (invoke) => Python
#   - Delegate: QML <= (invoke) <= Python
#

from Qt5.QtQml import qmlRegisterType
import features

QML_EFFECT = 'QtEffects'
QML_PLUGIN = 'org.sakuradite.reader'

# See: http://stackoverflow.com/questions/8894244/qml-and-blurring-image
from Qt5 import QtWidgets
qmlRegisterType(QtWidgets.QGraphicsBlurEffect, QML_EFFECT, 1, 0, 'Blur')
qmlRegisterType(QtWidgets.QGraphicsColorizeEffect, QML_EFFECT, 1, 0, 'Colorize')
qmlRegisterType(QtWidgets.QGraphicsDropShadowEffect, QML_EFFECT, 1, 0, 'DropShadow')
qmlRegisterType(QtWidgets.QGraphicsOpacityEffect, QML_EFFECT, 1, 0, 'Opacity')

from sakurakit import skqml
qmlRegisterType(skqml.SkDeclarativeBusySpinner, QML_PLUGIN, 1, 0, 'BusySpinner')
qmlRegisterType(skqml.SkDesktopProxy, QML_PLUGIN, 1, 0, 'DesktopProxy')
qmlRegisterType(skqml.SkClipboardProxy, QML_PLUGIN, 1, 0, 'ClipboardProxy')

#from sakuradite import skqmlnet
#qmlRegisterType(skqmlnet.QmlAjax, QML_PLUGIN, 1, 0, 'Ajax')

import main
qmlRegisterType(main.MainObjectProxy, QML_PLUGIN, 1, 0, 'MainObjectProxy')

import ttsman
qmlRegisterType(ttsman.TtsQmlProxy, QML_PLUGIN, 1, 0, 'Tts')

import settings
qmlRegisterType(settings.SettingsProxy, QML_PLUGIN, 1, 0, 'Settings')

import qmlutil
qmlRegisterType(qmlutil.QmlUtil, QML_PLUGIN, 1, 0, 'Util')
qmlRegisterType(qmlutil.BBCodeParser, QML_PLUGIN, 1, 0, 'BBCodeParser')

#import qmlbeans
#qmlRegisterType(qmlbeans.SliderBean, QML_PLUGIN, 1, 0, 'SliderBean')

import spell
qmlRegisterType(spell.SpellChecker, QML_PLUGIN, 1, 0, 'SpellChecker')

import mecabman
qmlRegisterType(mecabman.QmlMeCabHighlighter, QML_PLUGIN, 1, 0, 'MeCabHighlighter')

import status
qmlRegisterType(status.SystemStatus, QML_PLUGIN, 1, 0, 'SystemStatus')
qmlRegisterType(status.ThreadPoolStatus, QML_PLUGIN, 1, 0, 'ThreadPoolStatus')

import kagami
qmlRegisterType(kagami.GospelBean, QML_PLUGIN, 1, 0, 'GospelBean')
qmlRegisterType(kagami.GossipBean, QML_PLUGIN, 1, 0, 'GossipBean')
qmlRegisterType(kagami.GrimoireBean, QML_PLUGIN, 1, 0, 'GrimoireBean')
qmlRegisterType(kagami.OmajinaiBean, QML_PLUGIN, 1, 0, 'OmajinaiBean')
qmlRegisterType(kagami.MirageBean, QML_PLUGIN, 1, 0, 'MirageBean')

import growl
qmlRegisterType(growl.GrowlBean, QML_PLUGIN, 1, 0, 'GrowlBean')
qmlRegisterType(growl.GrowlQmlProxy, QML_PLUGIN, 1, 0, 'Growl')

import shiori
qmlRegisterType(shiori.ShioriBean, QML_PLUGIN, 1, 0, 'ShioriBean')
qmlRegisterType(shiori.ShioriQmlProxy, QML_PLUGIN, 1, 0, 'ShioriProxy')

import submaker
qmlRegisterType(submaker.GraffitiBean, QML_PLUGIN, 1, 0, 'GraffitiBean')
qmlRegisterType(submaker.SubtitleContextBean, QML_PLUGIN, 1, 0, 'SubtitleContextBean')

import textman
qmlRegisterType(textman.TextManagerProxy, QML_PLUGIN, 1, 0, 'TextManagerProxy')

#import cometman
#qmlRegisterType(cometman.CometManagerProxy, QML_PLUGIN, 1, 0, 'CometManagerProxy')

import hkman
qmlRegisterType(hkman.HotkeyManagerProxy, QML_PLUGIN, 1, 0, 'HotkeyManagerProxy')

import dataman
qmlRegisterType(dataman.DataManagerProxy, QML_PLUGIN, 1, 0, 'DataManagerProxy')
qmlRegisterType(dataman.CommentModel, QML_PLUGIN, 1, 0, 'CommentModel')
qmlRegisterType(dataman.GameModel, QML_PLUGIN, 1, 0, 'GameModel')
qmlRegisterType(dataman.ReferenceModel, QML_PLUGIN, 1, 0, 'ReferenceModel')
qmlRegisterType(dataman.TermModel, QML_PLUGIN, 1, 0, 'TermModel')
qmlRegisterType(dataman.VoiceModel, QML_PLUGIN, 1, 0, 'VoiceModel')

import gameman
qmlRegisterType(gameman.TaskBarProxy, QML_PLUGIN, 1, 0, 'TaskBarProxy')
qmlRegisterType(gameman.GameProxy, QML_PLUGIN, 1, 0, 'GameProxy')
qmlRegisterType(gameman.GameWindowProxy, QML_PLUGIN, 1, 0, 'GameWindowProxy')
qmlRegisterType(gameman.GameManagerProxy, QML_PLUGIN, 1, 0, 'GameManagerProxy')

import subedit
qmlRegisterType(subedit.SubtitleEditorManagerProxy, QML_PLUGIN, 1, 0, 'SubtitleEditorManagerProxy')

import postedit
qmlRegisterType(postedit.PostEditorManagerProxy, QML_PLUGIN, 1, 0, 'PostEditorManagerProxy')

import gameedit
qmlRegisterType(gameedit.GameEditorManagerProxy, QML_PLUGIN, 1, 0, 'GameEditorManagerProxy')

#import prompt
#qmlRegisterType(prompt.PromptProxy, QML_PLUGIN, 1, 0, 'PromptProxy')

import gameview
qmlRegisterType(gameview.GameViewManagerProxy, QML_PLUGIN, 1, 0, 'GameViewManagerProxy')

import userview
qmlRegisterType(userview.UserViewManagerProxy, QML_PLUGIN, 1, 0, 'UserViewManagerProxy')

import refinput
qmlRegisterType(refinput.QmlReferenceInput, QML_PLUGIN, 1, 0, 'ReferenceInput')

if not features.WINE:
  import textspy
  qmlRegisterType(textspy.TextSpyProxy, QML_PLUGIN, 1, 0, 'TextSpyProxy')
  import textreader
  qmlRegisterType(textreader.TextReaderProxy, QML_PLUGIN, 1, 0, 'TextReaderProxy')

# EOF
