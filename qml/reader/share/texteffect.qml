/** texteffect.qml
 *  1/6/2013 jichi
 */
//import QtEffects 1.0 as Effects
import '../../imports/qmleffects' as Effects

Effects.TextShadow {
  blurRadius: 8; offset: '1,1'

  property bool highlight: false
  //property bool visible: true
  color: //!visible ? 'transparent' :
         highlight ? 'red' :
         '#2d5f5f' // dark green
}
