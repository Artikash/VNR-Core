/** ocredit.qml
 *  9/7/2014 jichi
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import '../../../js/sakurakit.min.js' as Sk
import '../share' as Share

Rectangle { id: root_

  function show(image) { // OcrImageObject
    imageObject = image
    visible = true
  }

  function hide() {
    visible = false
    imageObject = null
  }

  // - Private -

  property QtObject imageObject // OcrImageObject  ocr controller

  property int _ROOT_MARGIN: 9

  width: 300 // TODO: allow dynamically adjust this value
  height: image_.height + rightButtonRow_.height + _ROOT_MARGIN * 3

  radius: 10 // the same as ocrpopup

  color: '#99000000' // black
  //color: hover ? '#99000000' : '#55000000' // black

  //property bool hover: tip_.containsMouse || closeButton_.hover

  Image { id: image_
    anchors {
      left: parent.left; right: parent.right
      bottom: rightButtonRow_.top
      bottomMargin: _ROOT_MARGIN
    }
    source: imageObject ? imageObject.imagePath : ''
    smooth: true
    fillMode: Image.PreserveAspectFit
  }

  MouseArea { id: mouse_
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton
    drag.target: root_; drag.axis: Drag.XandYAxis
  }

  //Desktop.TooltipArea { id: tip_
  //  anchors.fill: parent
  //  text: Sk.tr("Edit")
  //}

  property int buttonWidth: 50

  Share.CheckButton { //id: enableButton_
    anchors {
      left: parent.left
      bottom: parent.bottom
      bottomMargin: _ROOT_MARGIN
      leftMargin: _ROOT_MARGIN
    }
    property bool checked
    width: root_.buttonWidth
    //text: checked ? Sk.tr("Enable") : Sk.tr('Disable')
    text: Sk.tr("Enable")
    toolTip: Sk.tr("Enable")
  }

  Row { id: rightButtonRow_
    anchors {
      right: parent.right
      bottom: parent.bottom
      bottomMargin: _ROOT_MARGIN
      rightMargin: _ROOT_MARGIN
    }

    spacing: _ROOT_MARGIN

    Share.PushButton { //id: cancelButton_
      width: root_.buttonWidth
      text: Sk.tr("Cancel")
      toolTip: Sk.tr("Cancel")
      onClicked: root_.hide()
    }

    Share.PushButton { //id: saveButton_
      width: root_.buttonWidth
      text: Sk.tr("Save")
      toolTip: Sk.tr("Save")
    }
  }

  Share.CloseButton { id: closeButton_
    anchors { left: parent.left; top: parent.top; margins: -4 }
    onClicked: root_.hide()
    //visible: hover || tip_.containsMouse // this will make it difficult to close
  }
}
