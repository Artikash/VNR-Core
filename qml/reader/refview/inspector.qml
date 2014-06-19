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

  height: 150 // should be large enough to cover all texts
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

  Text { id: footer_
    anchors {
      left: parent.left; right: parent.right
      bottom: parent.bottom
      leftMargin: 9; rightMargin: 9
      bottomMargin: 5
    }
    textFormat: Text.RichText
    font.pixelSize: 12

    wrapMode: Text.WordWrap

    text: footer()
  }

  function footer() {
    var ret = ""
    if (model.trailersItem)
      ret += '<br/><span style="color:green">' + qsTr("Found 1 reference from {0}").replace('{0}', 'ErogeTrailers.com') + '</span>'
    else
      ret += '<br/><span style="color:red">' + qsTr("Missing references from {0}").replace('{0}', 'ErogeTrailers.com') + '</span>'
    if (model.scapeItem)
      ret += '<br/><span style="color:green">' + qsTr("Found 1 reference from {0}").replace('{0}', 'ErogameScape') + '</span>'
    else
      ret += '<br/><span style="color:red">' + qsTr("Missing references from {0}").replace('{0}', 'ErogameScape') + '</span>'
    if (model.getchuItem)
      ret += '<br/><span style="color:green">' + qsTr("Found 1 reference from {0}").replace('{0}', 'Getchu.com') + '</span>'
    else
      ret += '<br/><span style="color:red">' + qsTr("Missing references from {0}").replace('{0}', 'Getchu.com') + '</span>'
    if (model.holysealItem)
      ret += '<br/><span style="color:green">' + qsTr("Found 1 reference from {0}").replace('{0}', 'Holyseal.net') + '</span>'
    else
      ret += '<br/><span style="color:brown">' + qsTr("Missing references from {0}").replace('{0}', 'Holyseal.net') + '</span>'
    if (model.dmmItem)
      ret += '<br/><span style="color:green">' + qsTr("Found 1 reference from {0}").replace('{0}', 'DMM.co.jp') + '</span>'
    else
      ret += '<br/><span style="color:brown">' + qsTr("Missing references from {0}").replace('{0}', 'DMM.co.jp') + '</span>'
    if (model.amazonItem)
      ret += '<br/><span style="color:green">' + qsTr("Found 1 reference from {0}").replace('{0}', 'Amazon.co.jp') + '</span>'
    else
      ret += '<br/><span style="color:brown">' + qsTr("Missing references from {0}").replace('{0}', 'Amazon.co.jp') + '</span>'
    if (model.dlsiteItem)
      ret += '<br/><span style="color:green">' + qsTr("Found 1 reference from {0}").replace('{0}', 'DLsite.com') + '</span>'
    else
      ret += '<br/><span style="color:brown">' + qsTr("Missing references from {0}").replace('{0}', 'DLsite.com') + '</span>'
    return ret
  }
}
