/** popupcomponent.qml
 *  8/16/2014 jichi
 *  Component to create OCR text popup.
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
//import org.sakuradite.reader 1.0 as Plugin
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/reader.min.js' as My
import '../../../js/util.min.js' as Util
import '../../../components' as Components
import '../share' as Share

Component { //id: root_

  Rectangle { id: root_
    property int minimumX
    property int minimumY
    property int maximumX
    property int maximumY

    signal closeRequested()

    property alias text: textEdit_.text
    property real zoomFactor: 1.0
    property bool ignoresFocus: false // used to disable context menu, not implemented though

    // - Private -

    Component.onCompleted: {
      console.log("popupcomponent.qml:onCompleted: pass")
      ensureVisible()
    }

    Component.onDestruction: console.log("popupcomponent.qml:onDestruction: pass")

    radius: 10
    color: '#99000000' // black

    property int _CONTENT_MARGIN: 10

    width: scrollArea_.width + _CONTENT_MARGIN * 2
    height: scrollArea_.height + _CONTENT_MARGIN * 2

    property int _MAX_WIDTH: 200 * zoomFactor
    property int _MAX_HEIGHT: 200 * zoomFactor

    function close() {
      visible = false
      closeRequested()
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

    function translate() {
      var text = textEdit_.text
      if (text) {
        var keys = trPlugin_.translators().split(',')
        if (keys.length)
          textEdit_.textFormat = TextEdit.RichText
          for (var i in keys) {
            var key = keys[i]
            var tr = trPlugin_.translate(text, key)
            if (tr && tr != text)
              appendTranslation(tr, key)

            var name = My.tr(Util.translatorName(key))
            if (i == 0)
              toolTip_.text = name
            else
              toolTip_.text += ", " + name
          }
      }
    }

    function appendTranslation(tr, key) { // translator key, translation text
      var text = textEdit_.text
      if (text)
        text += '<br/>'
      text += tr
      textEdit_.text = text
    }

    MouseArea {
      anchors.fill: parent
      acceptedButtons: Qt.LeftButton
      drag {
        target: root_
        axis: Drag.XandYAxis

        minimumX: root_.minimumX; minimumY: root_.minimumY
        maximumX: root_.maximumX; maximumY: root_.maximumY
      }
    }

    Desktop.TooltipArea { id: toolTip_
      anchors.fill: parent
      text: qsTr("You can drag the border to move the text box")
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
        width: _MAX_WIDTH // FIXME: automatically adjust width

        //selectByMouse: true // conflicts with flickable

        textFormat: TextEdit.PlainText
        wrapMode: TextEdit.Wrap
        focus: true
        color: 'snow'
        font.pixelSize: 12 * root_.zoomFactor
        //font.bold: true
        //font.family: 'MS Mincho' // 明朝

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

          // Disabled since TTS does not work in admin
          //onDoubleClicked: {
          //  textEdit_.cursorPosition = textEdit_.positionAt(mouse.x, mouse.y)
          //  textEdit_.selectWord()
          //  var t = textEdit_.selectedText
          //  if (t) {
          //    clipboardPlugin_.text = t
          //    ttsPlugin_.speak(t, 'ja')
          //  }
          //}
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

    Desktop.ContextMenu { id: contextMenu_
      //property int popupX
      //property int popupY

      function popup(x, y) {
        //popupX = x; popupY = y
        showPopup(x, y)
      }

      Desktop.MenuItem { id: copyAct_
        text: Sk.tr("Copy")
        shortcut: "Ctrl+C"
        onTriggered: {
          textEdit_.selectAll()
          textEdit_.copy()
        }
      }

      Desktop.Separator {}

      Desktop.MenuItem {
        text: Sk.tr("Close")
        //shortcut: "Esc"
        onTriggered: root_.close()
      }
    }

    MouseArea { id: mouse_
      anchors.fill: parent
      //hoverEnabled: true
      acceptedButtons: Qt.RightButton
      //onEntered: root_.show()
      //onExited: root_.show() // bypass restart timer issue
      onPressed: if (!root_.ignoresFocus) {
        //var gp = Util.itemGlobalPos(parent)
        var gp = mapToItem(null, x + mouse.x, y + mouse.y)
        contextMenu_.popup(gp.x, gp.y)
      }
    }

    Rectangle { id: header_
      anchors {
        left: parent.left
        bottom: parent.top
        //bottomMargin: -_MARGIN*2
      }
      radius: 7

      property int _MARGIN: 2
      width: headerRow_.width + _MARGIN * 2 + _MARGIN * 8
      height: headerRow_.height + _MARGIN * 2

      color: root_.color

      MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        drag {
          target: root_
          axis: Drag.XandYAxis

          minimumX: root_.minimumX; minimumY: root_.minimumY
          maximumX: root_.maximumX; maximumY: root_.maximumY
        }
      }

      Row { id: headerRow_
        //anchors.centerIn: parent
        anchors {
          verticalCenter: parent.verticalCenter
          left: parent.left
          leftMargin: header_._MARGIN
        }

        spacing: header_._MARGIN * 2

        property int cellWidth: 15
        property int pixelSize: 10
        property color backgroundColor

        Share.CircleButton {
          diameter: parent.cellWidth
          font.pixelSize: parent.pixelSize
          font.bold: hover
          font.family: 'MS Gothic'
          backgroundColor: 'transparent'

          text: "×" // ばつ
          toolTip: Sk.tr("Close")
          onClicked: root_.close()
        }

        Share.CloseButton {
          diameter: parent.cellWidth
          font.pixelSize: parent.pixelSize
          //font.bold: hover    // bold make the text too bold
          font.family: 'MS Gothic'
          backgroundColor: 'transparent'

          color: enabled ? 'snow' : 'gray'
          text: '訳'
          toolTip: enabled ? Sk.tr("Translate") : qsTr('Translating') + '...'
          onClicked:
            if (enabled && textEdit_.text) {
              enabled = false
              toolTip_.text = qsTr('Translating') + '...'
              root_.translate()
            }
        }
      }
    }
  }
}
