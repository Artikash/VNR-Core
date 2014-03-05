/** cometcounter.qml
 *  3/3/2014 jichi
 */
import QtQuick 1.1
//import org.sakuradite.reader 1.0 as Plugin
import '../share' as Share

Share.TwinkleButton { //id: root_

  property int count
  text: String(count)

  //visible: globalComet_.active //&& count > 1

  // - Private -

  height: 21; width: 21

  //property int count: Math.max(1, globalComet_.connectionCount)

  property bool highlight: count > 1

  font.pixelSize: 12
  //font.bold: true
  radius: 7
  //visible: !root_.ignoresFocus
  //visible: !statusPlugin_.wine

  property bool checked

  //pauseColor: checked ? parent.buttonCheckedColor : parent.buttonColor
  pauseColor: '#aa555555' // black

  //language: root_.language
  font.family: 'DFGirl'
  //toolTip: qsTr("Read current Japanese game text using TTS")
  toolTip: qsTr("{0} people are playing games now").replace('{0}', count)

  onClicked: checked = !checked
}
