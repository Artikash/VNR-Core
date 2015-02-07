// multicontextmenu.qml
// 2/7/2015 jichi
// Modified from QtContextMenu to support multiple selection.
import QtQuick 1.1
import QtDesktop 0.1 as Desktop

Desktop.Menu { id: root_
  property int x
  property int y
  property bool visible

  property variant items // list<MenuItem>

  signal selectionChanged()

  property string selectedText
  property string seperatorText: ", "

  property variant selectedItems

  // 'centerSelectedText' means that the menu will be positioned
  //  so that the selected text' top left corner will be at x, y.
  property bool centerSelectedText: true


  // - Private -

  visible: false
  onMenuClosed: visible = false

  onSelectionChanged: updateSelection()

  Component.onCompleted: {
    updateSelection()
    connectActions()
  }
  Component.onDestruction: disconnectActions()

  signal f()

  function connectActions() {
    for (var i in items)
      items[i].toggled.connect(selectionChanged)
  }

  function disconnectActions() {
    for (var i in items)
      items[i].toggled.disconnect(selectionChanged)
  }

  onVisibleChanged: {
    if (visible) {
      var globalPos = mapToItem(null, x, y)
      showPopup(globalPos.x, globalPos.y, centerSelectedText ? selectedIndex : 0)
    } else {
      hidePopup()
      //updateSelectedText()
    }
  }

  function updateSelection() {
    updateSelectedText()
    updateSelectedItems()
  }

  function updateSelectedText() {
    var t = ""
    for (var i in items) {
      var item = items[i]
      if (item.checked) {
        if (t)
          t += seperatorText + item.text
        else
          t = item.text
      }
    }
    selectedText = t
  }

  function updateSelectedItems() {
    var l = []
    for (var i in items) {
      var item = items[i]
      if (item.checked)
        l.push(item)
    }
    selectedItems = l
  }
}
