/** popupmanager.qml
 *  8/16/2014 jichi
 *  Text popup manager for OCR.
 */
import QtQuick 1.1
//import org.sakuradite.reader 1.0 as Plugin
import '.' as Kagami

Item { id: root_

  // - Private -
  Kagami.PopupComponent { id: comp_ }

/*
  Component.onCompleted: {
    console.log(1111111)
    var popup = comp_.createObject(root_, {
      x: 100
      , y: 100
      //, width: 640
      //, height: 480
      , text: "hellllllllllo worldddddddd"
      , zoomFactor: 1.33
    })
  }
  */
}
