/** appmodel.qml
 *  9/30/2012 jichi
 *  Model for applets
 *
 *  It should contain either "show" or "run" method.
 */
import QtQuick 1.1
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/reader.min.js' as My

// Translation on property text is not allowed
ListModel {

  // - Private -

  /**
   *  Applet properties:
   *  - name: name of the applet
   *  - icon: image of the applet
   *  - toolTip: popup tool tip
   *  - searchText: string used for searching
   *  - qml: String that could be executed by Qt.createQmlObject
   *  It should contain either "show" or "run" method.
   */

  Component.onCompleted: {

    append({ name: Sk.tr("Software Update")
      , searchText: "Software Update" + "\n" + Sk.tr("Software Update")
      , icon: 'image://rc/app-update'
      , toolTip: My.tr("Check for updates")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { update() } \
} \
"
    })

    append({ name: Sk.tr("Preferences")
      , searchText: "Preferences" + "\n" + Sk.tr("Preferences")
      , icon: 'image://rc/dock-prefs'
      , toolTip: qsTr("Settings of language, translation, UI, etc")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showPreferences() } \
} \
"
    })

    append({ name: My.tr("Game Wizard")
      , searchText: "Game Wizard" + "\n" + My.tr("Game Wizard")
      , icon: 'image://rc/dock-gamewiz'
      , toolTip: qsTr("Add new game step by step")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showGameWizard() } \
} \
"
    })

    append({ name: My.tr("Shared Dictionary")
      , searchText: "Shared Dictionary" + "\n" + My.tr("Shared Dictionary")
      , icon: 'image://rc/dock-dict'
      , toolTip: qsTr("Shared dictionary for machine translation")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showTermView() } \
} \
"
    })

    append({ name: Sk.tr("Machine Translation")
      , searchText: "Machine Translation" + "\n" + Sk.tr("Machine Translation")
      , icon: 'image://rc/dock-mttest'
      , toolTip: My.tr("Test machine translation")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showMachineTranslationTester() } \
} \
"
    })

    append({ name: My.tr("Japanese Dictionary")
      , searchText: "Japanese Dictionary" + "\n" + Sk.tr("Japanese Dictionary")
      , icon: 'image://rc/dock-jdict'
      , toolTip: qsTr("Japanese word dictionary")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showDictionaryTester() } \
} \
"
    })

    if (!statusPlugin_.wine)
      append({ name: My.tr("Text Reader") + " (α)" // あるふぁ
      , searchText: "Text Reader" + "\n" + My.tr("Text Reader")
      , icon: 'image://rc/dock-textreader'
      , toolTip: qsTr("Window text translator")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showTextReader() } \
} \
"
    })

    append({ name: My.tr("Game Finder") + " (α)" // あるふぁ
      , searchText: "Game Finder" + "\n" + My.tr("Game Finder")
      , icon: 'image://rc/dock-gamefinder'
      , toolTip: qsTr("Searching game information online")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showGameFinder() } \
} \
"
    })

    append({ name: My.tr("Game Board")
      , searchText: "Game Board" + "\n" + My.tr("Game Board")
      , icon: 'image://rc/dock-gameboard'
      , toolTip: qsTr("Information of online games")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showGameBoard() } \
} \
"
    })

    append({ name: Sk.tr("Regular Expression")
      , searchText: "Regular Expression" + "\n" + Sk.tr("Regular Expression")
      , icon: 'image://rc/dock-regexp'
      , toolTip: My.tr("Test regular expression")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showRegExpTester() } \
} \
"
    })

    append({ name: "BBCode"
      , searchText: "BBCode"
      , icon: 'image://rc/dock-bbcode'
      , toolTip: My.tr("Test BBCode")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showBBCodeTester() } \
} \
"
    })

    if (!statusPlugin_.wine)
      append({ name: My.tr("YouTube Downloader")
      , searchText: "YouTube Downloader" + "\n" + My.tr("Download YouTube videos")
      , icon: 'image://rc/dock-youtube'
      , toolTip: My.tr("Download YouTube videos")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showYouTubeInput() } \
} \
"
    })

    if (!statusPlugin_.wine)
      append({ name: My.tr("Web Browser")
      , searchText: "Web Browser" + "\n" + My.tr("Web Browser")
      , icon: 'image://rc/dock-browser'
      , toolTip: qsTr("VNR's built-in web browser")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showWebBrowser() } \
} \
"
    })
  }
}
