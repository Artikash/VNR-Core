# chat.coffee
# 8/29/2014 jichi
# Required by chat.haml
#
# Beans:
# - cacheBean: cacheman.CacheCoffeeBean
# - i18nBean: coffeebean.I18nBean

dprint = -> console.log.apply console, arguments
#dprint = ->
timer = -> new choco.Timer arguments ...

POST_LIMIT = 20

HIGHLIGHT_INTERVAL = 1500

#HOST = 'http://sakuradite.com'
HOST = 'http://153.121.54.194'

@tr = (text) -> i18nBean.tr text # string -> string

cache_img = (url) -> url #cacheBean.cacheImage url

# Utilities

getImageUrl = (data) -> # object -> string
  cache_img "#{HOST}/upload/image/#{data.id}.#{data.suffix}"

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
.post(data-id="#{id}" data-type="#{type}")
  .left
    :if userAvatar
      %img.img-circle.avatar(src="#{userAvatar}")
  .right
    .head
      .user(style="#{userStyle}") #{userName}
      .time.text-minor = createTime
      .lang = lang
      .time.text-success = updateTime
    .content.bbcode = content
    .foot
    :if image
      .image
        %a(href="#{image.url}" title="#{image.title}")
          %img(src="#{image.url}" alt="#{image.title}")
  .reply
%hr
'''

renderPost = (data) -> # object post -> string
  HAML_POST
    id: data.id
    type: data.type
    userName: data.userName
    userStyle: if data.userColor then "color:#{data.userColor}" else ''
    lang: data.lang
    userAvatar: getAvatarUrl data.userAvatar
    content: renderContent data.content
    createTime: formatDate data.createTime, 'H:mm M/D/YY ddd'
    updateTime: if data.updateTime > data.createTime then formatDate data.updateTime, 'H:mm M/D/YY ddd' else ''
    image: if data.image then {title:data.image.title, url:getImageUrl data.image} else null

appendPosts = (posts) -> # [object post] ->
  h = (renderPost it for it in posts when it.type is 'post').join ''
  $('.topic > .posts').append h
  #$(h).hide().appendTo('.topic > .posts').fadeIn()

  replies = (it for it in posts when it.type is 'reply')
  if replies.length
    replies = _.sortBy replies, (it) -> it.createTime
    for it in replies
      $ref = $ ".post[data-id=#{it.replyId}]"
      if $ref.length
        h = renderPost it
        $ref.children('.reply').append h
      else
        dprint 'appendPosts: error: post lost'

appendPost = (post) -> # object post ->
  if post.type is 'post'
    h = renderPost post
    $(h).appendTo '.topic > .posts'
        .effect 'highlight', HIGHLIGHT_INTERVAL
  else if post.type is 'reply'
    $ref = $ ".post[data-id=#{post.replyId}]"
    if $ref.length
      h = renderPost it
      $(h).appendTo($ref.children('.reply'))
          .effect 'highlight', HIGHLIGHT_INTERVAL
    else
      dprint 'appendPost: error: post lost'
  else
    dprint 'appendPost: error: unknown post type'

# AJAX actions

spin = (t) -> $('#spin').spin if t then 'large' else false

POST_COUNT = 0 # int

paint = ->
  spin true
  rest.forum.list 'post',
    data:
      topic: TOPIC_ID
      sort: 'createTime'
      asc: false
      limit: POST_LIMIT
    error: ->
      spin false
      growl.warn tr "Internet error"
    success: (data) ->
      spin false
      if data.length
        POST_COUNT += data.length
        appendPosts data
      else
        growl.warn tr "Internet error"

more = ->
  spin true
  rest.forum.list 'post',
    data:
      topic: TOPIC_ID
      sort: 'createTime'
      asc: false
      first: POST_COUNT
      limit: POST_LIMIT
    error: ->
      spin false
      growl.warn tr "Internet error"
    success: (data) ->
      spin false
      if data.length
        POST_COUNT += data.length
        appendPosts data
      else
        growl tr "No more"

bind = ->
  $('.topic > .foot > .btn-more').click ->
    $this = $ @
    unless $this.data 'locked'
      $this.data 'lock', true
      more()
      $this.data 'lock', false
    false

## Main ##

init = ->
  unless @i18nBean? # the last injected bean
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    moment.locale 'ja'

    @TOPIC_ID = $('.topic').data 'id' # global game item id

    bind()
    paint()

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF
