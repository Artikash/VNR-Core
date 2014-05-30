/** subview.qml
 *  6/1/2013 jichi
 */
import QtQuick 1.1
//import QtDesktop 0.1 as Desktop
import org.sakuradite.reader 1.0 as Plugin
import '../../js/sakurakit.min.js' as Sk
import '../../js/util.min.js' as Util
import 'share' as Share
import 'subview' as SubView

Share.View { id: root_
  //implicitWidth: 480; implicitHeight: 360
  width: 800; height: 500

  // Window properties
  property string windowTitle: title() ///< window title
  function title() {
    var ret = Sk.tr("Subtitles")
    if (model_.count > 0)
      ret += " (" + model_.count + ")"
    else
      ret += " (" + Sk.tr("Empty") + ")"
    if (game)
      ret += " - " + game.name
    return ret
  }

  property QtObject game // dataman.GameObject, to be set by constructor

  // - Private -

  Component.onCompleted: console.log("subview.qml: pass")
  Component.onDestruction: console.log("subview.qml:destroy: pass")

  Plugin.MainObjectProxy { id: mainPlugin_ }
  Plugin.DataManagerProxy { id: datamanPlugin_ }

  Plugin.CommentModel { id: model_
    gameMd5: game ? game.md5 : ""
    sortingColumn: table_.sortIndicatorColumn
    sortingReverse: table_.sortIndicatorDirection === 'up'

    property int maximumPageNumber: Math.ceil(currentCount / pageSize)
    onPageNumberChanged:
      if (paginator_.value != pageNumber)
        paginator_.value = pageNumber
  }

  //Plugin.Settings { id: settings_ }

  Plugin.SystemStatus { id: status_ }
  property int userId: status_.online ? status_.userId : 0
  property alias userLevel: status_.userCommentLevel

  //function loadSettings() {
  //  toolBar_.enabled = settings_.termEnabled
  //}
  //function saveSettings() {
  //  settings_.termEnabled = toolBar_.enabled
  //}

  // ToolBar at the top

  SubView.MainToolBar { id: toolBar_
    anchors {
      left: parent.left; right: parent.right; top: parent.top
      topMargin: -1
    }

    model: model_
    //currentGame: root_.game
    currentItem: table_.currentItem
    userId: root_.userId
    userLevel: root_.userLevel

    //Component.onCompleted: enabled = settings_.termEnabled
    //onEnabledChanged:
    //  if (enabled !== settings_.termEnabled)
    //    settings_.termEnabled = enabled
  }

  // Table
  SubView.Table { id: table_
    anchors {
      left: parent.left; right: parent.right
      top: toolBar_.bottom
      //bottom: searchBox_.top
      bottom: selectToolBar_.visible ? selectToolBar_.top : searchBox_.top
    }
    model: model_
    userId: root_.userId
    userLevel: root_.userLevel
    //currentGame: root_.game
    //filterText: Util.trim(searchBox_.text)

    property QtObject currentItem: model.get(currentIndex) // dataman.Comment

    Share.Navigator { //id: navToolBar_ // scroll buttons
      anchors {
        bottom: parent.bottom; right: parent.right
        margins: 25
      }

      onScrollTop: table_.positionViewAtBeginning()
      onScrollBottom: table_.positionViewAtEnd()
    }

    Share.Blocker {
      anchors.fill: parent
      visible: model_.currentCount <= 0
    }
  }

  SubView.SelectToolBar { id: selectToolBar_
    anchors {
      left: parent.left; right: parent.right
      bottom: searchBox_.top
    }

    selectionCount: model_.selectionCount

    visible: selectionCount > 0 && !!root_.userId

    model: model_
  }

  Share.Paginator { id: paginator_
    anchors {
      left: parent.left
      verticalCenter: searchBox_.verticalCenter
      leftMargin: 2
    }
    maximumValue: model_.maximumPageNumber
    onValueChanged:
      if (value != model_.pageNumber)
        model_.pageNumber = value
  }

  Share.SearchBox { id: searchBox_
    anchors {
      left: paginator_.right
      //left: parent.left
      right: parent.right
      bottom: inspector_.top
      bottomMargin: 5
      leftMargin: 5
    }
    totalCount: model_.count
    currentCount: model_.currentCount
    toolTip: qsTr("Type part of the text, context, user, language, etc, and press Enter to search")
    onAccepted: model_.filterText = Util.trim(text)

    placeholderText: Sk.tr("Search") + " ... (" + holder() + Sk.tr("regular expression") + ", " + Sk.tr("case-insensitive") + ")"
    function holder() {
      return '@' + Sk.tr('user') + ", " + '#' + Sk.tr("game") + ", " + '#' + Sk.tr("game") + "ID, "
    }
  }

  // Inspector at the bottom
  SubView.Inspector { id: inspector_
    anchors {
      left: parent.left; right: parent.right
      bottom: parent.bottom
    }
    currentItem: table_.currentItem
  }
}
