/** popupmanager.qml
 *  8/16/2014 jichi
 *  Text popup manager for OCR.
 */
import QtQuick 1.1
import org.sakuradite.reader 1.0 as Plugin
import '.' as Kagami

Item { id: root_

  property real zoomFactor: 1.0
  property bool ignoresFocus: false

  // - Private -

  Plugin.PopupBean { //id: bean_
    Component.onCompleted:
      popupRequested.connect(showPopup)
  }

  Kagami.PopupComponent { id: comp_ }

  function showPopup(text, posX, posY) { // string ->
    var popup = comp_.createObject(root_, {
      x: posX
      , y: posY
      //, width: 640
      //, height: 480
      , zoomFactor: root_.zoomFactor
      , ignoresFocus: root_.ignoresFocus
      , minimumX: x
      , maximumX: x + width
      , minimumY: y
      , maximumY: y + height
      , text: text
    })
    popup.maximumX -= popup.width
    popup.maximumY -= popup.height

    popup.closeRequested.connect(popup.destroy)
  }
}
