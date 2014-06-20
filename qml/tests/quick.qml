/** quick.qml
 *  5/25/2014 jichi
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import '../ui/buttongroup' as ButtonGroup

Item {
  implicitWidth: 250; implicitHeight: 400

  // - Private -

  ButtonGroup.ButtonRow {
    anchors.centerIn: parent
    //spacing: 2
    Desktop.Button { text: "hello 1"; checkable: true }
    Desktop.Button { text: "hello 2"; checkable: true }
    Desktop.Button { text: "hello 3"; checkable: true }
    Desktop.Button { text: "hello 4"; checkable: true }

    exclusive: true
    //onCheckedButtonChanged:
    //  console.log(checkedButton)
  }
}
