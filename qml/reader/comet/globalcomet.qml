/** globalcomet.qml
 *  3/4/2014 jichi
 */
import QtQuick 1.1
import '.' as Comet

Comet.PostComet { //id: root_
  path: 'global'
  // http://developer.nokia.com/community/wiki/Using_objectName_to_find_QML_elements_from_Qt
  objectName: 'gComet' // Use objectName to communicate with C++

  // Require SystemStatus plugin
  active: statusPlugin_.online
}
