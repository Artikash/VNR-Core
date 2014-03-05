/** globalcomet.qml
 *  3/4/2014 jichi
 */
import QtQuick 1.1
import '../../../js/global.js' as Global

QtObject { id: root_

  property int connectionCount
  signal postReceived(variant obj)
  signal postUpdated(variant obj)

  // - Private -

  property QtObject comp

  function create() {
    //comp = Qt.createComponent('postcomet.qml');
    comp = Qt.createComponent('comet.qml');
    console.log("globalcomet.qml: create: status:", comp.status)
    switch (comp.status) {
    case Component.Error:
      console.log("globalcomet.qml: ERROR: failed to create component:", comp.errorString())
      break
    case Component.Ready:
      onCompFinished()
      break
    default:
      comp.statusChanged.connect(onCompFinished)
    }
    console.log("globalcomet.qml: create: leave")
  }

  function onCompFinished() {
    /*
    switch (mirageComp.status) {
    case Component.Ready:
      var mirage = mirageComp.createObject(root_, {
        'x': 100
        , 'y': 100
        , 'width': 640
        , 'height': 480
      });
      if (mirage) {
        mirage.yakuAt.connect(shiori_.popup)
        console.log("kagami.qml: mirage created")
      } else
        console.log("kagami.qml: ERROR: failed to create mirage object")
      break
    case Component.Error:
    default:
      console.log("kagami.qml: ERROR: failed to create mirage component:", mirageComp.errorString());
    }
  */
  }
}
