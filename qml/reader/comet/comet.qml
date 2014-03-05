/** comet.qml
 *  2/21/2014 jichi
 */
import QtQuick 1.1
import '../../../js/atmosphere.min.js' as Atmosphere
import '../../../js/define.min.js' as Define
import '../../../js/local.js' as Local

QtObject { id: root_

  property string path
  //string path: 'topic/term'

  property bool active: false

  signal message(string data)

  // - Private -

  property string host: Define.DOMAIN_COM + '/push/vnr/'
  //property string host: 'http://localhost:8080/push/vnr/'

  Timer { id: reconnectTimer_
    interval: 5000 // 5 seconds
    repeat: false
    onTriggered:
      if (active)
        Local.comet.reconnect()
    //onRunningChanged: console.log("timer: running = ", running)
  }

  onActiveChanged:
    if (active) open()
    else close()

  function open() {
    console.log("comet.qml:open: path =", path)
    Local.comet.connect()
  }

  function close() {
    console.log("comet.qml:close: path =", path)
    Local.comet.disconnect()
  }

  Component.onDestruction: close()

  Component.onCompleted: {
    var url = root_.host + root_.path
    var comet = Local.comet = Atmosphere.subscribe(url)
    comet.reconnectTimer = reconnectTimer_
    comet.onMessage = function (xhr, data) {
      if (data) root_.message(data)
    }
    comet.onError = function (xhr, msg) {
      console.log("comet.qml: error:", msg)
    }
  }
}
