/** buttonrow.qml
 *  6/20/2014 jichi
 *  See: QtDesktop/ButtonRow.qml
 */
import QtQuick 1.1
import "buttongroup.min.js" as Js

Row { id: root
  // Specifies the grouping behavior. If enabled, the checked property on buttons contained
  property bool exclusive: true

  // Contains the last checked Button.
  property Item checkedButton

  // - Private -
  Component.onCompleted:  Js.create(root, {direction:Qt.Horizontal})
  Component.onDestruction: Js.destroy()
}
