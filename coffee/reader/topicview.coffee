# topicview.coffee
# 1/2/2015 jichi
# Required by chat.haml
#
# Beans:
# - cacheBean: cacheman.CacheCoffeeBean
# - i18nBean: coffeebean.I18nBean
# - mainBean: coffeebean.MainBean
# - postInputBean: postinput.PostInputManagerBean
# - postEditBean: postedit.PostEditorManagerBean
# - topicEditBean: topicedit.TopicEditorManagerBean

dprint = ->
  Array::unshift.call arguments, 'topicsview:'
  console.log.apply console, arguments

# Global variables
@READY = false # needed by postView.py

#newTopic = (topicType, subjectId, subjectType) -> topicInputBean.newTopic topicType, subjectId, subjectType # long, string ->
newPost = (topicId) -> postInputBean.newPost topicId # long ->

# Export functions

@spin = (t) -> # bool ->
  if t
    @spin.count += 1
  else
    @spin.count -= 1
  $('#spin').spin if @spin.count > 0 then 'large' else false
@spin.count = 0

@updateTopic = (topic) -> @topicView?.updateTopic topic if READY
@addPost = (post) -> @postView?.addPost post if READY
@updatePost = (post) -> @postView?.updatePost post if READY

# Init

createObjects = ->
  # Topic
  @topicView = new topicjs.Topic TOPIC_ID,
    container: $ '.forum-topic'

  # Posts
  $sec = $ '.sec-posts'
  @postView = new postsjs.PostList
    container: $sec.children '.forum-posts'
    more: $sec.find '> .footer > .btn-more'
    topicId: TOPIC_ID

  @topicView.show
    success: @postView.show()

bind = ->
  $sec = $('.sec.sec-posts')
  $sec.find('.new-post').click ->
    newPost TOPIC_ID
    false

init = ->
  unless @i18nBean? # the last injected bean
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    moment.locale LOCALE

    topicjs.init()
    postsjs.init()

    createObjects()

    bind()

    @READY = true

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF
