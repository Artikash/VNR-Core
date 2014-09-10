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

  property string text: textArea_.textEdit.text

  function show(image, text) { // OcrImageObject
    imageObject = image
    loadImageProperties()
    textArea_.textEdit.text = text || ('(' + Sk.tr("empty") + ')')
    textArea_.textEdit.font.pixelSize = zoomFactor * 12
    visible = true
  }

  function hide() {
    visible = false
    saveImageProperties()
    imageObject = null
  }

  // - Private -

  property QtObject imageObject // OcrImageObject  ocr controller

  property bool savedEnabled
  property bool savedColorIntensityEnable
  property real savedMinimumColorIntensity
  property real savedMaximumColorIntensity

  function loadImageProperties() {
    if (imageObject) {
      enableButton_.checked = savedEnabled = imageObject.editable
      colorEnableButton_.checked = savedColorIntensityEnable = imageObject.colorIntensityEnabled
      colorSlider_.startValue = savedMinimumColorIntensity = imageObject.minimumColorIntensity
      colorSlider_.stopValue = savedMaximumColorIntensity = imageObject.maximumColorIntensity
    }
  }

  function saveImageProperties() {
    if (imageObject) {
      imageObject.editable = savedEnabled
      imageObject.colorIntensityEnabled = savedColorIntensityEnable
      imageObject.minimumColorIntensity = savedMinimumColorIntensity
      imageObject.maximumColorIntensity = savedMaximumColorIntensity
    }
  }

  function reset() {
    if (imageObject) {
      enableButton_.checked = savedEnabled
      colorEnableButton_.checked = savedColorIntensityEnable
      colorSlider_.startValue = savedMinimumColorIntensity
      colorSlider_.stopValue = savedMaximumColorIntensity
      ocr()
    }
  }

  property int _ROOT_MARGIN: 9
  //property int spacing: _ROOT_MARGIN

  width: 300 // TODO: allow dynamically adjust this value
  height: _ROOT_MARGIN
        + image_.height
        + _ROOT_MARGIN
        + textRect_.height
        + _ROOT_MARGIN
        + colorSlider_.height
        + _ROOT_MARGIN
        + rightButtonRow_.height
        + _ROOT_MARGIN

  radius: 9 // the same as ocrpopup

  color: '#99000000' // black
  //color: hover ? '#99000000' : '#55000000' // black

  //property bool hover: tip_.containsMouse || closeButton_.hover

  function ocr() {
    if (!imageObject)
      return

    imageObject.minimumColorIntensity = colorSlider_.startValue
    imageObject.maximumColorIntensity = colorSlider_.stopValue
    imageObject.colorIntensityEnabled = colorEnableButton_.checked
    imageObject.editable = enableButton_.checked

    textArea_.textEdit.text = imageObject.ocr() || ('(' + Sk.tr("empty") + ')')
  }

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
      left: parent.left; right: parent.right
      bottom: colorSlider_.top
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
        //wrapMode: TextEdit.WordWrap
        wrapMode: TextEdit.Wrap
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

  // Slider

  Share.CheckDot { id: colorEnableButton_
    anchors {
      left: parent.left
      verticalCenter: colorSlider_.verticalCenter
      margins: _ROOT_MARGIN
    }
    enabled: enableButton_.checked
    toolTip: qsTr("Color intensity")
  }


  function toPercentage(value) { // real -> string
    return Math.round(value * 100) + '%'
  }

  Share.LabeledRangeSlider { id: colorSlider_
    anchors {
      left: colorEnableButton_.right
      right: parent.right
      bottom: rightButtonRow_.top
      //margins: _ROOT_MARGIN // remove left margin
      rightMargin: _ROOT_MARGIN
      bottomMargin: _ROOT_MARGIN
    }
    enabled: enableButton_.checked && colorEnableButton_.checked
    spacing: _ROOT_MARGIN
    labelWidth: 30

    minimumValue: 0.0
    maximumValue: 1.0

    //startValue: 0.3
    //stopValue: 0.7

    startLabelText: toPercentage(startValue)
    stopLabelText: toPercentage(stopValue)

    startLabelToolTip: qsTr("Minimum text color intensity")
    stopLabelToolTip: qsTr("Maximum text color intensity")

    startHandleToolTip: startLabelToolTip
    stopHandleToolTip: stopLabelToolTip

    sliderToolTip: qsTr("Range of the text color intensity")

    //onStartValueChanged: root_.refresh()
    //onStopValueChanged: root_.refresh()
  }

  // Footer

  property int buttonWidth: 50

  Share.CheckButton { id: enableButton_
    anchors {
      left: parent.left
      bottom: parent.bottom
      margins: _ROOT_MARGIN
    }
    width: root_.buttonWidth
    //text: checked ? Sk.tr("Enable") : Sk.tr('Disable')
    text: Sk.tr("Enable")
    toolTip: qsTr("Enable image transformations for OCR")

    //onCheckedChanged: root_.refresh()
  }

  Row { id: rightButtonRow_
    anchors {
      right: parent.right
      bottom: parent.bottom
      margins: _ROOT_MARGIN
    }

    spacing: _ROOT_MARGIN

    Share.PushButton { //id: refreshButton_
      width: root_.buttonWidth
      text: "OCR"
      toolTip: qsTr("Apply OCR to the current image")
      onClicked: root_.ocr()

      //styleHint: 'primary'
      backgroundColor: '#aa00bfff' // blue
    }

    Share.PushButton { id: resetButton_
      width: root_.buttonWidth
      text: Sk.tr("Reset")
      toolTip: qsTr("Reset to the last OCR settings")
      onClicked: root_.reset()

      //styleHint: 'danger'
      backgroundColor: '#aaff0000' // red
    }
  }

  Share.CloseButton { id: closeButton_
    anchors { left: parent.left; top: parent.top; margins: -4 }
    onClicked: root_.hide()
    //visible: hover || tip_.containsMouse // this will make it difficult to close
  }
}
