/** navtoolbar.qml
 *  2/20/2013 jichi
 */
import QtQuick 1.1
//import QtDesktop 0.1 as Desktop
import '../../../js/sakurakit.min.js' as Sk
import '../../bootstrap3' as Bootstrap
//import '../share' as Share

//Row { id: root_
Column { id: root_

  signal scrollTop
  signal scrollBottom

  // - Private -

  spacing: 2

  //property int cellWidth: 20
  //property int cellHeight: 18
  //property int pixelSize: 10

  Bootstrap.Button {
    //width: parent.cellWidth
    //font.pixelSize: parent.pixelSize
    styleClass: 'btn btn-default'
    text: "↑" // うえ
    toolTip: qsTr("Scroll to the top")
    onClicked: root_.scrollTop()
    font.bold: true
    font.family: 'YouYuan'
    zoom: 0.8
    width: height
  }

  Bootstrap.Button {
    //width: parent.cellWidth
    //font.pixelSize: parent.pixelSize
    styleClass: 'btn btn-default'
    text: "↓" // した
    toolTip: qsTr("Scroll to the bottom")
    onClicked: root_.scrollBottom()
    font.bold: true
    font.family: 'YouYuan'
    zoom: 0.8
    width: height
  }
}

// EOF
