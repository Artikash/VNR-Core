/** filtertoolbar.qml
 *  2/20/2013 jichi
 */
import QtQuick 1.1
//import QtDesktop 0.1 as Desktop
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/reader.min.js' as My
import '../../../components/bootstrap3' as Bootstrap

Grid { id: root_ // Theme refers to Bootstrap.label

  property string values // types split by ','

  // - Private -
  spacing: 2
  rows: 2

  onValuesChanged: console.log("filtertoolbar.qml: values =", values)

  Component.onCompleted: {
    var m = [
      {value:'escape',   text:qsTr("Trans"),    toolTip:qsTr("Replace text matching pattern with translation")}
      , {value:'source', text:My.tr("Input"),   toolTip:qsTr("Fix input text before translation")}
      , {value:'target', text:My.tr("Output"),  toolTip:qsTr("Fix output text after translation")}
      , {value:'name',   text:My.tr("Name"),    toolTip:qsTr("A Japanese first or last name")}
      , {value:'yomi',   text:My.tr("Yomi"),    toolTip:qsTr("Yomigana of a Japanese name")}
      , {value:'title',  text:My.tr("Suffix"),  toolTip:qsTr("Title after a Japanese name")}
      , {value:'origin', text:Sk.tr("Game"),    toolTip:qsTr("Fix game text")}
      , {value:'speech', text:"TTS",            toolTip:qsTr("Fix text to send to TTS")}
      , {value:'ocr',    text:"OCR",            toolTip:qsTr("Fix text received from OCR")}
      , {value:'macro',  text:Sk.tr("Macro"),   toolTip:qsTr("Reusable regular expression")}
    ]
    for (var i in m)
      buttonComp_.createObject(root_, m[i])
  }

  function toggleValue(value, checked) { // string, bool
    var s = root_.values
    if (checked) {
      if (s)
        s += ','
      s += value
    } else
      s = s.replace(value, '') // assume types are not included by others
           .replace(',,', ',')
           .replace(/^,/, '')
           .replace(/,$/, '')
    root_.values = s
  }

  Component { id: buttonComp_
    Bootstrap.Button {
      width: 45; height: 18
      styleClass: checked ? 'btn btn-success' : 'btn btn-default'
      checkable: true
      property string value
      onClicked: root_.toggleValue(value, checked)
    }
  }
}

// EOF
