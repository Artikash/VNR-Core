/** avatarimage.qml
 *  2/21/2013 jichi
 */
import QtQuick 1.1

Image { //id: root_

  width: 50; height: 50

  property string url // use string instead of alias, since source is QUrl

  // - Private -

  fillMode: Image.PreserveAspectCrop
  //clip: true
  //fillMode: Image.Stretch
  //visible: status == Image.Ready

  sourceSize: Qt.size(50, 50) // small: 50x50

  visible: !!source && status === Image.Ready

  asynchronous: !!url && url.indexOf('http://') === 0
  source: url
}
