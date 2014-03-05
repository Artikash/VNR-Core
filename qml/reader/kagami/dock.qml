/** dock.qml
 *  9/30/2012 jichi
 *  Preferences panel
 *
 *  For unicode characters see:
 *  http://ja.wikipedia.org/wiki/JIS_X_0213非漢字一覧
 *  http://wiki.livedoor.jp/qvarie/d/%A5%E6%A5%CB%A5%B3%A1%BC%A5%C96.0%B0%CA%B9%DF%A4%C7%BB%C8%CD%D1%A4%C7%A4%AD%A4%EB%B3%A8%CA%B8%BB%FA%28%B4%EF%CA%AA%CA%D42%29
 */
import QtQuick 1.1
//import QtEffects 1.0 as Effects
//import org.sakuradite.reader 1.0 as Plugin
import '../../../js/sakurakit.min.js' as Sk
import '../../../js/reader.min.js' as My
import '../../../js/util.min.js' as Util
import '../share' as Share
import '.' as Kagami

Item { id: root_
  // should be larger enough to hold the panel
  width: 410 + floatingWidth
  height: 50 +
      _SHADOW_HEIGHT_3 +
      _SHADOW_HEIGHT_3 +
      _SHADOW_HEIGHT_1 +
      _SHADOW_HEIGHT_1 +
      _SHADOW_HEIGHT_4 +
      _SHADOW_HEIGHT_1

  property alias floatingWidth: floatingRect_.width

  property real zoomFactor: 1.0

  property bool ignoresFocus

  //property alias ignoresFocusChecked: focusButton_.checked

  signal resetTextPosRequested
  signal speakTextRequested

  //property string language

  property alias visibleChecked: visibleButton_.checked
  property alias commentBarChecked: commentBarButton_.checked
  property alias slimChecked: slimButton_.checked
  property alias hentaiChecked: hentaiButton_.checked
  property alias clockChecked: clockButton_.checked
  //property alias outlineChecked: outlineButton_.checked

  property alias stretchedChecked: stretchButton_.checked
  property alias displayStretchedChecked: stretchDisplayButton_.checked
  property alias displayRatioChecked: displayRatioButton_.checked
  property alias windowStretchedChecked: stretchWindowButton_.checked
  property alias paddingChecked: paddingButton_.checked

  property alias textChecked: textButton_.checked
  property alias translationChecked: translationButton_.checked
  property alias subtitleChecked: subtitleButton_.checked
  property alias commentChecked: commentButton_.checked
  property alias danmakuChecked: danmakuButton_.checked
  property alias nameChecked: nameButton_.checked

  property alias chatChecked: chatButton_.checked

  //property alias furiganaChecked: furiganaButton_.checked
  //property alias furiganaEnabled: furiganaButton_.enabled

  //property alias popupChecked: popupButton_.checked
  property alias hoverChecked: hoverButton_.checked
  //property alias copyChecked: copyButton_.checked
  property alias splitsTextChecked: splitsTextButton_.checked
  property alias splitsTranslationChecked: splitsTranslationButton_.checked
  //property alias hoverChecked: hoverButton_.checked
  property alias shadowChecked: shadowButton_.checked

  property alias glowChecked: glowButton_.checked

  property alias alignCenterChecked: centerButton_.checked

  property alias windowHookChecked: windowHookButton_.checked
  property alias windowTextChecked: windowTextButton_.checked

  //property alias speaksTextChecked: speakTextButton_.checked
  property alias speaksTextChecked: speakButton_.checked

  property alias copiesTextChecked: copyTextButton_.checked

  //property alias voiceChecked: voiceButton_.checked
  //property alias subtitleVoiceChecked: subtitleVoiceButton_.checked

  //property alias gameBorderChecked: gameBorderButton_.checked
  //property alias revertsColorChecked: revertsColorButton_.checked

  //property alias borderChecked: borderButton_.checked
  //property alias lockChecked: lockButton_.checked

  property alias shioriZoomFactor: shioriZoomSlider_.value
  property alias grimoireZoomFactor: grimoireZoomSlider_.value
  property alias widthFactor: widthSlider_.value
  property alias shadowOpacity: shadowSlider_.value
  property alias gameTextCapacity: textSlider_.value

  property alias glowIntensity: glowIntensitySlider_.value
  //property int minimumGlowIntensity: 1
  property int maximumGlowIntensity: 10

  property alias glowRadius: glowRadiusSlider_.value
  //property int minimumGlowRadius: 1
  property int maximumGlowRadius: 50

  //signal upButtonClicked
  //signal downButtonClicked

  //property int alignment: Qt.AlignLeft // horizontal

  property alias panelVisible: panel_.visible
  //property bool hover: panel_.visible && panel_.hover || visible && floatingRect_.hover
  property bool hover: panel_.hover || floatingRect_.hover

  // - Private -

  property bool wine: statusPlugin_.wine // cached

  //property bool fullScreen: ignoresFocus || stretchedChecked

  //Plugin.Growl { id: growl_ }
  //Plugin.SystemStatus { id: statusPlugin_ }
  //Plugin.PromptProxy { id: prompt_ }
  //Plugin.GameProxy { id: gamePlugin_ }

  property int _VISIBLE_DURATION: 5000

  //Component.onCompleted: console.log("grimoiredock.qml: pass")

  //property color hoverColor: '#33ff33cc' // magenta
  property color hoverColor: '#336a6d6a' // black

  //property bool canHide: !statusPlugin_.wine || slimChecked

  function hide() {
    //if (canHide)
    visible = false
  }

  //property color effectColor: '#aa007f' // purple-like
  //property color effectColor: '#6a6d6a' // black

  //property color styleColor: 'deepskyblue'

  // Shadow
  Share.FadingRectangle { id: floatingRect_
    anchors {
      left: parent.left
      verticalCenter: parent.verticalCenter
    }
    height: buttonCol_.height + 10
    width: buttonCol_.width + 10
    radius: 11

    color: active ? '#aa000000' : '#55000000'
    property bool active: !panel_.visible

    property bool hover:
      //menuButton_.hover ||    // Disable hover sensor for the first a few buttons
      //switchButton_.hover ||  // Disable hover sensor for the first a few buttons
      visibleButton_.hover ||
      slimButton_.hover ||
      hentaiButton_.hover ||
      clockButton_.hover ||
      shadowButton_.hover ||
      glowButton_.hover ||
      speakButton_.hover ||
      commentBarButton_.hover ||
      captureButton_.hover ||
      stretchButton_.hover ||
      stretchWindowButton_.hover ||
      stretchDisplayButton_.hover ||
      displayRatioButton_.hover ||
      paddingButton_.hover

    Column { id: buttonCol_
      anchors.centerIn: parent

      spacing: 4
      //height: children.count * cellHeight

      property int cellWidth: 54 * (slimChecked ? 0.4 : 1) * root_.zoomFactor
      property int cellHeight: 21 * root_.zoomFactor
      property int cellRadius: 7 * root_.zoomFactor
      property int pixelSize: 12 * root_.zoomFactor

      // * .9
      //property int cellWidth: 50 * (slimChecked ? 0.4 : 1) * root_.zoomFactor
      //property int cellHeight: 19 * root_.zoomFactor
      //property int cellRadius: 6 * root_.zoomFactor
      //property int pixelSize: 11 * root_.zoomFactor

      //property color buttonColor: '#aa434343' // gray
      property color buttonColor: '#aa00bfff' // blue
      //property color buttonColor: root_.fullScreen ? '#aa434343' : '#aa00bfff' // black:blue

      property color buttonCheckedColor: '#aa00ff00' // green
      property color buttonUncheckedColor: '#aaff0000' // red
      property color buttonDisabledColor: '#aabebebe' // gray

      property color buttonPopupColor: '#aaffff00' // yellow
      //property color buttonPopupColor: buttonCheckedColor
      //property color buttonPopupColor: buttonDisabledColor
      //property color buttonPopupColor: '#aaa020f0' // purple
      //property color buttonPopupColor: '#aaffff00' // yellow
      //property color buttonPopupColor: '#aaaa007f' // purple-like

      property string cellFont: 'DFGirl'

      Share.TextButton { id: menuButton_
        height: parent.cellHeight; width: parent.cellWidth
        text: Sk.tr("Menu")
        toolTip: Sk.tr("Show {0}").replace('{0}', Sk.tr("context menu"))
        font.pixelSize: parent.pixelSize
        //font.bold: true
        radius: parent.cellRadius
        backgroundColor: parent.buttonColor
        visible: !root_.ignoresFocus && !slimChecked

        //property alias checked: appMenu_.visible

        //language: root_.language
        font.family: parent.cellFont

        onClicked: if (!root_.ignoresFocus) {
          var gp = mapToItem(null, x + mouse.x, y + mouse.y)
          appMenu_.showPopup(gp.x, gp.y)
        }
      }

      Share.TextButton { id: switchButton_
        height: parent.cellHeight; width: parent.cellWidth
        text: slimChecked ? "⌘" : Sk.tr("Option") // U+2318 コマンド記号
        toolTip: Sk.tr("Show {0}").replace('{0}', Sk.tr("options"))
        font.pixelSize: parent.pixelSize
        //font.bold: true
        radius: parent.cellRadius
        //backgroundColor: '#aa00bfff' // deep sky blue
        backgroundColor: checked ? parent.buttonPopupColor : parent.buttonColor // gray

        //language: root_.language
        font.family: parent.cellFont

        property alias checked: panel_.visible
        //property bool enabled: root_.visibleChecked
        onClicked: checked = !checked
      }

      Share.TextButton { id: slimButton_
        height: parent.cellHeight; width: parent.cellWidth
        text: slimChecked ? "S" : "Slim" //qsTr("Slim") // Slim is not translated
        font.pixelSize: parent.pixelSize
        //font.bold: true
        radius: parent.cellRadius
        //visible: !root_.ignoresFocus

        property bool checked
        //backgroundColor: parent.buttonColor
        backgroundColor: checked ? parent.buttonCheckedColor : parent.buttonColor

        //language: root_.language
        font.family: parent.cellFont
        toolTip: qsTr("Toggle slim UI")

        onClicked: checked = !checked
      }

      //Share.TextButton { id: hentaiButton_
      //  height: parent.cellHeight; width: parent.cellWidth
      //  text: slimChecked ? "H" : "Hentai" // qsTr("Hentai")
      //  font.pixelSize: parent.pixelSize
      //  //font.bold: true
      //  radius: parent.cellRadius
      //  //visible: !root_.ignoresFocus

      //  property bool checked
      //  //backgroundColor: parent.buttonColor
      //  backgroundColor: checked ? parent.buttonCheckedColor : parent.buttonColor

      //  //language: root_.language
      //  font.family: parent.cellFont
      //  toolTip: qsTr("Enable Hentai terms in the Shared Dictionary for Machihne Translation")

      //  onClicked: {
      //    checked = !checked
      //    growl_.showMessage(checked ?
      //        qsTr("Enter hentai mode") :
      //        qsTr("Leave hentai mode"))
      //  }
      //}

      Share.TwinkleButton { id: chatButton_
        height: parent.cellHeight; width: parent.cellWidth

        property int count: Math.max(1, gameComet_.connectionCount)
        property int totalCount: Math.max(count, globalComet_.connectionCount)
        text: count + "/" + totalCount

        //property bool highlight: count > 1

        visible: gameComet_.active

        font.pixelSize: parent.pixelSize * 0.8
        //font.bold: true
        radius: parent.cellRadius
        //visible: !root_.ignoresFocus
        //visible: !statusPlugin_.wine

        property bool checked

        //pauseColor: checked ? parent.buttonCheckedColor : parent.buttonColor
        pauseColor: '#aa555555' // black

        //language: root_.language
        font.family: parent.cellFont
        //toolTip: qsTr("Read current Japanese game text using TTS")
        toolTip: qsTr("{0} out of {1} online users are playing this game now")
            .replace('{0}', count)
            .replace('{1}', totalCount)

        onClicked: checked = !checked
      }

      Share.TextButton { id: commentBarButton_
        height: parent.cellHeight; width: parent.cellWidth
        //text: checked ? "×" : "◯" // ばつ、まる
        //text: slimChecked ? "⏎" : My.tr("Sub") // U+23ce リターン記号
        text: My.tr("Sub")
        font.pixelSize: parent.pixelSize
        //font.bold: true
        //language: root_.language
        //color: enabled ? 'snow' : 'silver'
        backgroundColor: !enabled ? parent.buttonDisabledColor : checked ? parent.buttonCheckedColor : parent.buttonColor
        radius: parent.cellRadius
        font.family: parent.cellFont

        visible: !root_.ignoresFocus && !slimChecked //|| root_.stretchedChecked)

        property bool checked

        property bool enabled: statusPlugin_.online && statusPlugin_.login
        onEnabledChanged:
          if (!enabled) checked = false

        toolTip: checked ? qsTr("Hide comment input bar") : qsTr("Show comment input bar")
        onClicked: {
          if (enabled) {
            growl_.showMessage(toolTip)
            checked = !checked
          } else //if (!root_.ignoresFocus && (!statusPlugin_.online || !statusPlugin_.login))
            growl_.showWarning(qsTr("Not login or offline"))
        }
      }

      Share.TextButton { id: speakButton_
        height: parent.cellHeight; width: parent.cellWidth
        text: slimChecked ? "♪" : My.tr("Speak") // ♪
        font.pixelSize: parent.pixelSize
        //font.bold: true
        radius: parent.cellRadius
        //visible: !root_.ignoresFocus
        //visible: !statusPlugin_.wine

        property bool checked
        //backgroundColor: parent.buttonColor
        backgroundColor: checked ? parent.buttonCheckedColor : parent.buttonColor

        //language: root_.language
        font.family: parent.cellFont
        //toolTip: qsTr("Read current Japanese game text using TTS")
        toolTip: qsTr("Automatically read Japanese game text using TTS")

        onClicked: {
          checked = !checked
          if (checked)
            root_.speakTextRequested()
          //growl_.showMessage(toolTip)
        }
      }

      Share.TextButton { id: captureButton_
        height: parent.cellHeight; width: parent.cellWidth
        text: slimChecked ? My.tr("Capture").charAt(0) : My.tr("Capture")
        font.pixelSize: parent.pixelSize
        //font.bold: true
        radius: parent.cellRadius
        //visible: !root_.ignoresFocus
        //visible: !statusPlugin_.wine

        //backgroundColor: parent.buttonColor
        backgroundColor: parent.buttonColor

        //language: root_.language
        font.family: parent.cellFont
        //toolTip: qsTr("Read current Japanese game text using TTS")
        toolTip: qsTr("Take a screen shot, and save to the desktop and the clipboard")

        onClicked: gamePlugin_.captureWindow()
      }

      Share.TextButton { id: glowButton_
        height: parent.cellHeight; width: parent.cellWidth
        text: slimChecked ? qsTr("Transp").charAt(0) : qsTr("Transp")
        font.pixelSize: parent.pixelSize
        //font.bold: true
        radius: parent.cellRadius
        //visible: !root_.ignoresFocus

        property bool checked
        //backgroundColor: parent.buttonColor
        backgroundColor: checked ? parent.buttonCheckedColor : parent.buttonColor

        //language: root_.language
        font.family: parent.cellFont
        toolTip: qsTr("Toggle background shadow")
        onClicked: checked = !checked
      }

      Share.TextButton { id: stretchButton_
        height: parent.cellHeight; width: parent.cellWidth
        //text: checked ? "×" : "◯" // ばつ、まる
        text: slimChecked ? My.tr("FS").charAt(0) : My.tr("FS")
        font.pixelSize: parent.pixelSize
        //font.bold: true
        //language: root_.language
        //color: enabled ? 'snow' : 'silver'
        backgroundColor: stretchRect_.visible ? parent.buttonPopupColor :
                         checked ? parent.buttonCheckedColor :
                         parent.buttonColor
        radius: parent.cellRadius
        font.family: parent.cellFont

        visible: !root_.ignoresFocus

        property bool checked: stretchWindowButton_.checked || stretchDisplayButton_.checked
        onCheckedChanged:
          paddingButton_.checked = checked
        //  if (checked)
        //    stretchRect_.visible = false

        //property bool enabled: statusPlugin_.online && statusPlugin_.login
        //onEnabledChanged:
        //  if (!enabled) checked = false

        toolTip: checked ? qsTr("Leave full screen") : qsTr("Enter full screen")
        onClicked:
          stretchRect_.visible = !stretchRect_.visible
          //if (checked && !stretchRect_.visible)
          //  stretchWindowButton_.checked = stretchDisplayButton_.checked = false
          //else

        Share.FadingRectangle { id: stretchRect_
          visible: false

          anchors {
            left: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: 6
          }
          height: stretchRow_.height + 10
          width: stretchRow_.width + 10
          radius: 11

          color: floatingRect_.color

          Row { id: stretchRow_
            anchors.centerIn: parent
            spacing: 4

            Share.TextButton { id: stretchWindowButton_
              height: buttonCol_.cellHeight; width: buttonCol_.cellWidth
              //text: checked ? "×" : "◯" // ばつ、まる
              text: slimChecked ? Sk.tr("Window").charAt(0) : Sk.tr("Window")
              font.pixelSize: buttonCol_.pixelSize
              //font.bold: true
              //language: root_.language
              //color: enabled ? 'snow' : 'silver'
              backgroundColor: checked ? buttonCol_.buttonCheckedColor : buttonCol_.buttonColor
              radius: buttonCol_.cellRadius
              font.family: buttonCol_.cellFont

              property bool checked
              onClicked: {
                stretchDisplayButton_.checked = false
                displayRatioButton_.checked = false
                checked = !checked
              }

              toolTip: qsTr("Change the game window size to fit the screen")
            }

            Share.TextButton { id: stretchDisplayButton_
              height: buttonCol_.cellHeight
              width: buttonCol_.cellWidth
              //text: checked ? "×" : "◯" // ばつ、まる
              text: slimChecked ? Sk.tr("Screen").charAt(0) : Sk.tr("Screen")
              font.pixelSize: buttonCol_.pixelSize
              //font.bold: true
              //language: root_.language
              //color: enabled ? 'snow' : 'silver'
              backgroundColor: (checked && !displayRatioButton_.checked) ? buttonCol_.buttonCheckedColor : buttonCol_.buttonColor
              radius: buttonCol_.cellRadius
              font.family: buttonCol_.cellFont

              visible: !root_.wine

              property bool checked
              onClicked: {
                stretchWindowButton_.checked = false
                if (checked && displayRatioButton_.checked) {
                  checked = false
                  displayRatioButton_.checked = false
                  checked = true
                } else {
                  displayRatioButton_.checked = false
                  checked = !checked
                }
              }

              toolTip: qsTr("Change the display resolution to fit the game window")
            }

            Share.TextButton { id: displayRatioButton_
              height: buttonCol_.cellHeight
              width: buttonCol_.cellWidth
              //text: checked ? "×" : "◯" // ばつ、まる
              text: slimChecked ? qsTr("Ratio").charAt(0) : qsTr("Ratio")
              font.pixelSize: buttonCol_.pixelSize
              //font.bold: true
              //language: root_.language
              //color: enabled ? 'snow' : 'silver'
              backgroundColor: (checked && stretchDisplayButton_.checked) ? buttonCol_.buttonCheckedColor : buttonCol_.buttonColor
              radius: buttonCol_.cellRadius
              font.family: buttonCol_.cellFont

              visible: !root_.wine

              property bool checked: false
              onClicked: {
                stretchWindowButton_.checked = false
                if (!checked && stretchDisplayButton_.checked) {
                  stretchDisplayButton_.checked = false
                  checked = true
                  stretchDisplayButton_.checked = true
                } else {
                  checked = true
                  stretchDisplayButton_.checked = !stretchDisplayButton_.checked
                }
              }

              toolTip: qsTr("Change the display resolution while keeping display ratio")
            }

            Share.TextButton { id: paddingButton_
              height: buttonCol_.cellHeight
              width: buttonCol_.cellWidth
              //text: checked ? "×" : "◯" // ばつ、まる
              text: slimChecked ? Sk.tr("Padding").charAt(0) : Sk.tr("Padding")
              font.pixelSize: buttonCol_.pixelSize
              //font.bold: true
              //language: root_.language
              //color: enabled ? 'snow' : 'silver'
              backgroundColor: checked ? buttonCol_.buttonCheckedColor : buttonCol_.buttonColor
              radius: buttonCol_.cellRadius
              font.family: buttonCol_.cellFont

              property bool checked: false
              onClicked: checked = !checked

              toolTip: qsTr("Show black padding around game window")
            }
          }

          Share.CloseButton { //id: closeButton_
            anchors { left: parent.left; top: parent.top; margins: -9 }
            onClicked: stretchRect_.visible = false
          }
        }
      }

      Share.TextButton { id: visibleButton_
        height: parent.cellHeight; width: parent.cellWidth
        //text: checked ? "×" : "◯" // ばつ、まる
        text: slimChecked ? "◯": //	まる
              Sk.tr("Pause")
              //checked ? Sk.tr("Hide") : Sk.tr("Show")
        font.pixelSize: parent.pixelSize
        //font.bold: true
        //language: root_.language
        backgroundColor: checked ? parent.buttonColor : parent.buttonUncheckedColor
        radius: parent.cellRadius
        font.family: parent.cellFont

        property bool checked: true
        toolTip: checked ? qsTr("Hide subtitles") : qsTr("Show subtitles")
        onClicked: {
          checked = !checked
          growl_.showMessage(checked ?
              qsTr("Show subtitles") :
              qsTr("Hide subtitles"))
        }
      }

      //Share.TextButton { id: focusButton_
      //  height: parent.cellHeight; width: parent.cellWidth
      //  text: My.tr("Φ") // ふぁい
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  radius: parent.cellRadius
      //  language: root_.language
      //  backgroundColor: parent.buttonColor // gray
      //  property bool checked: true
      //  toolTip: qsTr("Show the blue button in full screen as well (might NOT work for OLD games)")
      //  onClicked: {
      //    checked = !checked
      //    if (checked)
      //      growl_.showMessage(qsTr("Hide the blue button in full screen"))
      //    else
      //      growl_.showMessage(qsTr("Show the blue button in full screen"))
      //  }
      //}
    }

    Share.CloseButton { //id: closeButton_
      anchors { left: parent.left; top: parent.top; margins: -9 }
      onClicked: root_.hide()
    }
  }

  onVisibleChanged: stretchRect_.visible = false
  onIgnoresFocusChanged: stretchRect_.visible = false

  property int _SHADOW_HEIGHT_1: buttonGrid_.spacing + 3
  property int _SHADOW_HEIGHT_2: buttonGrid_.spacing * 2 + buttonGrid_.cellHeight + 3
  property int _SHADOW_HEIGHT_3: buttonGrid_.spacing * 3 + buttonGrid_.cellHeight * 2 + 3
  property int _SHADOW_HEIGHT_4: buttonGrid_.spacing * 4 + buttonGrid_.cellHeight * 3 + 5

  Share.FadingRectangle { id: panel_
    anchors.fill: parent
    anchors.leftMargin: root_.floatingWidth + 2

    property bool hover: mouseArea_.hover ||
                         //closeButton_.hover ||
                         textButton_.hover ||
                         nameButton_.hover ||
                         chatButton_.hover ||
                         translationButton_.hover ||
                         subtitleButton_.hover ||
                         commentButton_.hover ||
                         danmakuButton_.hover ||
                         //popupButton_.hover ||
                         hoverButton_.hover ||
                         //copyButton_.hover ||
                         splitsTextButton_.hover ||
                         splitsTranslationButton_.hover ||
                         shadowButton_.hover ||
                         centerButton_.hover ||
                         windowHookButton_.hover ||
                         windowTextButton_.hover ||
                         //speakTextButton_.hover ||
                         copyTextButton_.hover ||
                         //voiceButton_.hover ||
                         //subtitleVoiceButton_.hover ||
                         shioriZoomSlider_.hover ||
                         grimoireZoomSlider_.hover ||
                         widthSlider_.hover ||
                         shadowSlider_.hover ||
                         textSlider_.hover ||
                         glowIntensitySlider_.hover ||
                         glowRadiusSlider_.hover

    onVisibleChanged: if (visible) hideTimer_.restart()
    Component.onCompleted: visible = false  // hide on startup

    radius: 10
    //gradient: Gradient {  // color: aarrggbb
    //  GradientStop { position: 0.0;  color: '#9c8f8c8c' }
    //  GradientStop { position: 0.17; color: '#7a6a6d6a' }
    //  GradientStop { position: 0.77; color: '#4f3f3f3f' }
    //  GradientStop { position: 1.0;  color: '#7a6a6d6a' }
    //}

    color: '#66000000' // black

    Rectangle { id: shadow1_ // Group#1 shadow
      anchors {
        left: parent.left; right: parent.right
        top: parent.top
        leftMargin: 10; rightMargin: 10
        topMargin: 8
      }
      radius: 10
      height: _SHADOW_HEIGHT_3

      //color: '#8b545454' // gray
      color: '#55000000' // black
      //gradient: Gradient {  // color: aarrggbb
      //  GradientStop { position: 0.0;  color: '#9c8f8c00' }
      //  GradientStop { position: 0.17; color: '#7a6a6d00' }
      //  GradientStop { position: 0.77; color: '#4f3f3f00' }
      //  GradientStop { position: 1.0;  color: '#7a6a6d00' }
      //}
    }

    Rectangle { id: shadow2_// Group#2 shadow
      anchors {
        left: parent.left; right: parent.right
        top: shadow1_.bottom
        leftMargin: 10; rightMargin: 10
        topMargin: 7
      }
      radius: shadow1_.radius
      height: _SHADOW_HEIGHT_3
      color: shadow1_.color
    }

    Rectangle { id: shadow3_// Group#3 shadow
      anchors {
        left: parent.left; right: parent.right
        top: shadow2_.bottom
        leftMargin: 10; rightMargin: 10
        topMargin: 7
      }
      radius: shadow1_.radius
      height: _SHADOW_HEIGHT_1
      color: shadow1_.color
    }

    Rectangle { id: shadow4_ // Group#4 shadow
      anchors {
        left: parent.left; right: parent.right
        top: shadow3_.bottom
        leftMargin: 10; rightMargin: 10
        topMargin: 7
      }
      radius: shadow1_.radius
      height: _SHADOW_HEIGHT_1
      color: shadow1_.color
    }

    Rectangle { id: shadow5_ // Group#5 shadow
      anchors {
        left: parent.left; right: parent.right
        top: shadow4_.bottom
        leftMargin: 10; rightMargin: 10
        topMargin: 7
      }
      radius: shadow1_.radius
      height: _SHADOW_HEIGHT_4
      color: shadow1_.color
    }

    MouseArea { id: mouseArea_
      anchors.fill: parent
      acceptedButtons: Qt.NoButton
      hoverEnabled: true
    }

    Grid { id: buttonGrid_
      anchors.centerIn: parent
      columns: 2
      spacing: 20

      property int cellWidth: 160
      property int cellHeight: 10
      property int pixelSize: 14
      property string cellFont: 'YouYuan'
      property color buttonColor: '#8b434343' // gray


      // - Group#1: text filers -

      Share.CheckBox { id: textButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: My.tr("Game text")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Display {0}").replace('{0}', My.tr("game text"))
        //language: root_.language
      }

      Share.CheckBox { id: nameButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: My.tr("Character name")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Display {0}").replace('{0}', My.tr("character name"))
        //language: root_.language
      }

      Share.CheckBox { id: translationButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: My.tr("Machine translation")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Display {0}").replace('{0}', My.tr("machine translation"))
        //language: root_.language
      }

      Share.CheckBox { id: subtitleButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: My.tr("Community subtitle")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Display {0} from other users").replace('{0}', Sk.tr("subtitles"))
        //language: root_.language
      }

      Share.CheckBox { id: commentButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: My.tr("User comment")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Display {0} from other users").replace('{0}', Sk.tr("comments"))
        //language: root_.language
      }

      Share.CheckBox { id: danmakuButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: My.tr("User danmaku")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Display {0} from other users").replace('{0}', Sk.tr("danmaku"))
        //language: root_.language
      }

      // - Group#2: text options -

      Share.CheckBox { id: shadowButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: qsTr("Shadow behind text")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Display background shadow behind the text")
        //language: root_.language
      }

      Share.CheckBox { id: centerButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: qsTr("Text aligns center")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Align center or align left")
        //language: root_.language
      }


      Share.CheckBox { id: splitsTextButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: qsTr("Split game text")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Split Japanese sentences by lines")
        //language: root_.language
      }

      Share.CheckBox { id: splitsTranslationButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: qsTr("Split translation")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Split machine translation sentences by lines")
        //language: root_.language
      }

      //Share.CheckBox { id: furiganaButton_
      //  width: parent.cellWidth; height: parent.cellHeight
      //  text: qsTr("Japanese furigana")
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  font.family: parent.cellFont
      //  toolTip: qsTr("Show furigana above the Japanese text")
      //  //language: root_.language
      //}

      //Share.CheckBox { id: popupButton_
      //  width: parent.cellWidth; height: parent.cellHeight
      //  text: qsTr("Popup when click")
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  font.family: parent.cellFont
      //  toolTip: qsTr("Popup translation from dictionary when click the game text")
      //  //language: root_.language
      //}

      Share.CheckBox { id: hoverButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: qsTr("Popup when hover")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Popup translation from dictionary when hover the game text")
        //language: root_.language
      }

      Share.CheckBox { id: copyTextButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: qsTr("Text to clipboard")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Automatically copy Japanese game text to the clipboard")
        //language: root_.language
      }

      // - Group#3: Other

      Share.CheckBox { id: clockButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: qsTr("Show current time")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Show current time")
      }

      Item { width: 1; height: 1 }

      //Share.CheckBox { id: copyButton_
      //  width: parent.cellWidth; height: parent.cellHeight
      //  text: qsTr("Copy when click")
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  font.family: parent.cellFont
      //  toolTip: qsTr("Copy the clicked text to clipboard")
      //  //language: root_.language
      //}

      Share.CheckBox { id: hentaiButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: qsTr("Enable hentai terms")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Enable Hentai terms in the Shared Dictionary for Machihne Translation")

        onCheckedChanged:
          growl_.showMessage(checked ?
              qsTr("Enter hentai mode") :
              qsTr("Leave hentai mode"))
      }

      //Share.CheckBox { id: gameBorderButton_
      //  width: parent.cellWidth; height: parent.cellHeight
      //  text: qsTr("Outline game window")
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  toolTip: qsTr("Display a red border outside the game window")
      //  language: root_.language
      //}

      //Item { width: 1; height: 1 }

      //Share.CheckBox { id: revertsColorButton_
      //  width: parent.cellWidth; height: parent.cellHeight
      //  text: qsTr("Black on white")
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  toolTip: qsTr("Use black font and white shadow")
      //}

      // - Group#4: Hook

      Share.CheckBox { id: windowHookButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: qsTr("Translate window")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Translate window title, menu, label, button, etc.")
        //language: root_.language
      }

      Share.CheckBox { id: windowTextButton_
        width: parent.cellWidth; height: parent.cellHeight
        text: qsTr("Keep Japanese text")
        font.pixelSize: parent.pixelSize
        font.bold: true
        font.family: parent.cellFont
        toolTip: qsTr("Keep original Japanese text in window translation")
        //language: root_.language
        enabled: windowHookButton_.checked
      }

      // - Group #4: TTS

      //Share.CheckBox { id: speakTextButton_
      //  width: parent.cellWidth; height: parent.cellHeight
      //  text: qsTr("Automatic TTS")
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  font.family: parent.cellFont
      //  toolTip: qsTr("Automatically read Japanese game text using TTS")
      //  //language: root_.language
      //}

      //Item { width: 1; height: 1 }

      //Share.CheckBox { id: voiceButton_
      //  width: parent.cellWidth; height: parent.cellHeight
      //  text: qsTr("Dub voices")
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  //enabled: speakTextButton_.checked
      //  font.family: parent.cellFont
      //  toolTip: qsTr("Add voice for selected characters instead of reading all game texts")
      //  //language: root_.language
      //}

      //Item { width: 1; height: 1 }

      //Share.CheckBox { id: subtitleVoiceButton_
      //  width: parent.cellWidth; height: parent.cellHeight
      //  text: qsTr("Dub subtitles")
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  //enabled: speakTextButton_.checked && voiceButton_.checked
      //  font.family: parent.cellFont
      //  toolTip: qsTr("Read machine or community subtitles instead of original game texts")
      //  //language: root_.language
      //}

      // - Group #5: Sliders

      Share.LabeledSlider { id: textSlider_
        height: parent.cellHeight
        width: parent.cellWidth
        text: qsTr("Count")
        font.pixelSize: parent.pixelSize
        font.bold: true
        handleWidth: 15
        toolTip: qsTr("The maximum length of allowed game text. Text longer than that will be ignored.")
        handleToolTip: qsTr("Maximum number of allowed characters in the game text is {0}").replace('{0}', Math.round(value / 2))

        //value: 50.0
        minimumValue: 100.0
        maximumValue: 1000.0
        font.family: parent.cellFont
        //language: root_.language
      }

      Share.LabeledSlider { id: shadowSlider_
        height: parent.cellHeight
        width: parent.cellWidth
        text: Sk.tr("Opacity")
        font.pixelSize: parent.pixelSize
        font.bold: true
        handleWidth: 15
        toolTip: qsTr("Text background shadow transparency")
        handleToolTip: Math.round(value * 100 / maximumValue) + "%"

        enabled: root_.shadowChecked

        //value: 0.27
        minimumValue: 0.0
        maximumValue: 1.0
        font.family: parent.cellFont
        //language: root_.language
      }

      Share.LabeledSlider { id: grimoireZoomSlider_
        height: parent.cellHeight
        width: parent.cellWidth
        text: Sk.tr("Zoom")
        font.pixelSize: parent.pixelSize
        font.bold: true
        handleWidth: 15
        toolTip: qsTr("Zoom font size")
        handleToolTip: Math.round(value * 100) + "%"

        //value: 1.0
        minimumValue: 0.5
        maximumValue: 3.0
        font.family: parent.cellFont
        //language: root_.language
      }

      Share.LabeledSlider { id: widthSlider_
        height: parent.cellHeight
        width: parent.cellWidth
        text: Sk.tr("Width")
        font.pixelSize: parent.pixelSize
        font.bold: true
        handleWidth: 15
        toolTip: qsTr("Text box width")
        handleToolTip: Math.round(value * 100) + "%"

        //value: 1.0
        minimumValue: 0.2
        maximumValue: 2.0
        font.family: parent.cellFont
        //language: root_.language
      }

      Share.LabeledSlider { id: shioriZoomSlider_
        height: parent.cellHeight
        width: parent.cellWidth
        text: Sk.tr("Popup")
        font.pixelSize: parent.pixelSize
        font.bold: true
        handleWidth: 15
        toolTip: qsTr("Zoom popup size")
        handleToolTip: Math.round(value * 100) + "%"

        //value: 1.0
        minimumValue: 0.5
        maximumValue: 3.0
        font.family: parent.cellFont
        //language: root_.language
      }

      Item { width: 1; height: 1 }

      Share.LabeledSlider { id: glowIntensitySlider_
        height: parent.cellHeight
        width: parent.cellWidth
        text: qsTr("G.I")
        font.pixelSize: parent.pixelSize
        font.bold: true
        handleWidth: 15
        toolTip: qsTr("Text glowing intensity")
        handleToolTip: Math.round(value * 100 / maximumValue) + "%"

        //value: 2
        minimumValue: 1 //root_.minimumGlowIntensity
        maximumValue: root_.maximumGlowIntensity
        font.family: parent.cellFont
        //language: root_.language
      }

      Share.LabeledSlider { id: glowRadiusSlider_
        height: parent.cellHeight
        width: parent.cellWidth
        text: qsTr("G.R")
        font.pixelSize: parent.pixelSize
        font.bold: true
        handleWidth: 15
        toolTip: qsTr("Text glowing range")
        handleToolTip: Math.round(value * 100 / maximumValue) + "%"

        //value: 8.0
        minimumValue: 1 //root_.minimumGlowRadius
        maximumValue: root_.maximumGlowRadius
        font.family: parent.cellFont
        //language: root_.language
      }

      // - Last group: Buttons -

      //Share.TextButton {
      //  height: 26 //parent.cellHeight
      //  width: parent.cellWidth + 10
      //  text: qsTr("Reset text position")
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  radius: 9
      //  //backgroundColor: '#aa00bfff' // deep sky blue
      //  backgroundColor: parent.buttonColor // gray

      //  onClicked: root_.resetTextPosRequested()
      //  toolTip: qsTr("Move the text box to the top of the game window")
      //  font.family: parent.cellFont
      //  //language: root_.language
      //}

      //Share.TextButton {
      //  height: 26 //parent.cellHeight
      //  width: parent.cellWidth /2 + 20
      //  text: Sk.tr("Screenshot")
      //  font.pixelSize: parent.pixelSize
      //  font.bold: true
      //  radius: 9
      //  //backgroundColor: '#aa00bfff' // deep sky blue
      //  backgroundColor: parent.buttonColor // gray

      //  onClicked: gamePlugin_.captureWindow()
      //  toolTip: qsTr("Take a screen shot, and save to the desktop and the clipboard")
      //  //language: root_.language
      //  font.family: parent.cellFont
      //}

      //Item { width: 1; height: 1 }
    }

    Share.CloseButton { id: closeButton_
      anchors { left: parent.left; top: parent.top; margins: -4 }
      onClicked: panel_.visible = false
    }
  }

  Timer { id: hideTimer_
    interval: root_._VISIBLE_DURATION
    onTriggered: {
      if (panel_.hover)
        start()
      else
        panel_.visible = false
    }
  }

  Kagami.AppMenu { id: appMenu_ }

  MouseArea {
    anchors.fill: parent
    acceptedButtons: Qt.RightButton
    enabled: !root_.ignoresFocus
    onPressed: if (!root_.ignoresFocus) {
      var gp = mapToItem(null, x + mouse.x, y + mouse.y)
      appMenu_.showPopup(gp.x, gp.y)
    }
  }
}

