# chat.coffee
# 8/29/2014 jichi
# Required by chat.haml
#
# Beans:
# - cacheBean: cacheman.CacheCoffeeBean
# - i18nBean: coffeebean.I18nBean
# - mainBean: coffeebean.MainBean
# - postInputBean: postinput.PostInputManagerBean
# - postEditBean: postedit.PostEditorManagerBean

dprint = -> console.log.apply console, arguments
#dprint = ->
timer = -> new choco.Timer arguments ...

POST_LIMIT = 20

HIGHLIGHT_INTERVAL = 1500

PAGE_TITLE = document.title

# Render

createTemplates = ->

  # HAML for topic
  # - id
  # - type
  # - userName
  # - userStyle
  # - lang
  # - userAvatarUrl  url or null
  # - content: string  html
  # - createTime
  # - updateTime
  # - image  url or null
  # - likeCount  int
  # - dislikeCount  int
  @HAML_POST = Haml """\
.post.post-new(data-id="${id}" data-type="${type}")
  .left
    :if userAvatarUrl
      %img.img-circle.avatar(src="${userAvatarUrl}")
  .right
    .header
      %a.user(style="${userStyle}") @${userName}
      .time.text-minor = createTime
      .lang = lang
      .time.text-success = updateTime
    .content.bbcode = content
    :if USER_NAME && USER_NAME != 'guest'
      .footer
        .btn-group.like-group.fade-in
          %a.like.btn.btn-link.btn-sm(role="button" title="#{tr 'Like'}")
            %span.fa.fa-thumbs-up
            %span.value = likeCount
          %a.dislike.btn.btn-link.btn-sm(role="button" title="#{tr 'Dislike'}")
            %span.fa.fa-thumbs-down
            %span.value = dislikeCount
        .btn-group.pull-right.fade-in
          :if userName == USER_NAME
            %a.btn.btn-link.btn-sm.btn-edit(role="button" title="#{tr 'Edit'}") #{tr 'Edit'}
          %a.btn.btn-link.btn-sm.btn-reply(role="button" title="#{tr 'Reply'}") #{tr 'Reply'}
    :if image
      .image
        %a(href="${image.url}" title="${image.title}")
          %img(src="${image.url}" alt="${image.title}")
  .reply
""".replace /\$/g, '#'

POSTS = [] # [object post]

renderPost = (data) -> # object post -> string
  HAML_POST
    id: data.id
    type: data.type
    userName: data.userName
    userStyle: if data.userColor then "color:#{data.userColor}" else ''
    lang: util.getLangName data.lang
    userAvatarUrl: util.getAvatarUrl data.userAvatar
    content: util.renderContent data.content
    createTime: util.formatDate data.createTime
    updateTime: if data.updateTime > data.createTime then util.formatDate data.updateTime else ''
    image: if data.image then {title:data.image.title, url:util.getImageUrl data.image} else null
    likeCount: data.likeCount or 0
    dislikeCount: data.dislikeCount or 0

$getPost = (postId) ->  $ ".post[data-id=#{postId}]" # long -> $el

findPost = (id) -> _.findWhere POSTS, id:id # long -> object

editPost = (post) -> postEditBean.editPost JSON.stringify post # long ->

replyPost = (postId) ->  postInputBean.replyPost postId # long ->

bindNewPosts = ->
  $('.post.post-new').each ->
    $post = $ @
      .removeClass 'post-new'

    postId = $post.data 'id'
    post = findPost postId

    $header = $post.find '> .right > .header'
    $footer = $post.find '> .right > .footer'

    $header.find('a.user').click ->
      mainBean.showUser post.userName if post?.userName
      false

    $footer.find('.btn-edit').click ->
      editPost post if post
      false

    $footer.find('.btn-reply').click ->
      replyPost postId
      false

    $footer.find('.like-group').removeClass 'fade-in' if post?.likeCount or post?.dislikeCount

    $footer.find('.btn.like').click ->
      if post and post.userName != USER_NAME
        $that = $footer.find '.btn.dislike.selected'
        if $that.length
          $that.removeClass 'selected'
          $value = $that.find '.value'
          $value.text -1 + Number $value.text()
        $this = $ @
        $this.parent('.like-group').removeClass 'fade-in'
        selected = $this.hasClass 'selected'
        value = if selected then 0 else 1
        ticket.update
          data:
            login: USER_NAME
            password: USER_PASSWORD
            targetType: 'post'
            targetId: postId
            type: 'like'
            value: value
          success: =>
            $this.toggleClass 'selected'
            $value = $this.find '.value'
            $value.text (if selected then -1 else 1) + Number $value.text()
      false

    $footer.find('.btn.dislike').click ->
      if post and post.userName != USER_NAME
        $that = $footer.find '.btn.like.selected'
        if $that.length
          $that.removeClass 'selected'
          $value = $that.find '.value'
          $value.text -1 + Number $value.text()
        $this = $ @
        $this.parent('.like-group').removeClass 'fade-in'
        selected = $this.hasClass 'selected'
        value = if selected then 0 else -1
        ticket.update
          data:
            login: USER_NAME
            password: USER_PASSWORD
            targetType: 'post'
            targetId: postId
            type: 'like'
            value: value
          success: =>
            $this.toggleClass 'selected'
            $value = $this.find '.value'
            $value.text (if selected then -1 else 1) + Number $value.text()
      false

