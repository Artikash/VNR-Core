/** popupmanager.qml
 *  8/16/2014 jichi
 *  Text popup manager for OCR.
 */
import QtQuick 1.1
import org.sakuradite.reader 1.0 as Plugin
import '.' as Kagami

Item { id: root_

  property real zoomFactor: 1.0

  // - Private -

  Plugin.PopupBean { //id: bean_
    Component.onCompleted:
      popupRequested.connect(showPopup)
  }

  Kagami.PopupComponent { id: comp_ }

  function showPopup(text, x, y) { // string ->
    var popup = comp_.createObject(root_, {
      x: x
      , y: y
      //, width: 640
      //, height: 480
      , zoomFactor: root_.zoomFactor
      , minimumX: x
      , maximumX: x + width
      , minimumY: y
      , maximumY: y + height
      , text: text
    })

    popup.closeRequested.connect(popup.destroy)
  }
}
