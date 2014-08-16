# coding: utf8
# mytr.py
# 10/8/2012 jichi

__all__ = ['my', 'mytr_']

from PySide.QtCore import QObject
from sakurakit.skclass import memoized

#import os
#LOCATION = os.path.dirname(__file__) + '/tr'
#TRANSLATIONS = frozenset(('ja_JP', 'zh_TW', 'zh_CN'))

# The class name must be same as reader.js
class reader(QObject):

  # Dummy translations to fool pyside-lupdate
  def translations(self):
    return (
      # Windows
      self.tr("Visual Novel Reader"),
      self.tr("Web Browser"),
      self.tr("Spring Board"),
      self.tr("Game Board"),
      self.tr("Subtitle Maker"), self.tr("Subtitle maker"),
      self.tr("Subtitle Editor"), self.tr("Subtitle editor"),
      self.tr("Post Editor"), #self.tr("Post editor"),
      self.tr("Game Settings"),
      self.tr("Game Finder"),
      self.tr("Game References"),
      self.tr("Game Properties"),
      self.tr("Text Settings"),
      self.tr("Voice Settings"),
      self.tr("Game Wizard"),
      self.tr("Shared Dictionary"),
      self.tr("Japanese Dictionary"),
      self.tr("Text Reader"),

      self.tr("YouTube Downloader"),
      self.tr("Download YouTube Videos"), self.tr("Download YouTube videos"),

      self.tr("Test Regular Expression"), self.tr("Test regular expression"),
      self.tr("Test BBCode"), #self.tr("Test BBCode"),
      self.tr("Test Machine Translation"), self.tr("Test machine translation"),

      self.tr("Title"),

      self.tr("Note"),

      # Messages

      self.tr("Unknown game"),

      # Articles
      self.tr("ctx"),

      self.tr("H-code"),

      self.tr("FS"),

      self.tr("Game engine"),

      self.tr("Matched text"),
      self.tr("Replaced text"),

      self.tr("Game text"), self.tr("game text"),
      self.tr("Machine translation"), self.tr("machine translation"),
      self.tr("Character name"), self.tr("character name"),
      self.tr("Community subtitle"),
      self.tr("User comment"),
      self.tr("User danmaku"),

      self.tr("Original text"), self.tr("original text"),
      self.tr("Voice"),
      self.tr("Speech"),
      self.tr("Embedded"),
      self.tr("Speak"),
      self.tr("Text-to-speech"),

      self.tr("Context count"),
      self.tr("Previous context"),

      self.tr("Scenario"), self.tr("scenario"),
      self.tr("Character"), self.tr("character"),

      self.tr("Dialog"), self.tr("dialog"),
      self.tr("Chara"), self.tr("chara"),

      self.tr('Moderate anonymous subs'),
      self.tr('Moderate anonymous terms'),

      self.tr('Discuss'),
      self.tr('Discussion'),
      self.tr('Game Discussion'),

      self.tr('Serious'),
      self.tr('Ecchi'),
      self.tr('Easy'),

      self.tr("ATLAS"),
      self.tr("Baidu"),
      #self.tr("Youdao"),
      self.tr("Infoseek"),
      self.tr("Excite"),
      self.tr("Yahoo!"),
      self.tr("Bing"),
      self.tr("Google"),
      self.tr("LEC"),
      self.tr("LEC Online"),
      #self.tr("Translate.Ru"),
      self.tr("ezTrans XP"),
      self.tr("Kojien"),
      self.tr("Daijirin"),
      self.tr("Daijisen"),
      self.tr("Wadoku"),
      self.tr("Zhongri"),
      self.tr("JBeijing"),
      self.tr("JBeijing7"),
      self.tr("Dr.eye"),
      #self.tr("Han Viet"),

      self.tr("Built-in"),
      self.tr("built-in"),

      self.tr("Shortcuts"),

      self.tr("Need to install"),
      self.tr("need to install"),

      self.tr("Installing"),
      self.tr("Installed"),
      self.tr("Not installed"),

      self.tr("Launcher"),

      self.tr("Game language"),
      self.tr("Text thread"),
      self.tr("Text threads"),
      self.tr("Hook code"),

      self.tr("Update reason"),

      self.tr("Fill color"),
      self.tr("Background shadow"),

      self.tr("Text color"),

      self.tr("Sub"),
      self.tr("Info"),

      self.tr("Transp"),

      #self.tr("Background shadow color"),
      #self.tr("Game text color"),
      #self.tr("Machine translation color"),
      #self.tr("Community subtitle color"),
      #self.tr("User comment color"),
      #self.tr("User danmaku color"),

      self.tr("Game-specific"),
      self.tr("Series-specific"),
      self.tr("Current game"),

      self.tr("{0} games"),

      self.tr("{0} people are viewing this page"),

      # Actions

      #self.tr("Stretch"),

      #self.tr("Show {0}"),
      #self.tr("Hide {0}"),
      self.tr("Speak {0}"),

      self.tr("Capture"),

      self.tr("Open in external browser"),
      self.tr("Open in external window"),

      self.tr("Monitor mouse"),
      self.tr("Monitor clipboard"),

      self.tr("Auto Hide"),
      self.tr("Edit Subtitle"),
      self.tr("New comment"),
      self.tr("New subtitle"),
      self.tr("Add comment"), self.tr("add comment"),
      self.tr("Add subtitle"), self.tr("add subtitle"),

      self.tr("Read Sentence"),

      self.tr("Sync with Running Game"),
      self.tr("Update Shared Dictionary"),
      self.tr("Update Shared Subtitles"),
      self.tr("Update Game Database"),
      self.tr("Update Translation Scripts"),
      self.tr("Reload Translation Scripts"),
    )

@memoized
def manager(): return reader()

def mytr_(text): return manager().tr(text)

class my(QObject): pass
my = my()

# EOF
