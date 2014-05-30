/** quick.qml
 *  5/25/2014 jichi
 */
import QtQuick 1.1
//import QtDesktop 0.1 as Desktop
import '../reader/share' as Share

Item {
  implicitWidth: 250; implicitHeight: 400

  // - Private -

  Share.IntSpinBox {
    anchors.centerIn: parent
    width: 100
    postfix: '/50'

    minimumValue: 1
  }
}
