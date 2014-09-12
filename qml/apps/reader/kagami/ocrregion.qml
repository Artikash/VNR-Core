/** ocrregion.qml
 *  9/10/2014 jichi
 *  Region selector for OCR.
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import org.sakuradite.reader 1.0 as Plugin
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/reader.min.js' as My
import '../../../js/local.js' as Local // Local.comet
import '../share' as Share
import '.' as Kagami

Item { id: root_

  property real zoomFactor: 1.0
  property bool ignoresFocus: false

  // - Private -

  //Component { id: editComp_
  //  Kagami.OcrEdit {
  //    zoomFactor: root_._zoomFactor
  //    ignoresFocus: root_.ignoresFocus
  //  }
  //}

  Component.onCompleted: Local.items = [] // [item]

  Plugin.OcrRegionBean { //id: bean_
    Component.onCompleted:
      regionRequested.connect(root_.showRegion)
  }

  function showRegion(x, y, width, height) { // int, int, int, int, OcrImageObject, WindowObject, string, string ->
    var items = Local.items
    for (var i in items) {
      var item = items[i]
      if (!item.visible) {
        console.log("ocrregion.qml:showRegion: reuse existing item")
        item.show(x, y, width, height)
        return
      }
    }
    console.log("ocrregion.qml:showRegion: create new item")
    var item = comp_.createObject(root_)
    item.show(x, y, width, height)
    items.push(item)
  }

  // Component

  Component { id: comp_
    Rectangle { id: item_
      function show(x, y, width, height) { // int, int, int, int
        item_.x = x
        item_.y = y
        item_.width = width
        item_.height = height
      }

      // - Private -

      color: 'transparent'

      border {
        width: 10
        color: 'red'
      }
    }
  }
}