addPosts = (posts) -> # [object post] ->
  POSTS.push.apply POSTS, posts
  document.title = "#{PAGE_TITLE} (#{POSTS.length})"
  h = (renderPost it for it in posts when it.type is 'post').join ''
  $('.topic > .posts').append h
  #$(h).hide().appendTo('.topic > .posts').fadeIn()

  replies = (it for it in posts when it.type is 'reply')
  if replies.length
    replies = _.sortBy replies, (it) -> it.createTime
    for it in replies
      $ref = $getPost it.replyId
      if $ref.length
        h = renderPost it
        $ref.children('.reply').append h
      else
        dprint 'addPosts: error: post lost'

  bindNewPosts()

highlightNewPosts = -> $('.post.post-new').effect 'highlight', HIGHLIGHT_INTERVAL

addPost = (post) -> # object post ->
  POSTS.push post
  document.title = "#{PAGE_TITLE} (#{POSTS.length})"
  if post.type is 'post'
    h = renderPost post
    $(h).prependTo '.topic > .posts'
        #.effect 'highlight', HIGHLIGHT_INTERVAL
    highlightNewPosts()
    bindNewPosts()
  else if post.type is 'reply'
    $ref = $getPost post.replyId
    if $ref.length
      h = renderPost post
      $(h).appendTo($ref.children('.reply'))
          #.effect 'highlight', HIGHLIGHT_INTERVAL
      highlightNewPosts()
      bindNewPosts()
    else
      dprint 'addPost: error: post lost'
  else
    dprint 'addPost: error: unknown post type'

updatePost = (post) -> # object post ->
  oldpost = findPost post.id
  if oldpost
    util.fillObject oldpost, post
    $post = $getPost post.id
    if $post.length
      $h = $ renderPost post
      $h.children('.reply').replaceWith $post.children '.reply'

      $post.replaceWith $h

      #$post = $getPost post.id
      #$post.children('reply').replaceWith $reply

      #$h.effect 'highlight', HIGHLIGHT_INTERVAL
      highlightNewPosts()
      bindNewPosts()
      return

  dprint 'updatePost: error: post lost'

# AJAX actions

spin = (t) -> $('#spin').spin if t then 'large' else false

paint = ->
  spin true
  rest.forum.list 'post',
    data:
      topic: TOPIC_ID
      sort: 'updateTime'
      asc: false
      limit: POST_LIMIT
    error: ->
      spin false
      growl.warn tr 'Internet error'
    success: (data) ->
      spin false
      if data.length
        addPosts data
      else
        growl.warn tr 'Internet error'

more = ->
  spin true
  rest.forum.list 'post',
    data:
      topic: TOPIC_ID
      sort: 'updateTime'
      asc: false
      first: POSTS.length
      limit: POST_LIMIT
    error: ->
      spin false
      growl.warn tr 'Internet error'
    success: (data) ->
      spin false
      if data.length
        addPosts data
      else
        growl tr "No more"

bind = ->
  $('.topic > .footer > .btn-more').click ->
    $this = $ @
    unless $this.data 'locked'
      $this.data 'lock', true
      more()
      $this.data 'lock', false
    false

## Main ##

@READY = false # needed by chatview.py
@addPost = addPost
@updatePost = updatePost

init = ->
  unless @i18nBean? # the last injected bean
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    moment.locale 'ja'

    #@TOPIC_ID = $('.topic').data 'id' # global game item id

    createTemplates()

    bind()
    paint()

    @READY = true

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF
