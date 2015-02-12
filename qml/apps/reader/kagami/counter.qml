/** counter.qml
 *  2/18/2015 jichi
 */
import QtQuick 1.1
import '../share' as Share

Share.TextButton {

  property int count
  property string prefix

  property real zoomFactor: 1.0

  // - Private -

  text: count ? prefix + count : prefix

  width: 30 * zoomFactor
  height: 30 * zoomFactor

  font.pixelSize: 12 * zoomFactor

  // TODO: color
  backgroundColor: enabled ? '#aa555555' : '#aa555555' // black : black
}
