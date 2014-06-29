/** inspector.qml
 *  2/21/2013 jichi
 */
import QtQuick 1.1
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/util.min.js' as Util
import '../share' as Share
import '.' as TermView

Item { id: root_

  property QtObject currentItem // dataman.Term

  // - Private -

  height: Math.max(50, text_.height + 10)
  //color: '#ced0d6'

  Share.CachedAvatarImage { id: avatar_
    anchors {
      left: parent.left
      top: parent.top
      //verticalCenter: parent.verticalCenter
      topMargin: 3
      leftMargin: 9
    }
    width: 40; height: 40
    userId: currentItem ? currentItem.userId : 0
    userHash: currentItem ? currentItem.userHash : 0
  }

  Text { id: text_
    anchors {
      top: parent.top //; bottom: parent.bottom
      right: parent.right
      left: avatar_.visible ? avatar_.right : parent.left
      leftMargin: 9; rightMargin: 9
      topMargin: 3
    }
    textFormat: Text.PlainText
    font.pixelSize: 12

    wrapMode: Text.WordWrap

    text: summary()
  }

  function summary() {
    if (!currentItem)
      return ""

    var ret = ""
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
    return ret
  }

  TermView.Counter {
    anchors {
      top: parent.top; bottom: parent.bottom
      right: parent.right
      rightMargin: 9
      bottomMargin: 5
    }
  }
}
