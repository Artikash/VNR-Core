/** table.qml
 *  2/20/2013 jichi
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import org.sakuradite.reader 1.0 as Plugin
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/reader.min.js' as My
import '../../../js/util.min.js' as Util

Item { id: root_

  property QtObject currentItem: model_.get(table_.currentIndex)

  property alias model: table_.model

  property alias displaysDuplicateRows: model_.duplicate

  property int userId
  property int userLevel
  property alias filterText: model_.filterText
  property alias filterTypes: model_.filterTypes
  property alias filterColumn: model_.filterColumn
  property alias filterLanguage: model_.filterLanguage
  property alias currentCount: model_.currentCount
  property alias count: model_.count

  property alias selectionCount: model_.selectionCount

  property alias pageNumber: model_.pageNumber
  //property alias pageSize: model_.pageSize

  function positionViewAtBeginning() { table_.positionViewAtBeginning() }
  function positionViewAtEnd() { table_.positionViewAtEnd() }

  // Not sure why this causes loop binding
  property int maximumPageNumber //:Math.ceil(model_.currentCount / model_.pageSize)
  // - Private -
  function updateMaximumPageNumber() {
    root_.maximumPageNumber = Math.ceil(model_.currentCount / model_.pageSize) // assume pageSize != 0
  }

  function refresh() { model_.refresh() }

  property int _GUEST_USER_ID: 4
  property int _SUPER_USER_ID: 2

  //property color _DISABLED_COLOR: 'silver'
  property color _EDITABLE_COLOR: 'green'
  property color _READONLY_COLOR: 'steelblue'
  property color _HIGHLIGHT_COLOR: 'red'

  //Plugin.DataManagerProxy { id: datamanPlugin_ }

  property string _UNSAVED_RICH_TEXT: // cached
       '<span style="background-color:red">' + Sk.tr("Unsaved") + '</span>'

  function canEdit(term) {
    return !!term && !!userId && (userId === _SUPER_USER_ID
        || term.userId === userId && !term.protected
        || term.userId === _GUEST_USER_ID && userLevel > 0)
  }

  function canImprove(term) {
    return !!term && !!userId && (term.userId === userId && !term.protected
        || !!userId && userId !== _GUEST_USER_ID)
  }

  function canSelect(term) {
    return canImprove(term)
  }

  function shouldHighlight(term) {
    return term.private_ || term.language === 'ja' || term.type === 'origin' || term.type === 'macro'
  }

  function itemColor(term) {
    return shouldHighlight(term) ? _HIGHLIGHT_COLOR :
      datamanPlugin_.queryUserColor(term.userId) || (
        canEdit(term) ? _EDITABLE_COLOR : 'black')
  }

  function commentColor(term) {
    return datamanPlugin_.queryUserColor(term.userId) || (
        canEdit(term) ? _EDITABLE_COLOR : 'black')
  }

  function updateCommentColor(term) {
    return datamanPlugin_.queryUserColor(term.updateUserId) || (
        canEdit(term) ? _EDITABLE_COLOR : 'black')
  }

  function gameSummary(id) {
    if (id <= 0)
      return ""
    var n = datamanPlugin_.queryGameName(id)
    if (!n)
      return "(" + id + ")"
    var s = datamanPlugin_.queryGameSeries(id)
    if (!s)
      return n + " (" + id + ")"
    return '[' + s + '] ' + n + " (" + id + ")"
  }

  function getErrorColor(v) { // int -> string
    return v > 10 ? 'orange'
         : v < 0 ? 'red'
         : 'green'
  }

  function getErrorText(v) { // int -> string
    switch (v) {
    case 0: return 'OK' // OK
    case 5: return Sk.tr("zht")    // W_CHINESE_TRADITIONAL
    case 6: return Sk.tr("zhs")    // W_CHINESE_SIMPLIFIED
    //case 7:  return Sk.tr("Kanji")  // W_CHINESE_KANJI
    case 11: return qsTr("Short")   // W_SHORT
    case 12: return qsTr("Long")    // W_LONG
    case 20: return qsTr("Missing") // W_MISSING_TEXT
    case 30: return Sk.tr("Game")   // W_NOT_GAME
    case 31: return My.tr("Input")  // W_NOT_INPUT
    case 100: return qsTr("Brackets") // W_BAD_REGEX
    case -100: return qsTr("Useless") // E_USELESS
    case -101: return qsTr("Regex") // E_USELESS_REGEX
    //case -1000: // E_EMPTY_PATTERN
    default: return v > 0 ? Sk.tr('Warning') : Sk.tr('Error')
    }
  }

  property int _MIN_TEXT_LENGTH: 1
  property int _MAX_TEXT_LENGTH: 255

  Plugin.TermModel { id: model_
    sortingReverse: table_.sortIndicatorDirection === 'up'
    sortingColumn: table_.sortColumn

    // FIX the loop binding bug
    Component.onCompleted: {
      root_.updateMaximumPageNumber()
      currentCountChanged.connect(root_.updateMaximumPageNumber)
      pageSizeChanged.connect(root_.updateMaximumPageNumber)
    }
  }

  Desktop.TableView { id: table_
    anchors.fill: parent
    model: model_

    contentWidth: width // Prevent recursive binding bug in QtDesktop

    sortIndicatorVisible: true

    property int cellHeight: 25
    property int cellSpacing: 5

    sortColumn: 1 // the initial sorting column is the second one

    // Column: Selected
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Select")
      width: 30
      delegate: Item {
        height: table_.cellHeight
        Desktop.CheckBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          enabled: canSelect(itemValue)
          checked: itemValue.selected
          onCheckedChanged:
            if (enabled && checked !== itemValue.selected) {
              itemValue.selected = checked
              model_.refreshSelection()
            }

          function setChecked(t) { checked = t }
          Component.onCompleted: itemValue.selectedChanged.connect(setChecked)
          Component.onDestruction: itemValue.selectedChanged.disconnect(setChecked)
        }
      }
    }

    // Column: Row
    Desktop.TableColumn {
      role: 'number'; title: "#"
      width: 30
      //delegate: Item {
      //  height: table_.cellHeight
      //  Text {
      //    anchors { fill: parent; leftMargin: table_.cellSpacing }
      //  textFormat: Text.PlainText
      //    clip: true
      //    verticalAlignment: Text.AlignVCenter
      //    text: itemValue
      //    color: itemSelected ? 'white' : editable ? 'green' : 'black'
      //    font.strikeout: itemValue.disabled
      //  }
      //}
    }

    // Column: ID
    Desktop.TableColumn {
      role: 'object'; title: "ID" // Sk.tr("ID")
      // role: 'id'; title: "ID" // Sk.tr("ID")
      width: 47
      delegate: Text {
        //anchors { fill: parent; leftMargin: table_.cellSpacing }
        height: table_.cellHeight
        textFormat: itemValue.id == 0 ? Text.RichText : Text.PlainText
        clip: true
        verticalAlignment: Text.AlignVCenter
        color: (itemSelected || itemValue.id == 0) ? 'white' : itemColor(itemValue)
        font.strikeout: !itemSelected && itemValue.disabled
        font.bold: itemValue.regex || itemValue.syntax
        text: itemValue.id == 0 ? root_._UNSAVED_RICH_TEXT : String(itemValue.id)
      }
    }

    // Column: error
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Check") // Sk.tr("ID")
      // role: 'id'; title: "ID" // Sk.tr("ID")
      width: 40
      delegate: Text {
        //anchors { fill: parent; leftMargin: table_.cellSpacing }
        height: table_.cellHeight
        //textFormat: itemValue.id == 0 ? Text.PlainText : Text.RichText
        textFormat: Text.PlainText
        clip: true
        verticalAlignment: Text.AlignVCenter
        color: itemSelected ? 'white' : root_.getErrorColor(itemValue.errorType)
        font.strikeout: !itemSelected && itemValue.disabled
        font.bold: itemValue.regex || itemValue.syntax
        text: root_.getErrorText(itemValue.errorType)
      }
    }

    // Column: Disabled
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Enable")
      width: 45
      delegate: Item {
        height: table_.cellHeight
        Desktop.CheckBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          enabled: canImprove(itemValue)
          checked: !itemValue.disabled
          onCheckedChanged:
            if (enabled && checked === itemValue.disabled)
              datamanPlugin_.enableTerm(itemValue, checked)
              //checked = !itemValue.disabled

          function setNotChecked(t) { checked = !t }
          Component.onCompleted: itemValue.disabledChanged.connect(setNotChecked)
          Component.onDestruction: itemValue.disabledChanged.disconnect(setNotChecked)
        }
      }
    }

    // Column: Game specific
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Private")
      //width: 40
      width: 40
      delegate: Item {
        height: table_.cellHeight
        Desktop.CheckBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          enabled: canEdit(itemValue) && itemValue.userId !== _GUEST_USER_ID
          checked: itemValue.private_
          onCheckedChanged:
            if (enabled && checked !== itemValue.private_)
              itemValue.private_ = checked
        }
      }
    }

    // Column: Type
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Type")
      width: 60
      delegate: Item {
        height: table_.cellHeight
        property bool editable: canEdit(itemValue)
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter
          text: {
            switch (itemValue.type) {
            case 'escape': return Sk.tr("Translation")
            case 'source': return My.tr("Input")
            case 'target': return My.tr("Output")
            case 'name': return My.tr("Name")
            case 'yomi': return My.tr("Yomi")
            case 'title': return My.tr("Suffix")
            case 'origin': return Sk.tr("Game")
            case 'speech': return My.tr("TTS")
            case 'ocr': return My.tr("OCR")
            case 'macro': return Sk.tr("Macro")
            default: return Sk.tr("Translation")
            }
          }
          visible: !itemSelected || !editable
          color: itemSelected ? 'white' : itemColor(itemValue)
          font.strikeout: itemValue.disabled
          font.bold: itemValue.regex || itemValue.syntax
        }
        Desktop.ComboBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          model: ListModel { //id: typeModel_
            Component.onCompleted: {
              append({value:'escape', text:Sk.tr("Translation")})
              append({value:'source', text:My.tr("Input")})
              append({value:'target', text:My.tr("Output")})
              append({value:'name', text:My.tr("Name")})
              append({value:'yomi', text:My.tr("Yomi")})
              append({value:'title', text:My.tr("Suffix")})
              append({value:'origin', text:Sk.tr("Game")})
              append({value:'speech', text:My.tr("TTS")})
              append({value:'ocr', text:My.tr("OCR")})
              append({value:'macro', text:Sk.tr("Macro")})
            }
          }

          tooltip: Sk.tr("Type")
          visible: itemSelected && editable

          onSelectedIndexChanged:
            if (editable) {
              var t = model.get(selectedIndex).value
              if (t !== itemValue.type) {
                itemValue.type = t
                if (t === 'macro' && !itemValue.regex)
                  itemValue.regex = true
                itemValue.updateUserId = root_.userId
                itemValue.updateTimestamp = Util.currentUnixTime()
              }
            }

          selectedText: model.get(selectedIndex).text
          Component.onCompleted: {
            switch (itemValue.type) { // Must be consistent with the model
            case 'escape': selectedIndex = 0; break
            case 'source': selectedIndex = 1; break
            case 'target': selectedIndex = 2; break
            case 'name': selectedIndex = 3; break
            case 'yomi': selectedIndex = 4; break
            case 'title': selectedIndex = 5; break
            case 'origin': selectedIndex = 6; break
            case 'speech': selectedIndex = 7; break
            case 'ocr': selectedIndex = 8; break
            case 'macro': selectedIndex = 9; break
            default: selectedIndex = 0 // this should never happend
            }
          }
        }
      }
    }

    // Column: Language
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Language")
      width: 40
      delegate: Item {
        height: table_.cellHeight
        property bool editable: canEdit(itemValue)
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter

          text: Sk.tr(itemValue.language === 'ja' ? "all" : itemValue.language)

          visible: !itemSelected || !editable
          color: itemSelected ? 'white' : itemColor(itemValue)
          font.strikeout: itemValue.disabled
          font.bold: itemValue.regex || itemValue.syntax
        }
        Desktop.ComboBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          model: ListModel { id: languageModel_
            Component.onCompleted: {
              for (var i in Util.LANGUAGES) {
                var lang = Util.LANGUAGES[i]
                append({
                  value: lang
                  , text: Sk.tr(lang === 'ja' ? "all" : lang)
                })
              }
            }
          }

          tooltip: My.tr("Language")
          visible: itemSelected && editable

          onSelectedIndexChanged:
            if (editable) {
              var t = model.get(selectedIndex).value
              if (t !== itemValue.language) {
                itemValue.language = t
                itemValue.updateUserId = root_.userId
                itemValue.updateTimestamp = Util.currentUnixTime()
              }
            }

          selectedText: model.get(selectedIndex).text

          Component.onCompleted: {
            for (var i = 0; i < model.count; ++i)
              if (model.get(i).value === itemValue.language)
                selectedIndex = i
          }
        }
      }
    }

    // Column: Syntax
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Syntax")
      width: 40
      delegate: Item {
        height: table_.cellHeight
        Desktop.CheckBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          enabled: canEdit(itemValue) && itemValue.type === 'escape' && root_.userId !== _GUEST_USER_ID // only escape syntax is allowed
          checked: itemValue.syntax && itemValue.type === 'escape' // force syntax for translatoin
          onCheckedChanged:
            if (enabled && checked !== itemValue.syntax)
              itemValue.syntax = checked
        }
      }
    }

    // Column: Regex
    Desktop.TableColumn {
      role: 'object'; title: qsTr("Regex")
      width: 40
      delegate: Item {
        height: table_.cellHeight
        Desktop.CheckBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          //enabled: canEdit(itemValue) && itemValue.type !== 'title' && itemValue.type !== 'macro' && !itemValue.syntax // prevent from using regex
          enabled: canEdit(itemValue) && itemValue.type !== 'macro' && !itemValue.syntax // prevent from using regex
          checked: itemValue.regex || itemValue.type === 'macro' // force regex for macros
          onCheckedChanged:
            if (enabled && checked !== itemValue.regex)
              itemValue.regex = checked
        }
      }
    }

/*
    // Column: IgnoreCase
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Ignore case")
      width: 40
      delegate: Item {
        height: table_.cellHeight
        Desktop.CheckBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          enabled: canEdit(itemValue)
          checked: itemValue.ignoresCase
          onCheckedChanged:
            if (enabled && checked !== itemValue.ignoresCase)
              itemValue.ignoresCase = checked
        }
      }
    }
*/
/*
    // Column: BBCode
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("BBCode")
      width: 40
      delegate: Item {
        height: table_.cellHeight
        Desktop.CheckBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          enabled: canEdit(itemValue)
          checked: itemValue.bbcode
          onCheckedChanged:
            if (enabled && checked !== itemValue.bbcode)
              itemValue.bbcode = checked
        }
      }
    }
*/

    // Column: Game specific
    Desktop.TableColumn {
      role: 'object'; title: "Hentai" // My.tr("Hentai")
      width: 40
      delegate: Item {
        height: table_.cellHeight
        Desktop.CheckBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          enabled: canEdit(itemValue)
          checked: itemValue.hentai
          onCheckedChanged:
            if (enabled && checked !== itemValue.hentai)
              itemValue.hentai = checked
        }
      }
    }

    // Column: Game specific
    Desktop.TableColumn {
      role: 'object'; title: qsTr("Series")
      width: 40
      delegate: Item {
        height: table_.cellHeight
        Desktop.CheckBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          enabled: !!Number(itemValue.gameId) && canEdit(itemValue)
          checked: itemValue.special && !!Number(itemValue.gameId)
          onCheckedChanged:
            if (enabled && checked !== itemValue.special)
              itemValue.special = checked
        }
      }
    }

    // Column: Game
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Game")
      width: 120
      delegate: Item {
        height: table_.cellHeight
        property bool editable: canEdit(itemValue)
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter
          text: gameSummary(itemValue.gameId)
          visible: !itemSelected || !editable
          color: itemSelected ? 'white' : itemColor(itemValue)
          font.strikeout: itemValue.disabled
          font.bold: itemValue.regex || itemValue.syntax
        }
        Desktop.ComboBox {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          model: ListModel { id: model_
            Component.onCompleted: {
              append({value:0, text:Sk.tr("Clear")})
              append({value:-1, text:My.tr("Current game")})
            }
            function containsValue(value) {
              for (var i = 0; i < model.count; ++i)
                if (model.get(i).value === value)
                  return true
              return false
            }
          }
          tooltip: My.tr("Game")
          visible: itemSelected && editable

          selectedText: model.get(selectedIndex).text

          onSelectedIndexChanged:
            if (editable) {
              // Use implicit conversion (==)
              var gameId = model.get(selectedIndex).value
              if (gameId === -1) {
                gameId = Number(datamanPlugin_.currentGameId())
                if (gameId === 0) {
                  //growlPlugin_.warn(qsTr("No running game"))
                  refresh()
                  return
                }
                if (!model_.containsValue(gameId))
                  model.append({value:gameId, text:gameSummary(gameId)})
              }
              if (gameId >= 0 && gameId !== Number(itemValue.gameId)) {
                itemValue.gameId = gameId
                itemValue.updateUserId = root_.userId
                itemValue.updateTimestamp = Util.currentUnixTime()
                refresh()
              }
            }

          Component.onCompleted: {
            var gameId = Number(itemValue.gameId)
            if (gameId)
              model.append({value:gameId, text:gameSummary(gameId)})
            refresh()
          }
          function refresh() {
            var gameId = Number(itemValue.gameId)
             for (var i = 0; i < model.count; ++i)
               if (model.get(i).value === gameId)
                 if (selectedIndex !== i)
                   selectedIndex = i
          }
        }
      }
    }


    // Column: Pattern
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Pattern")
      width: 120
      delegate: Item {
        height: table_.cellHeight
        property bool editable: canEdit(itemValue)
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter
          visible: !itemSelected //|| !editable
          text: itemValue.pattern
          color: itemSelected ? 'white' : itemColor(itemValue)
          font.strikeout: !itemSelected && itemValue.disabled
          font.bold: itemValue.regex || itemValue.syntax
        }
        TextInput {
          anchors { fill: parent; leftMargin: table_.cellSpacing; topMargin: 6 }
          color: valid ? 'white' : 'red'
          selectByMouse: true
          selectionColor: 'white'
          selectedTextColor: 'black'
          visible: itemSelected //&& editable
          readOnly: !editable
          maximumLength: _MAX_TEXT_LENGTH
          //font.bold: itemValue.regex

          property bool valid: Util.trim(text).length >= _MIN_TEXT_LENGTH

          Component.onCompleted: text = itemValue.pattern

          onTextChanged: save()
          onAccepted: save()
          function save() {
            if (editable) {
              var t = Util.trim(text)
              if (t && t !== itemValue.pattern && valid) {
                itemValue.pattern = t
                itemValue.updateUserId = root_.userId
                itemValue.updateTimestamp = Util.currentUnixTime()
              }
            }
          }
        }
      }
    }

    // Column: Text
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Translation")
      width: 90
      delegate: Item {
        height: table_.cellHeight
        property bool editable: canEdit(itemValue)
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter
          visible: !itemSelected //|| !editable
          text: itemValue.text
          color: itemSelected ? 'white' : itemColor(itemValue)
          font.strikeout: !itemSelected && itemValue.disabled
          font.bold: itemValue.regex || itemValue.syntax
        }
        TextInput {
          anchors { fill: parent; leftMargin: table_.cellSpacing; topMargin: 6 }
          color: 'white'
          selectByMouse: true
          selectionColor: 'white'
          selectedTextColor: 'black'
          visible: itemSelected //&& editable
          readOnly: !editable
          maximumLength: _MAX_TEXT_LENGTH
          //property bool valid: Util.trim(text).length >= _MIN_TEXT_LENGTH
          //font.bold: itemValue.regex || itemValue.syntax

          Component.onCompleted: text = itemValue.text

          onTextChanged: save()
          onAccepted: save()
          function save() {
            if (editable) {
              var t = Util.trim(text)
              if (t !== itemValue.text) {
                itemValue.text = t
                itemValue.updateUserId = root_.userId
                itemValue.updateTimestamp = Util.currentUnixTime()
              }
            }
          }
        }
      }
    }

    // Column: Comment
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Comment")
      width: 60
      delegate: Item {
        height: table_.cellHeight
        property bool editable: canEdit(itemValue)
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter
          visible: !itemSelected //|| !editable
          text: itemValue.comment
          color: itemSelected ? 'white' : commentColor(itemValue)
          //font.strikeout: !itemSelected && itemValue.disabled
          //font.bold: itemValue.regex || itemValue.syntax

          //function setText(v) { text = v }
          //Component.onCompleted: itemValue.commentChanged.connect(setText)
          //Component.onDestruction: itemValue.commentChanged.disconnect(setText)
        }
        TextInput {
          anchors { fill: parent; leftMargin: table_.cellSpacing; topMargin: 6 }
          color: 'white'
          selectByMouse: true
          selectionColor: 'white'
          selectedTextColor: 'black'
          visible: itemSelected //&& editable
          readOnly: !editable
          maximumLength: _MAX_TEXT_LENGTH

          text: itemValue.comment

          onTextChanged: save()
          onAccepted: save()
          function save() {
            if (editable) {
              var t = Util.trim(text)
              if (t !== itemValue.comment) {
                itemValue.comment = t
                itemValue.updateUserId = root_.userId
                itemValue.updateTimestamp = Util.currentUnixTime()
              }
            }
          }

          //Component.onCompleted: itemValue.commentChanged.connect(setText)
          //Component.onDestruction: itemValue.commentChanged.disconnect(setText)
          //function setText(v) { // string ->
          //  v = Util.trim(v)
          //  if (text !== v)
          //    text = v
          //}
        }
      }
    }

    // Column: User Name
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Creator")
      width: 80
      delegate: Item {
        height: table_.cellHeight
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter
          text: '@' + itemValue.userName
          color: itemSelected ? 'white' : _READONLY_COLOR
        }
      }
    }

    // Column: Timestamp
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Creation time")
      width: 120
      delegate: Item {
        height: table_.cellHeight
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter
          text: Util.timestampToString(itemValue.timestamp)
          color: itemSelected ? 'white' : _READONLY_COLOR
        }
      }
    }

    // Column: Update User Name
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Update")
      width: 80
      delegate: Item {
        height: table_.cellHeight
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter
          text: itemValue.updateUserId ? '@' + itemValue.updateUserName : ""
          color: itemSelected ? 'white' : _READONLY_COLOR
        }
      }
    }

    // Column: Timestamp
    Desktop.TableColumn {
      role: 'object'; title: Sk.tr("Update time")
      width: 120
      delegate: Item {
        height: table_.cellHeight
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter
          text: itemValue.updateTimestamp > 0 ? Util.timestampToString(itemValue.updateTimestamp) : ''
          color: itemSelected ? 'white' : _READONLY_COLOR
        }
      }
    }

    // Column: Update Comment

    Desktop.TableColumn {
      role: 'object'; title: My.tr("Update reason")
      width: 200
      delegate: Item {
        height: table_.cellHeight
        property bool editable: itemValue.updateUserId === root_.userId
        Text {
          anchors { fill: parent; leftMargin: table_.cellSpacing }
          textFormat: Text.PlainText
          clip: true
          verticalAlignment: Text.AlignVCenter
          visible: !itemSelected //|| !editable
          text: itemValue.updateComment
          color: itemSelected ? 'white' : updateCommentColor(itemValue)
          //font.strikeout: itemValue.disabled
          //font.bold: itemValue.regex || itemValue.syntax
        }
        TextInput {
          anchors { fill: parent; leftMargin: table_.cellSpacing; topMargin: 6 }
          color: 'white'
          selectByMouse: true
          selectionColor: 'white'
          selectedTextColor: 'black'
          visible: itemSelected //&& editable
          readOnly: !editable
          maximumLength: _MAX_TEXT_LENGTH

          // The updateComment could be modified by dataman
          //Component.onCompleted: text = itemValue.updateComment
          text: itemValue.updateComment

          onTextChanged: save()
          onAccepted: save()
          function save() {
            if (editable) {
              var t = Util.trim(text)
              if (t !== itemValue.updateComment) {
                itemValue.updateComment = t
                itemValue.updateUserId = root_.userId
                itemValue.updateTimestamp = Util.currentUnixTime()
              }
            }
          }
        }
      }
    }

    // New Column
  }

  //function enableTerm(term) {
  //  term.disabled = false
  //  term.updateUserId = userId
  //  term.updateTimestamp = Util.currentUnixTime()
  //  return true
  //}

  //function disableTerm(term) {
  //  return datamanPlugin_.disableTerm(term)
  //}
}
