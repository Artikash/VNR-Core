/** searchtoolbar.qml
 *  2/20/2013 jichi
 */
import QtQuick 1.1
//import QtDesktop 0.1 as Desktop
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/util.min.js' as Util
import '../share' as Share

Row { id: root_ // Theme refers to Bootstrap.label

  signal triggered(string text)

  // - Private -
  spacing: 2

  property int cellWidth: 45
  property int cellHeight: 18
  property int cellRadius: 3 // the same as Bootstrap.label
  property int pixelSize: 12 // the same as Bootstrap.label

  Share.TextButton {
    //language: 'ja'
    width: parent.cellWidth; height: parent.cellHeight
    radius: parent.cellRadius
    //shadowWidth: width + 15; shadowHeight: height + 15
    font.pixelSize: parent.pixelSize
    //font.bold: true
    color: 'snow'
    backgroundColor: hover ? '#333' : '#666'
    //font.family: 'MS Gothic'

    text: '@' + Sk.tr("user")
    toolTip: qsTr("Search your terms")

    onClicked: {
      var name = datamanPlugin_.getCurrentUserName()
      if (name)
        root_.triggered('@' + name)
    }
  }

  Share.TextButton {
    //language: 'ja'
    width: parent.cellWidth; height: parent.cellHeight
    radius: parent.cellRadius
    //shadowWidth: width + 15; shadowHeight: height + 15
    font.pixelSize: parent.pixelSize
    //font.bold: true
    color: 'snow'
    backgroundColor: hover ? '#333' : '#666'
    //font.family: 'MS Gothic'

    text: '#' + Sk.tr("game")
    toolTip: qsTr("Search terms specific to current game")

    onClicked: {
      var name = datamanPlugin_.getCurrentGameName()
      if (name)
        root_.triggered('#' + name)
    }
  }

  Share.TextButton {
    //language: 'ja'
    width: parent.cellWidth; height: parent.cellHeight
    radius: parent.cellRadius
    //shadowWidth: width + 15; shadowHeight: height + 15
    font.pixelSize: parent.pixelSize
    //font.bold: true
    color: 'snow'
    backgroundColor: hover ? '#333' : '#666'
    //font.family: 'MS Gothic'

    text: Sk.tr("language")
    toolTip: qsTr("Search terms in your language")

    onClicked: {
      var lang2 = statusPlugin_.userLanguage
      if (lang2)
        root_.triggered(Sk.tr(Util.languageName(lang2)))
    }
  }
}
