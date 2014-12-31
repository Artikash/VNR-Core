# topicsview.coffee
# 8/29/2014 jichi
# Required by chat.haml
#
# Beans:
# - cacheBean: cacheman.CacheCoffeeBean
# - i18nBean: coffeebean.I18nBean
# - mainBean: coffeebean.MainBean
# - postInputBean: postinput.PostInputManagerBean
# - postEditBean: postedit.PostEditorManagerBean

dprint = ->
  Array::unshift.call arguments, 'topicsview:'
  console.log.apply console, arguments

# Global variables
@READY = false # needed by chatview.py
@chatView = null

newPost = (topicId) -> postInputBean.newPost() # topicId # long ->

# Export functions

@addPost = (post) -> @chatView?.addPost post if READY
@updatePost = (post) -> @chatView?.updatePost post if READY

@spin = (t) -> # bool ->
  $('#spin').spin if t then 'large' else false

# Init

bind = ->
  $('.sec-btn').click ->
    $this = $ @
    $sec = $this.parent '.sec'
    if $sec.length
      $target = $sec.find '.sec-content'
      unless $target.is(':empty') and $this.hasClass('checked')
        $this.toggleClass 'checked'
        #effect = $this.data('effect') or 'blind'
        #effect = $this.data('effect') or 'fade'
        #$target.toggle effect unless $target.is ':empty'
        $target.toggle 'fade' unless $target.is ':empty'
    false

  # Chat
  $sec = $('.sec.sec-chat')
  $sec.find('.new-chat').click ->
    topicId = $(@).parent().data 'topic-id'
    newPost topicId
    false

init = ->
  unless @i18nBean? # the last injected bean
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    moment.locale 'ja'

    postsjs.init()
    @chatView = new postsjs.PostList
      container: $ '.sec-chat > .sec-content > .forum-posts'
      more: $ '.sec-chat > .sec-content > .footer'
      topicId: CHAT_TOPIC_ID

    bind()

    @READY = true

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF
