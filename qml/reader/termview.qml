/** termview.qml
 *  2/20/2013 jichi
 */
import QtQuick 1.1
//import QtDesktop 0.1 as Desktop
import org.sakuradite.reader 1.0 as Plugin
import '../../js/sakurakit.min.js' as Sk
import '../../js/reader.min.js' as My
import '../../js/util.min.js' as Util
import 'share' as Share
import 'termview' as TermView

//Share.View { id: root_
Item { id: root_
  //implicitWidth: 480; implicitHeight: 360
  width: 800; height: 500

  // Window properties
  property string windowTitle: title() ///< window title
  function title() {
    var ret = My.tr("Shared Dictionary")
    if (table_.count > 0)
      ret += " (" + table_.count + ")"
    else
      ret += " (" + Sk.tr("Empty") + ")"
    return ret
  }

  // - Private -

  clip: true

  Component.onCompleted: console.log("termview.qml: pass")
  Component.onDestruction: console.log("termview.qml:destroy: pass")

  Plugin.MainObjectProxy { id: mainPlugin_ }
  Plugin.DataManagerProxy { id: datamanPlugin_ }
  //Plugin.CometManagerProxy { id: cometPlugin_ }

  Plugin.Settings { id: settings_ }

  Plugin.SystemStatus { id: statusPlugin_ } // FIXME: why this is indispensible
  property int userId: statusPlugin_.online ? statusPlugin_.userId : 0
  property alias userLevel: statusPlugin_.userTermLevel


  //function loadSettings() {
  //  mainToolBar_.enabled = settings_.termEnabled
  //}
  //function saveSettings() {
  //  settings_.termEnabled = mainToolBar_.enabled
  //}

  // ToolBar at the top

  TermView.MainToolBar { id: mainToolBar_
    anchors {
      left: parent.left; right: parent.right; top: parent.top
      topMargin: -1
    }

    currentItem: table_.currentItem
    userId: root_.userId
    userLevel: root_.userLevel

    Component.onCompleted: enabled = settings_.termEnabled
    onEnabledChanged:
      if (enabled !== settings_.termEnabled)
        settings_.termEnabled = enabled
  }

  // Table
  TermView.Table { id: table_
    anchors {
      left: parent.left; right: parent.right
      top: mainToolBar_.bottom
      bottom: selectToolBar_.visible ? selectToolBar_.top : searchBox_.top
    }
    userId: root_.userId
    userLevel: root_.userLevel

    Share.Blocker {
      anchors.fill: parent
      visible: table_.currentCount <= 0 || !mainToolBar_.enabled
      text: (mainToolBar_.enabled ? Sk.tr("Empty") : Sk.tr("Disabled")) + "! ><"
    }

    TermView.NavToolBar { //id: navToolBar_ // scroll buttons
      anchors {
        bottom: parent.bottom; right: parent.right
        margins: 25
      }

      onScrollTop: table_.positionViewAtBeginning()
      onScrollBottom: table_.positionViewAtEnd()
    }

    onPageNumberChanged:
      if (paginator_.value != pageNumber)
        paginator_.value = pageNumber
  }

  TermView.SelectToolBar { id: selectToolBar_
    anchors {
      left: parent.left; right: parent.right
      bottom: searchBox_.top
    }

    selectionCount: table_.selectionCount

    visible: selectionCount > 0 && settings_.termEnabled && !!root_.userId

    model: table_.model
  }

  Share.Paginator { id: paginator_
    anchors {
      left: parent.left
      verticalCenter: searchBox_.verticalCenter
      leftMargin: 2
    }
    maximumValue: table_.maximumPageNumber
    onValueChanged:
      if (value != table_.pageNumber)
        table_.pageNumber = value
  }

  Share.SearchBox { id: searchBox_
    anchors {
      left: paginator_.right
      //left: parent.left
      right: searchToolBar_.left
      bottom: inspector_.top
      leftMargin: 5
      bottomMargin: 5
    }
    totalCount: table_.count
    currentCount: table_.currentCount
    toolTip: qsTr("Type part of the pattern, text, user, language, etc, and press Enter to search")
    onAccepted: table_.filterText = Util.trim(text)

    placeholderText: Sk.tr("Search") + " ... (" + holder() + Sk.tr("regular expression") + ", " + Sk.tr("case-insensitive") + ")"
    function holder() {
      return '@' + Sk.tr('user') + ", " + '#' + Sk.tr("game") + ", " + '#' + Sk.tr("game") + "ID, "
    }
  }

  TermView.SearchToolBar { id: searchToolBar_ // search buttons
    anchors {
      verticalCenter: searchBox_.verticalCenter
      right: parent.right
      rightMargin: 2
    }

    onTriggered: {
      searchBox_.text = text
      searchBox_.accepted()
    }
  }

  // Inspector at the bottom
  TermView.Inspector { id: inspector_
    anchors {
      left: parent.left; right: parent.right
      bottom: parent.bottom
    }
    currentItem: table_.currentItem
  }
}
