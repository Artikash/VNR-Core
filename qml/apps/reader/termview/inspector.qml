/** inspector.qml
 *  2/21/2013 jichi
 */
import QtQuick 1.1
import org.sakuradite.reader 1.0 as Plugin
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/reader.min.js' as My
import '../../../js/eval.min.js' as Eval
import '../../../js/linkify.min.js' as Linkify
import '../../../js/underscore.min.js' as Underscore
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

  TextEdit { id: text_
    anchors {
      top: parent.top //; bottom: parent.bottom
      right: parent.right
      left: avatar_.visible ? avatar_.right : parent.left
      leftMargin: 9; rightMargin: 9
      topMargin: 3
    }
    textFormat: TextEdit.RichText
    font.pixelSize: 12

    wrapMode: TextEdit.WordWrap
    selectByMouse: true
    onLinkActivated: Eval.evalLink(link)

    text: summary()
  }

  Plugin.JlpUtil { id: jlp_ }

  function summary() {
    if (!currentItem)
      return ""

    var pattern = Underscore.escape(currentItem.pattern) // cached
    var text = Underscore.escape(currentItem.text)
    var type = currentItem.type
    var lang = currentItem.language

    var ret = ""
    if (currentItem.errorType != 0)
      ret = Sk.tr("Check") + ": " + errorMessage(currentItem.errorType)

    if (ret)
      ret += "<br/>"
    ret += Sk.tr("Pattern") + ": " + pattern

    ret += "<br/>" + Sk.tr("Translation") + ": "
    if (!text)
      ret += "(" + Sk.tr("Delete") + ")"
    else if (text == pattern)
      ret += "(" + Sk.tr("Not changed") + ")"
    else
      ret += text

    if (text && type == 'yomi')
      ret += "<br/>" + My.tr("Yomi") + ": " + renderYomi(text)

    if (text && (type == 'escape' || type == 'target' || type == 'name' || type == 'title') && lang != 'en' && lang != 'ja')
      ret += "<br/>" + Sk.tr("Romaji") + ": " + jlp_.toroman(text, lang)

    if (currentItem.gameId > 0)
      ret += "<br/>" + Sk.tr("Game") + ": " + gameSummary(currentItem.gameId)

    var ts = Util.timestampToString(currentItem.timestamp)
    ret += "<br/>" + Sk.tr("Creation") + ": " + renderUser(currentItem.userName) + " (" + ts + ")"
    if (currentItem.comment)
      ret += ": " + renderComment(currentItem.comment)

    if (currentItem.updateUserId) {
      ret += "<br/>"
      ts = Util.timestampToString(currentItem.updateTimestamp)
      ret += Sk.tr("Update") + ": " + renderUser(currentItem.updateUserName) + " (" + ts + ")"
      if (currentItem.updateComment)
        ret += ": " + renderComment(currentItem.updateComment)
    }
    return ret
  }

  function renderYomi(text) { // string -> string
    return jlp_.kana2name(text, '') + ' (' + Sk.tr('romaji') +  '), '
         + jlp_.kana2name(text, 'ko') + ' (' + Sk.tr('ko') +  '), '
         + jlp_.kana2name(text, 'th') + ' (' + Sk.tr('th') +  ')'
  }

  function gameSummary(id) {
    if (id <= 0)
      return ""
    var itemId = datamanPlugin_.queryGameItemId(id)

    var ret = Sk.tr("File") + "ID: "
            + "<a href=\"javascript://main.searchDictionary(" + id + ",'game')\">" + id + "</a>"
    if (itemId > 0)
      ret = Sk.tr("Game") + "ID: "
          + "<a href=\"javascript://main.searchDictionary(" + itemId + ",'game')\">" + itemId + "</a>"
          + ", " + ret
    ret = "(" + ret + ")"

    var n = datamanPlugin_.queryGameName(id)
    if (!n)
      return ret
    n = Underscore.escape(n)
    n = "<a href=\"javascript://main.showGameView(" + id + ")\">" + n + "</a>"
    ret = n + " " + ret

    //if (itemId) // slow to compute
    //  ret += ' <a href="javascript://main.showGameNames(' + itemId + ')">+' + My.tr("Names") + '</a>'

    var s = datamanPlugin_.queryGameSeries(id)
    if (!s)
      return ret
    s = Underscore.escape(s)
    ret = "[<a href=\"javascript://main.searchDictionary('" + s + "','game')\">" + s + "</a>] "
        + ret
    return ret
  }

  function renderUser(name) { // string -> string
    name = Underscore.escape(name)
    return "<a href=\"javascript://main.showUser('" + name + "')\">@" + name + "</a>"
  }

  function renderComment(text) { // string -> string
    text = Underscore.escape(text, '/') // do not escape '/'
    if (~text.indexOf('//')) {
      text = text.replace(/([^:\/])\/\/([^\/])/g, '$1<br/>//$2')
      text = Linkify.parse(text)
    }
    if (~text.indexOf('@'))
      text = text.replace(/@(\w+)/g,
        "<a href=\"javascript://main.showUser('$1')\">@$1</a>"
      )
    return text
  }

  function errorMessage(v) { // int -> string
    // Must be consistent with Term error type in dataman.py
    switch (v) {
    case 0:   return "OK"
    case 5:   return qsTr("Language should be Simplified Chinese not Chinese") // W_CHINESE_TRADITIONAL
    case 6:   return qsTr("Language should be Chinese not Simplified Chinese") // W_CHINESE_SIMPLIFIED
    //case 7:   return qsTr("Having Japanese kanji in Chinese translation") // W_CHINESE_KANJI
    case 11:  return qsTr("Pattern or translation is very long") // W_LONG
    case 12:  return qsTr("Pattern is too short") // W_SHORT
    case 20:  return qsTr("Translation is empty") // W_MISSING_TEXT
    case 30:  return qsTr("Game type should not be used for translation") // W_NOT_GAME
    case 31:  return qsTr("Type should not translation instead of input") // W_NOT_INPUT
    case 100: return qsTr("Parentheses or bracks mismatch in regular expression") // W_BAD_REGEX
    case -100: return qsTr("This rule is useless that has no effect") // E_USELESS
    case -101: return qsTr("Pattern does not need enabling regex") // E_USELESS_REGEX
    case -900: return qsTr("New line characters are not allowed in text") // E_NEWLINE
    case -1000: return qsTr("Missing pattern") // E_EMPTY_PATTERN
    default: return v > 0 ? Sk.tr("Unknown warning") : Sk.tr("Unknown error")
    }
  }
}
