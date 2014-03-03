/** circlebutton.qml
 *  3/2/2014 jichi
 */
import '.' as Share

Share.TextButton {

  property int diameter: 25

  // - Private -

  width: diameter; height: diameter

  radius: diameter * 0.6

  font.pixelSize: diameter / 2
}
