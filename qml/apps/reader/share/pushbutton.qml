/** pushbutton.qml
 *  9/7/2014 jichi
 *  A black color theme for text button
 */
import '.' as Share

Share.TextButton {

  property bool enabled: true

  // - Private -
  width: 50
  height: 25
  backgroundColor: enabled ? '#aa434343' : '#aabebebe' // black : gray

  radius: 5
}
