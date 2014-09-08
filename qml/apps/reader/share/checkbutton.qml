/** checkbutton.qml
 *  9/7/2014 jichi
 */
import '.' as Share

Share.PushButton {

  property bool checked

  // - Private -

  backgroundColor: checked ? '#aa00ff00' : '#aa434343' // green : black

  onClicked: checked = !checked
}
