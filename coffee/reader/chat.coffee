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

tr = (t) -> t

cache_img = (url) -> url #cacheBean.cacheImage url

# Utilities

getAvatarUrl = (id, size=128) -> # string, int -> string
  unless id
    ''
  else unless size
    cache_img "http://media.getchute.com/media/#{id}"
  else
    cache_img "http://media.getchute.com/media/#{id}/#{size}x#{size}"

renderContent = (t) -> # string -> string
  return '' unless t?
  bbcode.parse linkify _.escape t.replace /]\n/g, ']'
    .replace /<li><\/li>/g, '<li>'

formatDate = (t, fmt) -> # long, string -> string
  try
    t *= 1000 if typeof(t) in ['number', 'string']
    if t then moment(t).format fmt else ''
    #date = @dateFromUnixTime date if typeof(date) in ['number', 'string']
    #dateformat date, fmt
  catch
    ''

# Render

# - content  string html
# - userAvatar  string url or ''
# - createTime  string
# - updateTime  string or ''
# - userStyle  string or ''
# - lang  string or ''
HAML_POST = Haml '''\
.post
  .left
    :if userAvatar
      %img.img-circle.avatar(src="#{userAvatar}")
  .right
    .head
      .user(style="#{userStyle}") #{userName}
      .time.text-minor = createTime
      .lang = lang
      .time.text-success = updateTime
    .content = content
%hr
'''

renderPost = (post) -> # kw -> string
  HAML_POST
    userName: post.userName
    userStyle: if post.userColor then "color:#{post.userColor}" else ''
    lang: post.lang
    userAvatar: getAvatarUrl post.userAvatar, 50
    content: renderContent post.content
    createTime: formatDate post.createTime, 'H:mm M/D/YY ddd'
    updateTime: if post.updateTime > post.createTime then formatDate post.updateTime, 'H:mm M/D/YY ddd' else ''

renderPosts = (l) -> # [post] -> string
  l.map(renderPost).join ''

showPosts = (l) -> # [post] -> string
  h = renderPosts l
  $('.topic > .posts').html h

# AJAX actions

refresh = ->
  spin true
  rest.forum.list 'post',
    data:
      topic: TOPIC_ID
      limit: POST_LIMIT
    error: ->
      spin false
      #$spin.spin false
      #$container.removeClass 'rendered'
      #$msg.addClass 'text-danger'
      #    .text "(Internet #{tr 'error'})"
    success: (data) ->
      spin false
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

spin = (t) -> $('#spin').spin if t then 'large' else false

## Main ##

init = ->
  unless @cacheBean?
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    moment.locale 'ja'

    @TOPIC_ID = $('.topic').data 'id' # global game item id

    refresh()

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF
