/** inspector.qml
 *  2/21/2013 jichi
 */
import QtQuick 1.1
import org.sakuradite.reader 1.0 as Plugin
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/reader.min.js' as My
import '../../../js/util.min.js' as Util
import '../share' as Share
import '.' as TermView

Item { id: root_

  property QtObject currentItem // dataman.Term

  // - Private -

  height: Math.max(60, text_.height + 10)
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

  Plugin.JlpUtil { id: jlp_ }

  function summary() {
    if (!currentItem)
      return ""

    var pattern = currentItem.pattern // cached
    var text = currentItem.text
    var type = currentItem.type
    var lang = currentItem.language

    var ret = Sk.tr("Pattern") + ": " + pattern

    ret += "\n" + Sk.tr("Translation") + ": "
    if (!text)
      ret += "(" + Sk.tr("Delete") + ")"
    else if (text == pattern)
      ret += "(" + Sk.tr("Not changed") + ")"
    else
      ret += text

    if (text && type == 'yomi')
      ret += "\n" + My.tr("Yomi") + ": " + renderYomi(text)

    if (text && (type == 'escape' || type == 'target' || type == 'name') && lang != 'en' && lang != 'ja')
      ret += "\n" + Sk.tr("Romaji") + ": " + jlp_.toromaji(text, lang)

    var ts = Util.timestampToString(currentItem.timestamp)
    ret += "\n" + Sk.tr("Creation") + ": @" + currentItem.userName + " (" + ts + ")"
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

  function renderYomi(text) { // string -> string
    return jlp_.kana2yomi(text, '') + ' (' + Sk.tr('romaji') +  '), '
         + jlp_.kana2yomi(text, 'ko') + ' (' + Sk.tr('ko') +  '), '
         + jlp_.kana2yomi(text, 'th') + ' (' + Sk.tr('th') +  ')'
  }
}
