/** dockmodel.qml
 *  9/30/2012 jichi
 *  Model for applets
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
   *  - qml: String that could be executed by Qt.createQmlObject
   *  It should contain either "show" or "run" method.
   */

  Component.onCompleted: {
    append({ name: Sk.tr("Launch")
      , icon: 'image://rc/dock-launch'
      , toolTip: qsTr("Sync with current running game by searching the online database")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { openRunningGame() } \
} \
"
    })

    append({ name: My.tr("Game Wizard")
      , icon: 'image://rc/dock-gamewiz'
      , toolTip: qsTr("Add new game from my computer")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showGameWizard() } \
} \
"
    })

    append({ name: My.tr("Game Board")
      , icon: 'image://rc/dock-gameboard'
      , toolTip: qsTr("Information of online games")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showGameBoard() } \
} \
"
    })

    append({ name: My.tr("Web Browser")
      , icon: 'image://rc/dock-browser'
      , toolTip: qsTr("VNR's built-in web browser")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showWebBrowser() } \
} \
"
    })

    append({ name: Sk.tr("Preferences")
      , icon: 'image://rc/dock-prefs'
      , toolTip: qsTr("Settings of language, translation, UI, etc")
      , qml: "\
import org.sakuradite.reader 1.0 as Plugin; \
Plugin.MainObjectProxy { \
  function run() { showPreferences() } \
} \
"
    })
  }
}
