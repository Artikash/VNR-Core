// multicombobox.qml
// 2/7/2015 jichi
// Modified from QtDesktop.ComboBox to support multiple selection.
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import "../../../../Qt/imports/QtDesktop/custom" as DesktopCustom
import "." as DesktopEx

DesktopCustom.BasicButton { id: root_

  default property alias menuItems: popup_.menuItems
  property alias model: popup_.model
  property alias popupOpen: popup_.visible

  property alias selectedIndex: popup_.selectedIndex
  property alias hoveredIndex: popup_.hoveredIndex
  property alias selectedText: popup_.selectedText
  property alias hoveredText: popup_.hoveredText
  property string styleHint

  background: Desktop.StyleItem {
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

  onWidthChanged: popup_.setMinimumWidth(width)
  checkable: false

  onPressedChanged: if (pressed) popup_.visible = true

  DesktopEx.MultiContextMenu { id: popup_
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
