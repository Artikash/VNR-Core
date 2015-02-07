// multicombobox.qml
// 2/7/2015 jichi
import QtQuick 1.1
import QtDesktop 0.1
import "../../../../Qt/imports/QtDesktop/custom" as Custom

Custom.BasicButton { id: root_

  default property alias menuItems: popup.menuItems
  property alias model: popup.model
  property alias popupOpen: popup.visible

  property alias selectedIndex: popup.selectedIndex
  property alias hoveredIndex: popup.hoveredIndex
  property alias selectedText: popup.selectedText
  property alias hoveredText: popup.hoveredText
  property string styleHint

  background: StyleItem {
    anchors.fill: parent
    elementType: "combobox"
    sunken: root_.pressed
    raised: !sunken
    hover: root_.containsMouse
    enabled: root_.enabled
    text: root_.selectedText
    hasFocus: root_.focus
    contentHeight: 18
  }

  width: implicitWidth
  height: implicitHeight

  implicitWidth: Math.max(80, backgroundItem.implicitWidth)
  implicitHeight: backgroundItem.implicitHeight

  onWidthChanged: popup.setMinimumWidth(width)
  checkable: false

  onPressedChanged: if (pressed) popup.visible = true

  ContextMenu {
    id: popup
    property bool center: backgroundItem.styleHint("comboboxpopup")
    centerSelectedText: center
    y: center ? 0 : root_.height
  }

  // The key bindings below will only be in use when popup is
  // not visible. Otherwise, native popup key handling will take place:
  Keys.onSpacePressed: { root_.popupOpen = !root_.popupOpen }
  Keys.onUpPressed: { if (selectedIndex < model.count - 1) selectedIndex++ }
  Keys.onDownPressed: { if (selectedIndex > 0) selectedIndex-- }
}
