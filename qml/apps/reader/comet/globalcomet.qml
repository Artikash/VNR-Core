/** globalcomet.qml
 *  3/4/2014 jichi
 */
import QtQuick 1.1
import '.' as Comet

Comet.PostComet { //id: root_

  // http://developer.nokia.com/community/wiki/Using_objectName_to_find_QML_elements_from_Qt
  objectName: 'globalComet' // Use objectName to communicate with C++

  signal postDataReceived(string obj)
  signal postDataUpdated(string obj)

  // - Private -

  onPostReceived: postDataReceived(JSON.stringify(obj))
  onPostUpdated: postDataUpdated(JSON.stringify(obj))

  path: 'global'

  // Require SystemStatus plugin
  active: statusPlugin_.online
}
