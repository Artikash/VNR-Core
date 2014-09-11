/** ajaxindicator.qml
 *  2/10/2013 jichi
 *  Fullscreen invisible screen.
 */
import QtQuick 1.1
import QtDesktop 0.1 as Desktop
import org.sakuradite.reader 1.0 as Plugin
import '../share' as Share

Share.GradientSpinner { // ajax indicator

  //width: 80; height: 80
  width: 60; height: 60

  property alias containsMouse: toolTip_.containsMouse

  // - Private -

  //backgroundColor: '#46b1d28f'
  //foregroundColor: '#ff77b753'

  function increase(c) { // param  c  float, return float
    return Math.min(Math.max(c + count * 40 - 40,
        0), 255)
  }
  function decrease(c) { // param  c  float, return float
    return Math.min(Math.max(c - count * 40 + 40,
        0), 255)
  }

  property int delta: 40

  // From green to red
  backgroundColor: Qt.rgba(
      increase(177) / 255.0,
      decrease(210) / 255.0,
      decrease(143) / 255.0,
      70/255.0)
  foregroundColor: Qt.rgba(
      increase(119) / 255.0,
      decrease(183) / 255.0,
      decrease(83) / 255.0,
      1.0)

  //outerRadius: 1.0
  innerRadius: Math.max(0.5, 0.9 - count * 0.1)

  Behavior on backgroundColor { ColorAnimation {} }
  Behavior on foregroundColor { ColorAnimation {} }
  Behavior on innerRadius { NumberAnimation {} }

  Plugin.ThreadPoolStatus { id: threadPool_
    //property bool empty: !threadCount
  }

  property int count: threadPool_.threadCount // cached
  visible: count > 0

  onVisibleChanged: running = visible   // broken orz

  Desktop.TooltipArea { id: toolTip_
    anchors.fill: parent
    text: qsTr("{0} background threads are running").replace('{0}', threadPool_.threadCount)
  }
}

// EOF

  //states: [
  //  State { // visible
  //    when: root_.visible
  //    PropertyChanges { target: root_
  //      opacity: 1
  //    }
  //  }
  //  , State { // hidden
  //    when: !root_.visible
  //    PropertyChanges { target: root_
  //      opacity: 0
  //   }
  //  }
  //]
  //transitions: Transition {
  //  NumberAnimation { property: 'opacity' }
  //}