// EOF

/*
  Share.TextButton { id: upButton_
    anchors {
      top: switchButton_.bottom
      left: root_.alignment === Qt.AlignLeft ? parent.left : undefined
      right: root_.alignment === Qt.AlignRight ? parent.right : undefined
      margins: 10
    }
    language: 'ja'
    width: 10*_ZOOM; height: 15*_ZOOM
    text: "↑" // うえ
    font.pixelSize: 12*_ZOOM; //font.bold: true
    color: enabled ? 'snow' : 'silver'
    backgroundColor: hover ? root_.hoverColor : 'transparent'
    //effect: Share.TextEffect {}
    style: Text.Raised
    styleColor: enabled ? root_.styleColor : 'transparent'

    property bool enabled: root_.visibleChecked
    onClicked: if (enabled) root_.upButtonClicked()
    toolTip: qsTr("Scroll to top")
  }

  Share.TextButton { id: downButton_
    anchors {
      top: upButton_.bottom
      left: root_.alignment === Qt.AlignLeft ? parent.left : undefined
      right: root_.alignment === Qt.AlignRight ? parent.right : undefined
      margins: 10
    }
    language: 'ja'
    width: 10*_ZOOM; height: 15*_ZOOM
    text: "↓" // した
    font.pixelSize: 12*_ZOOM; //font.bold: true
    color: enabled ? 'snow' : 'silver'
    backgroundColor: hover ? root_.hoverColor : 'transparent'
    //effect: Share.TextEffect {}
    style: Text.Raised
    styleColor: enabled ? root_.styleColor : 'transparent'

    property bool enabled: root_.visibleChecked
    onClicked: if (enabled) root_.downButtonClicked()
    toolTip: qsTr("Scroll to bottom")
  }


      Share.TextButton { id: borderButton_
        anchors {
          top: switchButton_.bottom
          left: root_.alignment === Qt.AlignLeft ? parent.left : undefined
          right: root_.alignment === Qt.AlignRight ? parent.right : undefined
          margins: 10
        }
        language: 'ja'
        width: 10*_ZOOM; height: 15*_ZOOM
        text: "□" // しかく
        font.pixelSize: 14*_ZOOM; //font.bold: true
        color: enabled ? 'snow' : 'silver'
        backgroundColor: hover ? root_.hoverColor : 'transparent'
        //effect: Share.TextEffect {}
        style: Text.Raised
        styleColor: !enabled ? 'transparent' :
                    checked ? 'red' :
                    root_.styleColor
        property bool checked
        property bool enabled: root_.visibleChecked
        onClicked: if (enabled) checked = !checked
        toolTip: qsTr("Toggle text box border, which you can use to drag the unlocked text box")
      }

      Share.TextButton { id: lockButton_
        anchors {
          top: borderButton_.bottom
          left: root_.alignment === Qt.AlignLeft ? parent.left : undefined
          right: root_.alignment === Qt.AlignRight ? parent.right : undefined
          margins: 10
        }
        language: 'ja'
        width: 10*_ZOOM; height: 15*_ZOOM
        text: "鎖" // くさり
        font.pixelSize: 14*_ZOOM
        font.bold: false
        color: enabled ? 'snow' : 'silver'
        backgroundColor: hover ? root_.hoverColor : 'transparent'
        //effect: Share.TextEffect {}
        style: Text.Raised
        styleColor: !enabled ? 'transparent' :
                    checked ? 'red' :
                    root_.styleColor
        property bool checked
        property bool enabled: root_.visibleChecked
        onClicked: if (enabled) checked = !checked
        toolTip: qsTr("Lock the position of the text box")
      }
*/
