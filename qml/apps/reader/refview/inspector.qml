/** inspector.qml
 *  2/21/2013 jichi
 */
import QtQuick 1.1
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/util.min.js' as Util
import '../share' as Share

Item { id: root_

  property QtObject currentItem // dataman.Reference
  property QtObject model // dataman.ReferenceModel

  // - Private -

  height: 70 // should be large enough to cover all texts
  //color: '#ced0d6'

  Share.CachedAvatarImage { id: avatar_
    anchors {
      left: parent.left; top: parent.top
      leftMargin: 9; topMargin: 5
    }
    width: 40; height: 40
    userId: currentItem ? currentItem.userId : 0
    userHash: currentItem ? currentItem.userHash : 0
  }

  Text {
    anchors {
      top: parent.top; bottom: footer_.bottom
      right: parent.right
      left: avatar_.visible ? avatar_.right : parent.left
      leftMargin: 9; rightMargin: 9
      topMargin: 5
    }
    textFormat: Text.PlainText
    font.pixelSize: 12

    wrapMode: Text.WordWrap

    text: summary()
  }

  function summary() {
    var ret = ""
    if (currentItem) {
      var ts = Util.timestampToString(currentItem.timestamp)
      ret += Sk.tr("Creation") + ": @" + currentItem.userName + " (" + ts + ")"
      if (currentItem.comment)
        ret += ": " + currentItem.comment

      if (currentItem.updateUserId) {
        ret += "\n"
        ts = Util.timestampToString(currentItem.updateTimestamp)
        ret += Sk.tr("Update") + ": @" + currentItem.updateUserName + " (" + ts + ")"
        if (currentItem.updateComment)
          ret += ": " + currentItem.updateComment
      }
    }
    return ret
  }

  Row { id: footer_
    anchors {
      left: parent.left; right: parent.right
      bottom: parent.bottom
      leftMargin: 9; rightMargin: 9
      bottomMargin: 5
    }

    spacing: 20

    //property int cellWidth: 80
    property int pixelSize: 12

    Text {
      //width: parent.cellWidth
      font.pixelSize: parent.pixelSize
      textFormat: Text.RichText
      wrapMode: Text.NoWrap
      onLinkActivated: Qt.openUrlExternally(link)
      text: root_.formatText('ErogeTrailers', 'erogetrailers.com', model.trailersItem ? 'green' : 'red')
    }

    Text {
      //width: parent.cellWidth
      font.pixelSize: parent.pixelSize
      textFormat: Text.RichText
      wrapMode: Text.NoWrap
      onLinkActivated: Qt.openUrlExternally(link)
      text: root_.formatText('ErogameScape', 'erogamescape.dyndns.org/~ap2/ero/toukei_kaiseki', model.scapeItem ? 'green' : 'red')
    }

    Text {
      //width: parent.cellWidth
      font.pixelSize: parent.pixelSize
      textFormat: Text.RichText
      wrapMode: Text.NoWrap
      onLinkActivated: Qt.openUrlExternally(link)
      text: root_.formatText('Holyseal', 'holyseal.net', model.holysealItem ? 'green' : 'brown')
    }

    Text {
      //width: parent.cellWidth
      font.pixelSize: parent.pixelSize
      textFormat: Text.RichText
      wrapMode: Text.NoWrap
      onLinkActivated: Qt.openUrlExternally(link)
      text: root_.formatText('Getchu', 'getchu.com', model.getchuItem ? 'green' : 'brown')
    }

    Text {
      //width: parent.cellWidth
      font.pixelSize: parent.pixelSize
      textFormat: Text.RichText
      wrapMode: Text.NoWrap
      onLinkActivated: Qt.openUrlExternally(link)
      text: root_.formatText('Amazon', 'amazon.co.jp', model.amazonItem ? 'green' : 'brown')
    }

    Text {
      //width: parent.cellWidth
      font.pixelSize: parent.pixelSize
      textFormat: Text.RichText
      wrapMode: Text.NoWrap
      onLinkActivated: Qt.openUrlExternally(link)
      text: root_.formatText('DMM', 'dmm.co.jp', model.dmmItem ? 'green' : 'brown')
    }

    Text {
      //width: parent.cellWidth
      font.pixelSize: parent.pixelSize
      textFormat: Text.RichText
      wrapMode: Text.NoWrap
      onLinkActivated: Qt.openUrlExternally(link)
      text: root_.formatText('DLsite', 'dlsite.co.jp', model.dlsiteItem ? 'green' : 'brown')
    }

    Text {
      //width: parent.cellWidth
      font.pixelSize: parent.pixelSize
      textFormat: Text.RichText
      wrapMode: Text.NoWrap
      onLinkActivated: Qt.openUrlExternally(link)
      text: root_.formatText('DiGiket', 'www.digiket.com', model.digiketItem ? 'green' : 'brown')
    }

    Text {
      //width: parent.cellWidth
      font.pixelSize: parent.pixelSize
      textFormat: Text.RichText
      wrapMode: Text.NoWrap
      onLinkActivated: Qt.openUrlExternally(link)
      text: root_.formatText('Gyutto', 'gyutto.com', model.gyuttoItem ? 'green' : 'brown')
    }
  }

  function formatText(name, url,color) { // string -> string
    return '<a style="color:' + color + '" href="http://' + url + '">' + name + '</a>'
  }
}
