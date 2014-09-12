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

  // - Private -

  //Component { id: editComp_
  //  Kagami.OcrEdit {
  //    zoomFactor: root_._zoomFactor
  //    ignoresFocus: root_.ignoresFocus
  //  }
  //}

  Component.onCompleted: Local.items = [] // [item]

  Plugin.OcrRegionBean { id: bean_
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
        item.active = true
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

      function show(x, y, width, height) { // int, int, int, int
        item_.x = x
        item_.y = y
        setWidth(width)
        setHeight(height)
      }

      function hide() {
        visible = false
      }

      // - Private -

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

      MouseArea { //id: leftResizeArea_
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

      MouseArea { //id: rightResizeArea_
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

      MouseArea { //id: topResizeArea_
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

      MouseArea { //id: bottomResizeArea_
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

      MouseArea { //id: topLeftMoveArea_
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

      MouseArea { //id: topRightMoveArea_
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

      MouseArea { //id: bottomLeftMoveArea_
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

      MouseArea { //id: bottomRightMoveArea_
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
          margins: -2
        }
        spacing: 0

        property bool hover: closeButton_.hover
                          || enableButton_.hover

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
          onClicked: item_.hide()
        }

        Share.CircleButton { id: enableButton_
          diameter: parent.cellWidth
          font.pixelSize: parent.pixelSize
          font.bold: hover
          font.family: 'MS Gothic'
          backgroundColor: 'transparent'

          text: checked ? "◯" : "Φ" //  まる / ふぁい
          //text: "◯" // まる
          toolTip: checked ? Sk.tr("Enabled") : Sk.tr("Disabled")

          property alias checked: item_.enabled
          onClicked: checked = !checked
        }
      }
    }
  }
}
