/** searchbox.qml
 *  10/21/2012 jichi
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import '../../../js/sakurakit.min.js' as Sk
import '.' as Share

Share.InputBox { id: root_
  property int currentCount
  property int totalCount

  // - Private -

  placeholderText: Sk.tr("Search") + " ... (" + Sk.tr("regular expression") + ", " + Sk.tr("case-insensitive") + ")"
  toolTip: Sk.tr("Search")

  //onTextChanged:
  //  graffiti_.setSearchText(Util.trim(text))

  font.pixelSize: 16
  //font.italic: true

  Text { // item count
    anchors {
      right: clearButton_.left
      top: parent.top
      topMargin: 4
    }
    color: root_.text ? 'blue' : '#7f8185'
    font.pixelSize: 12
    //font.bold: true
    text: totalCount ? root_.currentCount + "/" + root_.totalCount : String(root_.currentCount)
  }

  Share.ClearButton { id: clearButton_
    anchors {
      right: parent.right
      verticalCenter: parent.verticalCenter
    }
    onClicked: {
      root_.text = ""
      root_.accepted()
    }
  }
}
