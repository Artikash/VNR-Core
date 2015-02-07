/** quick.qml
 *  5/25/2014 jichi
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import '../components/buttongroup' as ButtonGroup
import '../components/desktopex' as DesktopEx

Item {
  implicitWidth: 250; implicitHeight: 400

  // - Private -

  DesktopEx.MultiComboBox {
    anchors.centerIn: parent
    width: 100
    height: 30

    model: ListModel { //id: typeModel_
      Component.onCompleted: {
        append({value:'google', text:"Google", checkable:true, checked:true})
        append({value:'bing', text:"Bing", checkable:true, checked:true})
        append({value:'naver', text:"Naver", checkable:true})
      }
    }
  }

  //ButtonGroup.ButtonRow {
  //  anchors.centerIn: parent
  //  //spacing: 2
  //  Desktop.Button { text: "hello 1"; checkable: true }
  //  Desktop.Button { text: "hello 2"; checkable: true }
  //  Desktop.Button { text: "hello 3"; checkable: true }
  //  Desktop.Button { text: "hello 4"; checkable: true }

  //  exclusive: true
  //  //onCheckedButtonChanged:
  //  //  console.log(checkedButton)
  //}
}
