/** inspector.qml
 *  1/6/2013 jichi
 *  Game inspector
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import org.sakuradite.reader 1.0 as Plugin
import '../../bootstrap3' as Bootstrap
import '../share' as Share
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/reader.min.js' as My
import '../../../js/util.min.js' as Util

Rectangle { id: root_

  property QtObject game // dataman.GameObject

  // - Private -

  //height: 30
  //height: 50
  height: topRegion_.height + bottomRegion_.height
  color: '#ddced0d6' // opacity: 0xdd/0xff = 87%

  property bool containsMouse:
    toolTip_.containsMouse ||
    openButton_.hover ||
    infoButton_.hover ||
    editButton_.hover ||
    subButton_.hover ||
    //discussButton_.hover ||
    browseButton_.hover ||
    removeButton_.hover
    //allButton_.hover

  //Plugin.MainObjectProxy { id: mainPlugin_ }
  Plugin.GameManagerProxy { id: gameman_ }
  Plugin.GameEditorManagerProxy { id: gameedit_ }
  Plugin.GameViewManagerProxy { id: gameview_ }

  Desktop.TooltipArea { id: toolTip_
    anchors.fill: parent
    //text: game ? (game.launchPath ? game.launchPath : game.path) : ""
    text: qsTr("Click to copy the game information to the clipboard and read the text using TTS")
  }

  // - Label -

  MouseArea {
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton
    onPressed:
      if (game) {
        var t = game.name
        if (t) {
          var l = [t]
          t = game.brand
          if (t) l.push(t)
          //t = game.tags
          //if (t) l.push(t)
          t = l.join("、")
          clipboardPlugin_.text = t
          ttsPlugin_.speak(t, 'ja')
        }
      }
  }

  // Top region
  Item { id: topRegion_
    anchors {
      left: parent.left; right: parent.right
      top: parent.top
      //bottom: parent.bottom
    }
    height: 50

    Image { id: gameIcon_
      anchors {
        verticalCenter: parent.verticalCenter
        left: parent.left
        leftMargin: 9
      }
      //smooth: true
      //property int size: containsMouse ? 48 : 24
      //sourceSize.width: 24; sourceSize.height: 24
      sourceSize.width: 48; sourceSize.height: 48

      source: game ? 'image://file/' + game.path : '' //'image://rc/game'
    }

    Text { id: nameLabel_
      anchors {
        verticalCenter: parent.verticalCenter
        left: gameIcon_.right
        right: toolbar_.left
        leftMargin: 9; rightMargin: 4
      }
      font.pixelSize: 12
      //font.family: 'MS HGothic'
      //font.family: 'Meiryo'
      //font.bold: root_.containsMouse
      //color: root_.containsMouse ? 'snow' : 'black'
      effect: Share.TextEffect {} // highlight: root_.containsMouse }
      text: !game ? '' : renderGame(game)

      wrapMode: Text.WordWrap
      textFormat: Text.RichText
      function renderGame(g) { // param game, return string
        var ret = g.name
        if (!g.known)
          ret += " <span style='color:red'>(" + Sk.tr('Unknown') + ")</span>"
        else {
          var brand = g.brand
          if (g.language && g.language !== 'ja')
            ret += " <span style='color:crimson'>(" + Sk.tr(g.language) + ")</span>"
          if (brand)
            ret += " <span style='color:purple'>@" + brand.replace(/,/g, " @") + "</span>"
          var tags = g.tags
          if (tags)
            ret += " <span style='color:darkblue'>" + tags + "</span>"
          //  ret += " <span style='color:darkblue'>*" + tags.replace(/,/g, " *") + "</span>"
          var ts = g.date
          if (ts > 0) {
            ts = Util.datestampToString(ts)
            ret += " <span style='color:brown'>" + ts + "</span>"
          }
        }
        return ret
      }
    }

    // - Buttons -

    Grid { id: toolbar_
      anchors {
        verticalCenter: parent.verticalCenter
        right: parent.right
        rightMargin: 2
        //right: parent.right; top: parent.top
        //topMargin: 4
      }
      //height: 30
      rows: 2 //columns: 3
      //spacing: 5
      spacing: 1

      property int cellWidth: 50

      Bootstrap.Button { id: openButton_
        styleClass: 'btn btn-primary'
        width: parent.cellWidth
        //text: Sk.tr("Launch")
        text: qsTr("Launch")
        toolTip: qsTr("Launch the game, or attach to the running game") + " (" + Sk.tr("Double-click") + ")"
        onClicked: if (game) gameman_.open(game)
      }

      Bootstrap.Button { id: editButton_
        styleClass: 'btn btn-success'
        width: parent.cellWidth
        //text: Sk.tr("Edit")
        text: qsTr("Edit")
        toolTip: qsTr("Edit game properties")
        onClicked: if (game) gameedit_.showGame(game.md5)
      }

      //Bootstrap.Button { id: discussButton_
      //  //styleClass: 'btn btn-default'
      //  styleClass: 'btn btn-info'
      //  width: parent.cellWidth
      //  //text: Sk.tr("Browse")
      //  text: qsTr("Discuss")
      //  toolTip: qsTr("Visit the discussion page online")
      //  onClicked:
      //    if (game) {
      //      var id = game.itemId
      //      if (id)
      //        Qt.openUrlExternally('http://sakuradite.com/game/' + id)
      //      else
      //        growlPlugin_.warn(My.tr("Unknown game"))
      //    }
      //}

      Bootstrap.Button { id: subButton_
        styleClass: 'btn btn-inverse'
        width: parent.cellWidth
        text: My.tr("Sub")
        toolTip: qsTr("Edit shared subtitles")
        onClicked: if (game) mainPlugin_.showGameObjectSubtitles(game)
      }

      Bootstrap.Button { id: infoButton_
        styleClass: 'btn btn-info'
        width: parent.cellWidth
        text: My.tr("Info")
        toolTip: qsTr("Show game information")
        onClicked: if (game) gameview_.showGame(game.id)
      }

      Bootstrap.Button { id: browseButton_
        styleClass: 'btn btn-default'
        width: parent.cellWidth
        //text: Sk.tr("Browse")
        text: qsTr("Browse")
        toolTip: qsTr("Show the game folder")
        onClicked: if (game) gameman_.openDirectory(game)
      }

      Bootstrap.Button { id: removeButton_
        styleClass: 'btn btn-danger'
        width: parent.cellWidth
        //text: Sk.tr("Remove")
        text: qsTr("Remove")
        toolTip: qsTr("Remove this game from the dashboard")
        onClicked: if (game) gameman_.remove(game)
      }
    }
  }

  // Bottom region
  Item { id: bottomRegion_
    anchors {
      left: parent.left; right: parent.right
      //top: parent.top
      bottom: parent.bottom
    }
    height: 0

/*
    Row { //id: typeSelector_
      //spacing: 5
      anchors {
        verticalCenter: parent.verticalCenter
        right: parent.right
        rightMargin: 2
        //right: parent.right; top: parent.top
        //topMargin: 4
      }
      //height: 30
      spacing: 1

      property int cellWidth: 50

      Bootstrap.Button { id: allButton_
        styleClass: 'btn btn-default'
        width: parent.cellWidth
        //text: Sk.tr("Launch")
        text: "全て"
        toolTip: Sk.tr("All")
        onClicked: if (game) gameman_.open(game)
      }
    }
*/
  }
}
