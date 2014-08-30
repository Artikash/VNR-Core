/** shiori.qml
 *  11/25/2012 jichi
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import org.sakuradite.reader 1.0 as Plugin
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/reader.min.js' as My
//import '../../../js/util.min.js' as Util
import '../../../components' as Components
import '../share' as Share

Rectangle { id: root_

  property int minimumX
  property int minimumY
  property int maximumX
  property int maximumY

  property bool ignoresFocus: false
  property bool toolTipEnabled: true

  signal yakuAt(string text, int x, int y) // popup honyaku of text at (x, y)

  property real globalZoomFactor: 1.0

  property real zoomFactor: 1.0

  property int defaultWidth //: textEdit_.width / (zoomFactor * globalZoomFactor)

  // - Private -

  property int _RESIZABLE_AREA_WIDTH: 20 // resizable mouse area thickness

  property bool enabled: bean_.enabled // cached

  //clip: true // to hide scroll bars, but the close button will also be clipped

  property int _VISIBLE_DURATION: 4000   // 4 seconds
  property int _X_OFFSET: 20
  property int _Y_OFFSET: 15

  property int _MAX_HEIGHT: 200 * zoomFactor * globalZoomFactor

  //property int _DEFAULT_WIDTH: 300 * zoomFactor
  property int _MIN_WIDTH: 50 * zoomFactor * globalZoomFactor
  property int _MAX_WIDTH: 800 * zoomFactor * globalZoomFactor

  property int _CONTENT_MARGIN: 10

  width: scrollArea_.width + _CONTENT_MARGIN * 2
  height: scrollArea_.height + _CONTENT_MARGIN * 2

  radius: 10
  opacity: 0 // initial opacity is zero

  property real zoomStep: 0.1
  property real minZoomFactor: 0.5
  property real maxZoomFactor: 5.0

  function zoomIn() {
    var v = zoomFactor + zoomStep
    if (v < maxZoomFactor)
      zoomFactor = v
  }
  function zoomOut() {
    var v = zoomFactor - zoomStep
    if (v > minZoomFactor)
      zoomFactor = v
  }

  //gradient: Gradient {  // color: aarrggbb
  //  GradientStop { position: 0.0;  color: '#ec8f8c8c' }
  //  GradientStop { position: 0.17; color: '#ca6a6d6a' }
  //  GradientStop { position: 0.77; color: '#9f3f3f3f' }
  //  GradientStop { position: 1.0;  color: '#ca6a6d6a' }
  //}
  color: '#aa000000'

  Plugin.ShioriBean { id: bean_
    Component.onCompleted:
      bean_.popup.connect(root_.popup)
  }

  //Plugin.Tts { id: ttsPlugin_ }
  //Plugin.ClipboardProxy { id: clipboardPlugin_ }

  //Rectangle { // border
  //  anchors {
  //    fill: parent
  //    margins: _CONTENT_MARGIN / 4
  //  }
  //  color: 'transparent'
  //  border.color: '#55000000' // black
  //  border.width: _CONTENT_MARGIN / 3
  //  radius: parent.radius
  //}

  property bool hover: toolTip_.containsMouse || header_.hover

  Desktop.TooltipArea { id: toolTip_
    anchors.fill: parent
    text: root_.toolTipEnabled ? qsTr("You can drag the border to move the text box") : ''
  }

  MouseArea { // draggable area
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton
    drag {
      target: root_
      axis: Drag.XandYAxis

      minimumX: root_.minimumX; minimumY: root_.minimumY
      maximumX: root_.maximumX; maximumY: root_.maximumY
    }
  }

  MouseArea { // left draggable area
    anchors {
      top: parent.top; bottom: parent.bottom
      left: parent.left
    }
    width: _RESIZABLE_AREA_WIDTH
    acceptedButtons: Qt.LeftButton

    property int pressedX
    onPressed: pressedX = mouseX
    onPositionChanged:
      if (pressed) {
        var dx = mouseX - pressedX
        var w = textEdit_.width - dx
        if (w > _MIN_WIDTH && w < _MAX_WIDTH) {
          root_.defaultWidth = w / (root_.zoomFactor * root_.globalZoomFactor)
          root_.x += dx
        }
      }

    Desktop.TooltipArea { id: leftResizeTip_
      anchors.fill: parent
      text: Sk.tr("Resize")
    }
  }

  MouseArea { // right draggable area
    anchors {
      top: parent.top; bottom: parent.bottom
      right: parent.right
    }
    width: _RESIZABLE_AREA_WIDTH
    acceptedButtons: Qt.LeftButton

    property int pressedX
    onPressed: pressedX = mouseX
    onPositionChanged:
      if (pressed) {
        var w = textEdit_.width + mouseX - pressedX
        if (w > _MIN_WIDTH && w < _MAX_WIDTH)
          root_.defaultWidth = w / (root_.zoomFactor * root_.globalZoomFactor)
      }

    Desktop.TooltipArea { id: rightResizeTip_
      anchors.fill: parent
      text: Sk.tr("Resize")
    }
  }

  function scrollTop() {
    scrollArea_.contentY = 0
  }

  Flickable { id: scrollArea_
    anchors.centerIn: parent
    height: Math.min(_MAX_HEIGHT, textEdit_.paintedHeight)
    width: textEdit_.width

    //contentWidth: textEdit_.paintedWidth
    contentHeight: textEdit_.paintedHeight
    clip: true

    states: State {
      when: scrollArea_.movingVertically || scrollArea_.movingHorizontally
      PropertyChanges { target: verticalScrollBar_; opacity: 1 }
      //PropertyChanges { target: horizontalScrollBar_; opacity: 1 }
    }

    transitions: Transition {
      NumberAnimation { property: 'opacity'; duration: 400 }
    }

    TextEdit { id: textEdit_
      anchors.centerIn: parent
      width: root_.defaultWidth * root_.zoomFactor * root_.globalZoomFactor
      //width: _DEFAULT_WIDTH // FIXME: automatically adjust width

      //selectByMouse: true // conflicts with flickable

      textFormat: TextEdit.RichText
      wrapMode: TextEdit.Wrap
      focus: true
      color: 'snow'
      font.pixelSize: 12 * root_.zoomFactor * root_.globalZoomFactor
      font.bold: true
      font.family: 'MS Mincho' // 明朝

      // Not working, which cause textedit width to shrink
      //onTextChanged: width = Math.min(_MAX_WIDTH, paintedWidth)

      onLinkActivated: Qt.openUrlExternally(link)
      //console.log("shiori.qml: link activated:", link)

      effect: Share.TextEffect {}

      MouseArea { //id: textCursor_
        anchors.fill: parent
        //acceptedButtons: enabled ? Qt.LeftButton : Qt.NoButton
        acceptedButtons: Qt.LeftButton
        //enabled: !!model.text
        //hoverEnabled: enabled

        onDoubleClicked: {
          textEdit_.cursorPosition = textEdit_.positionAt(mouse.x, mouse.y)
          textEdit_.selectWord()
          var t = textEdit_.selectedText
          if (t) {
            clipboardPlugin_.text = t
            ttsPlugin_.speak(t, 'ja')
          }
        }
      }
    }
  }

  Components.ScrollBar { id: verticalScrollBar_
    width: 12
    height: Math.max(0, scrollArea_.height - 12)
    anchors.right: scrollArea_.right
    anchors.verticalCenter: scrollArea_.verticalCenter
    opacity: 0
    orientation: Qt.Vertical
    position: scrollArea_.visibleArea.yPosition
    pageSize: scrollArea_.visibleArea.heightRatio
  }

  //Components.ScrollBar { id: horizontalScrollBar_
  //  width: Math.max(0, scrollArea_.width - 12)
  //  height: 12
  //  anchors.bottom: scrollArea_.bottom
  //  anchors.horizontalCenter: scrollArea_.horizontalCenter
  //  opacity: 0
  //  orientation: Qt.Horizontal
  //  position: scrollArea_.visibleArea.xPosition
  //  pageSize: scrollArea_.visibleArea.widthRatio
  //}

  //NumberAnimation on opacity { id: fadeOutAni_
  NumberAnimation { id: fadeOutAni_
    target: root_; property: 'opacity'
    to: 0; duration: 1000
  }

  Timer { id: visibleTimer_
    interval: _VISIBLE_DURATION
    onTriggered: root_.hide()
  }

  function show() {
    scrollTop()
    opacity = 1
    visibleTimer_.restart()
    //console.log("shiori.qml: showing")
  }

  function hide() {
    if (!autoHideAct_.checked || root_.hover)
      show() // QtBUG: cannot restart timer within onTriggered, see: http://comments.gmane.org/gmane.comp.lib.qt.qml/3085
    else {
      //console.log("shiori.qml: hiding")
      fadeOutAni_.start()
    }
  }

  function hideNow() {
    //autoHideAct_.checked = true
    visibleTimer_.stop()
    fadeOutAni_.stop()
    opacity = 0
  }

  function reset() {
    //textEdit_.width = _DEFAULT_WIDTH
  }

  function popup(text, x, y) {
    if (!root_.enabled)
      return
    reset()
    root_.x = x + _X_OFFSET; root_.y = y + _Y_OFFSET
    textEdit_.text = bean_.render(text)
    ensureVisible()
    show()
  }

  // So that the popup will not be out of screen
  function ensureVisible() {
    if (x < minimumX)
      x = minimumX
    if (y < minimumY)
      y = minimumY
    if (x > maximumX)
      x = maximumX
    if (y > maximumY)
      y = maximumY
  }

  // - Context Menu -

  function hoverText() {
    var pos = textEdit_.mapFromItem(null,
        contextMenu_.popupX, contextMenu_.popupY)
    textEdit_.cursorPosition = textEdit_.positionAt(pos.x, pos.y)
    textEdit_.selectWord()
    return textEdit_.selectedText
  }

  Desktop.ContextMenu { id: contextMenu_
    property int popupX
    property int popupY

    function popup(x, y) {
      popupX = x; popupY = y
      showPopup(x, y)
    }

    Desktop.MenuItem { id: copyAct_
      text: Sk.tr("Copy")
      shortcut: "Ctrl+C"
      onTriggered:
        if (hoverText())
          textEdit_.copy()
    }

    Desktop.MenuItem {
      text: Sk.tr("Copy All")
      //shortcut: "Ctrl+A"
      onTriggered: {
        textEdit_.selectAll()
        textEdit_.copy()
      }
    }

    Desktop.MenuItem {
      text: Sk.tr("Select Word")
      //shortcut: "Ctrl+A"
      onTriggered: {
        var pos = textEdit_.mapFromItem(null,
            contextMenu_.popupX, contextMenu_.popupY)
        textEdit_.cursorPosition = textEdit_.positionAt(pos.x, pos.y)
        textEdit_.selectWord()
      }
    }

    Desktop.MenuItem {
      text: Sk.tr("Select All")
      shortcut: "Ctrl+A"
      onTriggered: textEdit_.selectAll()
    }

    Desktop.MenuItem {
      text: qsTr("Lookup Selection")
      //shortcut: "Ctrl+A"
      onTriggered: {
        var t = hoverText()
        if (t)
          yakuAt(t, contextMenu_.popupX, contextMenu_.popupY)
      }
    }

    Desktop.MenuItem {
      text: qsTr("Read Selection") + " (" + Sk.tr("Double-click") + ")"
      //shortcut: "Ctrl+A"
      onTriggered: {
        var t = hoverText()
        if (t)
          ttsPlugin_.speak(t, 'ja')
          //ttsPlugin_.speak(t,
          //    Util.containsLatin(textEdit_.selectedText) ? 'en' : 'ja')
      }
    }

    //Desktop.MenuItem {
    //  text: qsTr("Read All")
    //  //shortcut: "Ctrl+A"
    //  onTriggered: {
    //    textEdit_.selectAll()
    //    if (textEdit_.selectedText) {
    //      ttsPlugin_.speak(textEdit_.selectedText, 'en')
    //      console.log("shiori.qml:readAll: pass")
    //    }
    //  }
    //}

    Desktop.Separator {}

    Desktop.MenuItem { id: autoHideAct_
      text: My.tr("Auto Hide")
      //shortcut: "Esc"
      checkable: true
      checked: true
    }

    Desktop.MenuItem {
      text: Sk.tr("Hide")
      //shortcut: "Esc"
      onTriggered: root_.hideNow()
    }
  }

  MouseArea { id: mouse_
    anchors.fill: parent
    //hoverEnabled: true
    acceptedButtons: Qt.RightButton
    onEntered: root_.show()
    onExited: root_.show() // bypass restart timer issue
    onPressed: if (!root_.ignoresFocus) {
      //var gp = Util.itemGlobalPos(parent)
      var gp = mapToItem(null, x + mouse.x, y + mouse.y)
      contextMenu_.popup(gp.x, gp.y)
    }
  }

  property int _HEADER_MARGIN: 2

  Row { id: header_
    anchors { left: parent.left; top: parent.top; margins: -4 }
    spacing: _HEADER_MARGIN * 2

    property bool hover: closeButton_.hover
                      || zoomInButton_.hover
                      || zoomOutButton_.hover

    property int cellWidth: 15
    property int pixelSize: 10

    Share.CircleButton { id: closeButton_
      diameter: parent.cellWidth
      font.pixelSize: parent.pixelSize
      font.bold: hover
      font.family: 'MS Gothic'
      backgroundColor: 'transparent'

      text: "×" // ばつ
      toolTip: Sk.tr("Close")
      onClicked: root_.hideNow()
    }

    Share.CircleButton { id: zoomOutButton_
      diameter: parent.cellWidth
      font.pixelSize: parent.pixelSize
      font.bold: hover
      font.family: 'MS Gothic'
      backgroundColor: 'transparent'

      text: "-"
      toolTip: Sk.tr("Zoom out") + " " + Math.floor(root_.zoomFactor * 100) + "%"
      onClicked: root_.zoomOut()
    }

    Share.CircleButton { id: zoomInButton_
      diameter: parent.cellWidth
      font.pixelSize: parent.pixelSize
      font.bold: hover
      font.family: 'MS Gothic'
      backgroundColor: 'transparent'

      text: "+"
      toolTip: Sk.tr("Zoom in") + " " + Math.floor(root_.zoomFactor * 100) + "%"
      onClicked: root_.zoomIn()
    }
  }
}
