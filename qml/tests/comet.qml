/** comet.qml
 *  3/4/2014 jichi
 */
import QtQuick 1.1
import '../reader/comet' as Comet

Item {
  implicitWidth: 250; implicitHeight: 400

  // - Private -

  Comet.GlobalComet { id: root_ }
  Component.onCompleted: {
    console.log("completed")
    root_.create()
  }
}
