/** postview.qml
 *  2/19/2014 jichi
 */
import QtQuick 1.1
import '../../js/define.min.js' as Define
import 'postview' as PostView
import 'share' as Share

Item { id: root_
  implicitWidth: 250; implicitHeight: 400

  property string url: Define.DOMAIN_COM + '/api/json/post/'

  property string topicKey

  // - Private -

  Component.onCompleted: {
    termComet_.postReceived.connect(function(post) {
      growlPlugin_.msg(qsTr("New post from {0}").replace("{0}", post.userName))
      if (!searchEdit_.searchText)
        postList_.prepend(post)
    })

    termComet_.postUpdated.connect(function(post) {
      growlPlugin_.msg(qsTr("Post edited from {0}").replace("{0}", post.userName))
      if (!searchEdit_.searchText) {
        postList_.remove(post)
        postList_.prepend(post)
      }
    })
  }

  onVisibleChanged:
    if (visible)
      postList_.refresh()

  PostView.PostList { id: postList_
    anchors {
      left: parent.left
      right: parent.right
      top: parent.top
      bottom: searchEdit_.top
      margins: 9
    }
    postUrl: root_.url + 'list'
    postData: ({ // Parenthesis is indispensable here
      topic: root_.topicKey
      , search: searchEdit_.searchText
      , limit: 20
      , sort: 'updateTime'
      , asc: 'true'
    })

    Share.Blocker {
      anchors.fill: parent
      visible: postList_.count <= 0
      color: 'transparent'
    }
  }

  PostView.SearchEdit { id: searchEdit_
    anchors {
      left: parent.left; right: parent.right
      bottom: inputEdit_.top
      //bottomMargin: -2
      //margins: 10
      topMargin: 5
      bottomMargin: 5
    }
    currentCount: postList_.count

    //onTextChanged: postList_.refresh()
    onAccepted: postList_.refresh()
  }

  PostView.InputEdit { id: inputEdit_
    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      //margins: 10
      bottomMargin: 5
    }
    topicKey: root_.topicKey
    height: visible ? 55 : 0
    visible: !!statusPlugin_.userId && statusPlugin_.userId !== 4
  }
}
