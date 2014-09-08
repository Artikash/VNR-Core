/** ocredit.qml
 *  9/7/2014 jichi
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import '../../../js/sakurakit.min.js' as Sk
import '../share' as Share

Rectangle { id: root_

  property real zoomFactor: 1.0
  property bool ignoresFocus: false

  function show(image, text) { // OcrImageObject
    imageObject = image
    textArea_.textEdit.text = text
    textArea_.textEdit.font.pixelSize = zoomFactor * 12
    visible = true
  }

  function hide() {
    visible = false
    imageObject = null
  }

  // - Private -

  //property alias text: textEdit_.text

  property QtObject imageObject // OcrImageObject  ocr controller

  property int _ROOT_MARGIN: 9

  width: 300 // TODO: allow dynamically adjust this value
  height: _ROOT_MARGIN
        + image_.height
        + _ROOT_MARGIN
        + textRect_.height
        + _ROOT_MARGIN
        + rightButtonRow_.height
        + _ROOT_MARGIN

  radius: 9 // the same as ocrpopup

  color: '#99000000' // black
  //color: hover ? '#99000000' : '#55000000' // black

  //property bool hover: tip_.containsMouse || closeButton_.hover

  MouseArea { id: mouse_
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton
    drag.target: root_; drag.axis: Drag.XandYAxis
  }

  Image { id: image_
    anchors {
      left: parent.left; right: parent.right
      bottom: textRect_.top
      margins: _ROOT_MARGIN
    }
    source: imageObject ? imageObject.imageUrl : ''
    smooth: true
    fillMode: Image.PreserveAspectFit
  }

  //Desktop.TooltipArea { id: tip_
  //  anchors.fill: parent
  //  text: Sk.tr("Edit")
  //}

  // Text edit

  Rectangle { id: textRect_ // background color
    anchors {
      left: parent.left
      right: parent.right
      bottom: rightButtonRow_.top
      margins: _ROOT_MARGIN
    }
    color: '#aaffffff' // white
    height: 100
    radius: 4

    Share.TextArea { id: textArea_
      anchors.fill: parent
      //anchors.margins: _ROOT_MARGIN
      anchors.margins: 4

      contextMenuEnabled: !root_.ignoresFocus

      textEdit { // id: textEdit_  -- such syntax is not allowed
        textFormat: TextEdit.PlainText
        wrapMode: TextEdit.WordWrap
        //color: 'snow'
        color: 'black'
        //font.pixelSize: Math.12 * root_._zoomFactor // FIXME: Do not work
      }

      onCopyTriggered: {
        var t = textEdit.text
        if (t)
          clipboardPlugin_.text = t
      }
    }
  }

  // Footer

  property int buttonWidth: 50

  Share.CheckButton { //id: enableButton_
    anchors {
      left: parent.left
      bottom: parent.bottom
      margins: _ROOT_MARGIN
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
      margins: _ROOT_MARGIN
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
