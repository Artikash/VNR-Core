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
  #l = Array::slice arguments
  l = Array::slice.call arguments
  l.unshift 'topicsview:'
  console.log.apply console, l

# Global variables
@READY = false # needed by chatview.py
@chatView = null

# Export functions

@addPost = (post) -> @chatView?.addPost post if READY
@updatePost = (post) -> @chatView?.updatePost post if READY

@spin = (t) -> # bool ->
  $('#spin').spin if t then 'large' else false

# Init

init = ->
  unless @i18nBean? # the last injected bean
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    moment.locale 'ja'

    postsjs.init()
    @chatView = new postsjs.PostList
      container: $ '.topic > .posts'
      more: $ '.topic > .footer > .btn-more'
      topicId: TOPIC_ID

    #@TOPIC_ID = $('.topic').data 'id' # global game item id

    @READY = true

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF
