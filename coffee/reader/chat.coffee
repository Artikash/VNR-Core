# chat.coffee
# 8/29/2014 jichi
# Required by chat.haml
#
# Beans:
# - i18nBean: coffeebean.I18nBean

dprint = -> console.log.apply console, arguments
#dprint = ->
timer = -> new choco.Timer arguments ...

POST_LIMIT = 20

#HOST = 'http://sakuradite.com'

# Render

renderPost = (post) -> # kw -> string
  post.content

renderPosts = (l) -> # [post] -> string
  l.map(renderPost).join ''

showPosts = (l) -> # [post] -> string
  h = renderPosts l
  $('.topic > .posts').html h

# AJAX actions

refresh = ->
  rest.forum.list 'post',
    data:
      topic: TOPIC_ID
      limit: POST_LIMIT
    error: ->
      #$spin.spin false
      #$container.removeClass 'rendered'
      #$msg.addClass 'text-danger'
      #    .text "(Internet #{tr 'error'})"
    success: (data) ->
      if data.length
        showPosts data
      #$spin.spin false
      #if data.file
      #  $msg.empty()
      #  h = renderSettings data.file
      #  $container
      #    .hide()
      #    .html h
      #    .fadeIn()

## Main ##

init = ->
  unless @cacheBean?
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    @TOPIC_ID = $('.topic').data 'id' # global game item id

    refresh()

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF
