/** popupcomponent.qml
 *  8/16/2014 jichi
 *  Component to create OCR text popup.
 */
import QtQuick 1.1
//import org.sakuradite.reader 1.0 as Plugin
import '../../../js/sakurakit.min.js' as Sk
import '../../../components' as Components
import '../share' as Share

Component { //id: root_

  Item { id: root_

    property string text
    property real zoomFactor: 1.0

    // - Private -

    property int _MAX_WIDTH: 300 * zoomFactor
    property int _MAX_HEIGHT: 200 * zoomFactor

    width: scrollArea_.width
    height: scrollArea_.height

    Flickable { id: scrollArea_

      Component.onCompleted: console.log("popupcomponent.qml:onCompleted: pass")
      Component.onDestruction: console.log("popupcomponent.qml:onDestruction: pass")

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

        textFormat: TextEdit.RichText
        wrapMode: TextEdit.Wrap
        focus: true
        color: 'snow'
        font.pixelSize: 12 * root_.zoomFactor
        font.bold: true
        font.family: 'MS Mincho' // 明朝

        text: "hello wwwwwworld"

        // Not working, which cause textedit width to shrink
        //onTextChanged: width = Math.min(_MAX_WIDTH, paintedWidth)

        onLinkActivated: Qt.openUrlExternally(link)
        //console.log("shiori.qml: link activated:", link)

        effect: Share.TextEffect { }

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
  }
}
