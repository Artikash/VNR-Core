/** gamecomet.qml
 *  2/21/2014 jichi
 */
import QtQuick 1.1
import '.' as Comet

Comet.PostComet { //id: root_
  property int gameId

  // - Private -
  path: 'game/' + gameId
}
