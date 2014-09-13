/** ocrregion.qml
 *  9/10/2014 jichi
 *  Region selector for OCR.
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import org.sakuradite.reader 1.0 as Plugin
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/local.js' as Local // Local.comet
import '../share' as Share
import '.' as Kagami

Item { id: root_

  property real zoomFactor: 1.0
  property bool ignoresFocus: false
  property bool wine: false
  property bool enabled: false
  visible: false

  // - Private -

  Component { id: editComp_
    Kagami.OcrEdit {
      zoomFactor: root_.zoomFactor
      ignoresFocus: root_.ignoresFocus
    }
  }

  Component.onCompleted: Local.items = [] // [item]

  Plugin.OcrRegionBean { id: bean_
    enabled: root_.enabled
    visible: root_.visible

    Component.onCompleted:
      regionRequested.connect(root_.showRegion)
  }

  function showRegion(globalX, globalY, width, height) { // int, int, int, int, OcrImageObject, WindowObject, string, string ->
    var pos = mapFromItem(null, globalX, globalY)

    var items = Local.items
    for (var i in items) {
      var item = items[i]
      if (!item.active) {
        console.log("ocrregion.qml:showRegion: reuse existing item")
        item.show(pos.x, pos.y, width, height)
        return
      }
    }
    console.log("ocrregion.qml:showRegion: create new item")
    var item = comp_.createObject(root_)
    item.show(pos.x, pos.y, width, height)
    items.push(item)
    bean_.addRegionItem(item)
  }

  // Component

  property int _MIN_WIDTH: _ITEM_BORDER_WIDTH * 2 + 5
  property int _ITEM_BORDER_WIDTH: 10

  Component { id: comp_
    Item { id: item_
      property bool active: true
      property bool enabled: true

      property string recognizedText // last ocr-ed text

      property QtObject imageObject // OcrImageObject

      property bool dragging:
          leftResizeArea_.pressed ||
          rightResizeArea_.pressed ||
          topResizeArea_.pressed ||
          bottomResizeArea_.pressed ||
          topLeftMoveArea_.drag.active ||
          topRightMoveArea_.drag.active ||
          bottomLeftMoveArea_.drag.active ||
          bottomRightMoveArea_.drag.active

      function show(x, y, width, height) { // int, int, int, int
        item_.x = x
        item_.y = y
        setWidth(width)
        setHeight(height)
        active = visible = true
      }

      function close() {
        active = visible = false
      }


      // - Private -

      onActiveChanged: if (!active) release()

      onVisibleChanged: if (!visible) hideEdit()

      property real normalizedWidth: 1.0
      property real normalizedHeight: 1.0
      width: normalizedWidth * root_.width
      height: normalizedHeight * root_.height

      function setWidth(v) { normalizedWidth = root_.width ? v / root_.width : 0 }
      function setHeight(v) { normalizedHeight = root_.height ? v / root_.height : 0 }

      property color borderColor: enabled ? '#aa0000ff' : '#aa800080' // blue : purple

      property bool hover: header_.hover
                        || leftTip_.containsMouse
                        || rightTip_.containsMouse
                        || topTip_.containsMouse
                        || bottomTip_.containsMouse
                        || topLeftTip_.containsMouse
                        || topRightTip_.containsMouse
                        || bottomLeftTip_.containsMouse
                        || bottomRightTip_.containsMouse

      property Item editItem
      property bool editLocked: false

      function release() {
        if (imageObject) {
          imageObject.release()
          imageObject = null
        }
      }

      function hideEdit() {
        if (editItem)
          editItem.hide()
      }

      function showEdit() {
        if (editLocked)
          return
        editLocked = true
        if (imageObject) {
          imageObject.x = x
          imageObject.y = y
          imageObject.width = width
          imageObject.height = height
          imageObject.capture()
        } else {
          console.log("ocregion.qml: create image object")
          imageObject = bean_.createImageObject(x, y, width, height)
          if (!imageObject) {
            growl_.showWarning(qsTr("Failed to capture an image for the selected region"))
            editLocked = false
            return
          }
        }
        if (!editItem)
          console.log("ocregion.qml: create ocr editor")
          editItem = editComp_.createObject(root_, {
            visible: false // hide on startup
          })

        var text = imageObject.ocr()
        editItem.x = Math.min(item_.x + 30, root_.x + root_.width - item_.width)
        editItem.y = Math.min(item_.y, root_.x + root_.height - item_.height)
        editItem.show(imageObject, text)
        editLocked = false
      }

      // Resizable

      Rectangle { id: leftBorder_
        anchors {
          top: parent.top; bottom: parent.bottom
          left: parent.left
        }
        width: _ITEM_BORDER_WIDTH
        color: item_.borderColor
      }

      Rectangle { id: rightBorder_
        anchors {
          top: parent.top; bottom: parent.bottom
          right: parent.right
        }
        width: _ITEM_BORDER_WIDTH
        color: item_.borderColor
      }

      Rectangle { id: topBorder_
        anchors {
          left: parent.left; right: parent.right
          top: parent.top
        }
        height: _ITEM_BORDER_WIDTH
        color: item_.borderColor
      }

      Rectangle { id: bottomBorder_
        anchors {
          left: parent.left; right: parent.right
          bottom: parent.bottom
        }
        height: _ITEM_BORDER_WIDTH
        color: item_.borderColor
      }

      MouseArea { id: leftResizeArea_
        anchors.fill: leftBorder_
        acceptedButtons: Qt.LeftButton

        property int pressedX
        onPressed: pressedX = mouseX
        onPositionChanged:
          if (pressed) {
            var dx = mouseX - pressedX
            var w = item_.width - dx
            if (w > _MIN_WIDTH) {
              item_.setWidth(w)
              item_.x += dx
            }
          }

        Desktop.TooltipArea { id: leftTip_
          anchors.fill: parent
          text: Sk.tr("Resize")
        }
      }

      MouseArea { id: rightResizeArea_
        anchors.fill: rightBorder_
        acceptedButtons: Qt.LeftButton

        property int pressedX
        onPressed: pressedX = mouseX
        onPositionChanged:
          if (pressed) {
            var dx = mouseX - pressedX
            var w = item_.width + dx
            if (w > _MIN_WIDTH)
              item_.setWidth(w)
          }

        Desktop.TooltipArea { id: rightTip_
          anchors.fill: parent
          text: Sk.tr("Resize")
        }
      }

      MouseArea { id: topResizeArea_
        anchors.fill: topBorder_
        acceptedButtons: Qt.LeftButton

        property int pressedY
        onPressed: pressedY = mouseY
        onPositionChanged:
          if (pressed) {
            var dy = mouseY - pressedY
            var h = item_.height - dy
            if (h > _MIN_WIDTH) {
              item_.setHeight(h)
              item_.y += dy
            }
          }

        Desktop.TooltipArea { id: topTip_
          anchors.fill: parent
          text: Sk.tr("Resize")
        }
      }

      MouseArea { id: bottomResizeArea_
        anchors.fill: bottomBorder_
        acceptedButtons: Qt.LeftButton

        property int pressedY
        onPressed: pressedY = mouseY
        onPositionChanged:
          if (pressed) {
            var dy = mouseY - pressedY
            var h = item_.height + dy
            if (h > _MIN_WIDTH)
              item_.setHeight(h)
          }

        Desktop.TooltipArea { id: bottomTip_
          anchors.fill: parent
          text: Sk.tr("Resize")
        }
      }

      MouseArea { id: topLeftMoveArea_
        anchors {
          top: parent.top
          left: parent.left
        }
        width: _ITEM_BORDER_WIDTH
        height: _ITEM_BORDER_WIDTH
        acceptedButtons: Qt.LeftButton
        drag {
          target: item_
          axis: Drag.XandYAxis
        }
        Desktop.TooltipArea { id: topLeftTip_
          anchors.fill: parent
          text: Sk.tr("Move")
        }
      }

      MouseArea { id: topRightMoveArea_
        anchors {
          top: parent.top
          right: parent.right
        }
        width: _ITEM_BORDER_WIDTH
        height: _ITEM_BORDER_WIDTH
        acceptedButtons: Qt.LeftButton
        drag {
          target: item_
          axis: Drag.XandYAxis
        }
        Desktop.TooltipArea { id: topRightTip_
          anchors.fill: parent
          text: Sk.tr("Move")
        }
      }

      MouseArea { id: bottomLeftMoveArea_
        anchors {
          bottom: parent.bottom
          left: parent.left
        }
        width: _ITEM_BORDER_WIDTH
        height: _ITEM_BORDER_WIDTH
        acceptedButtons: Qt.LeftButton
        drag {
          target: item_
          axis: Drag.XandYAxis
        }
        Desktop.TooltipArea { id: bottomLeftTip_
          anchors.fill: parent
          text: Sk.tr("Move")
        }
      }

      MouseArea { id: bottomRightMoveArea_
        anchors {
          bottom: parent.bottom
          right: parent.right
        }
        width: _ITEM_BORDER_WIDTH
        height: _ITEM_BORDER_WIDTH
        acceptedButtons: Qt.LeftButton
        drag {
          target: item_
          axis: Drag.XandYAxis
        }
        Desktop.TooltipArea { id: bottomRightTip_
          anchors.fill: parent
          text: Sk.tr("Move")
        }
      }

      // Header

      Rectangle { // header shadow
        anchors.fill: header_
        color: root_.wine ? '#10000000' : '#01000000' // alpha = 1/255 is too small that do not work on wine
      }

      Row { id: header_
        anchors {
          left: parent.left
          top: parent.top
          //margins: -_ITEM_BORDER_WIDTH - spacing
          margins: -3
        }
        spacing: 0

        property bool hover: closeButton_.hover
                          || enableButton_.hover
                          || editButton_.hover

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
          onClicked: item_.close()
        }

        Share.CircleButton { id: enableButton_
          diameter: parent.cellWidth
          font.pixelSize: parent.pixelSize
          font.bold: hover
          font.family: 'MS Gothic'
          backgroundColor: 'transparent'

          text: checked ? "◯" : "｜" //  まる
          //text: "◯" // まる
          toolTip: checked ? Sk.tr("Enabled") : Sk.tr("Disabled")

          property alias checked: item_.enabled
          onClicked: checked = !checked
        }

        Share.CircleButton { id: editButton_
          diameter: parent.cellWidth
          font.pixelSize: parent.pixelSize
          font.bold: hover
          font.family: 'MS Gothic'
          backgroundColor: 'transparent'

          text: "⌘" // U+2318 コマンド記号
          toolTip: Sk.tr("Option")

          onClicked: showEdit()
        }
      }
    }
  }
}
