/** globalcomet.qml
 *  3/4/2014 jichi
 */
import QtQuick 1.1
import '../../../js/global.js' as Global // Global.globalComet

QtObject { id: root_

  property bool active
  property int connectionCount
  signal postReceived(variant obj)
  signal postUpdated(variant obj)

  // - Private -

  property string path: 'global' // /push/vnr/global

  Component.onCompleted:
    if (!Global.globalComet) {
      Global.globalComet = 'locked'
      create();
    }

  function create() {
    var comp = Qt.createComponent('postcomet.qml')

    function finished() {
      console.log("globalcomet.qml: create finished")
      if (Global.globalComet = 'locked') {
        Global.globalComet = 'unlocked'
        var comet = Global.globalComet = comp.createObject(root_, {
          path:root_.path
        })
        comet.postReceived.connect(root_.postReceived)
        comet.postUpdated.connect(root_.postUpdated)
        comet.connectionCountChanged.connect(function() {
          root_.connectionCount = comet.connectionCount
        })
        comet.activeChanged.connect(function() {
          root_.active = comet.active
        })
        if (root_.active)
          comet.connect()
      }
    }

    //comp = Qt.createComponent('comet.qml');
    console.log("globalcomet.qml: create: status:", comp.status)
    switch (comp.status) {
    case Component.Error:
      console.log("globalcomet.qml: ERROR: failed to create component:", comp.errorString())
      break
    case Component.Ready: // == 1
      finished()
      break
    default:
      comp.statusChanged.connect(finished) // wait
    }
    console.log("globalcomet.qml: create: leave")
  }
}
