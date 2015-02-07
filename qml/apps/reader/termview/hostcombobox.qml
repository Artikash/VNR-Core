/** hostcombobox.qml
 *  2/7/2015 jichi
 */
import QtQuick 1.1
import '../../../components/desktopex' as DesktopEx
import '.' as TermView

DesktopEx.MultiComboBox { id: root_

  property string selectedValues

  // - Private -

  Component.onCompleted: {
    for (var i in menuItems) {
      var item = menuItems[i]
      if (~selectedValues.indexOf(item.value))
        item.checked = true
    }
  }

  //seperatorText: "|"
  emptyText: "*"

  TermView.HostMenuItem { id: i1, value: 'bing' }
  TermView.HostMenuItem { id: i2, value: 'google' }
  TermView.HostMenuItem { id: i3, value: 'lecol' }
  TermView.HostMenuItem { id: i4, value: 'infoseek' }
  TermView.HostMenuItem { id: i5, value: 'excite' }
  TermView.HostMenuItem { id: i6, value: 'transru' }
  TermView.HostMenuItem { id: i7, value: 'naver' }
  TermView.HostMenuItem { id: i8, value: 'baidu' }
  TermView.HostMenuItem { id: i9, value: 'jbeijing' }
  TermView.HostMenuItem { id: i10, value: 'fastait' }
  TermView.HostMenuItem { id: i11, value: 'dreye' }
  TermView.HostMenuItem { id: i12, value: 'eztrans' }
  TermView.HostMenuItem { id: i13, value: 'atlas' }
  TermView.HostMenuItem { id: i14, value: 'lec' }
  TermView.HostMenuItem { id: i15, value: 'hanviet' }

  menuItems: [i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15]

  onSelectionChanged: updateSelectedValues()

  function updateSelectedValues() {
    var l = []
    for (var i in menuItems) {
      var item = menuItems[i]
      if (item.checked)
        l.push(item.value)
    }
    selectedValues = l.join(',')
  }
}
