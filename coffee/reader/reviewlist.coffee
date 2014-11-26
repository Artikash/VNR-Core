# chat.coffee
# 8/29/2014 jichi
# Required by chat.haml
#
# Beans:
# - cacheBean: cacheman.CacheCoffeeBean
# - i18nBean: coffeebean.I18nBean
# - mainBean: coffeebean.MainBean
# - topicInputBean: topicinput.TopicInputManagerBean
# - topicEditBean: topicedit.TopicEditorManagerBean

dprint = -> console.log.apply console, arguments
#dprint = ->
timer = -> new choco.Timer arguments ...

TOPIC_LIMIT = 20

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
  @HAML_TOPIC = Haml """\
.topic.topic-new(data-id="${id}" data-type="${type}")
  :if userAvatarUrl
    %img.img-circle.avatar(src="${userAvatarUrl}")
  .right
    .header
      %a.item.title(title="#{tr 'Browse'}") ${title}
      %span.pull-right
        %a.item @${userName}
        .item.text-minor = lang
        .item.text-minor = createTime
        .item.text-success = updateTime
    :if scores
      .score
        .pp-table.dock
          :if scores.overall != undefined
            .pp-row(data-type='overall')
              .pp-name #{tr 'Overall'}:
              .pp-value ${scores.overall}/10
          :if scores.ecchi != undefined
            .pp-row(data-type='ecchi')
              .pp-name #{tr 'Ecchi'}:
              .pp-value ${scores.ecchi}/10
    .content.bbcode = content
    .footer
      .btn-group.like-group
        %a.like.btn.btn-link.btn-sm(role="button" title="#{tr 'Like'}" data-value="${likeCount}")
          %span.fa.fa-thumbs-up
          %span.value = likeCount
        %a.dislike.btn.btn-link.btn-sm(role="button" title="#{tr 'Dislike'}" data-value="${dislikeCount}")
          %span.fa.fa-thumbs-down
          %span.value = dislikeCount
      .btn-group
        %a.btn-reply.btn.btn-link.btn-sm(role="button" title="#{tr 'Reply'}" title="#{tr 'Browse'}")
          #{tr 'Reply'}
          :if postCount
            = ' (' + postCount + ')'
      :if userName == USER_NAME
        .btn-group.pull-right
          %a.btn-edit.btn.btn-link.btn-sm(role="button" title="#{tr('Edit')}") = tr('Edit')
    :if image
      .image
        %a(href="${image.url}" title="${image.url}")
          %img(src="${image.url}" alt="${image.title}")
""".replace /\$/g, '#'

TOPICS = [] # [object topic]

renderTopic = (data) -> # object topic -> string
  HAML_TOPIC
    id: data.id
    type: data.type
    userName: data.userName
    userStyle: if data.userColor then "color:#{data.userColor}" else ''
    lang: util.getLangName data.lang
    userAvatarUrl: util.getAvatarUrl data.userAvatar
    title: data.title
    content: util.renderContent data.content
    createTime: util.formatDate data.createTime
    updateTime: if data.updateTime > data.createTime then util.formatDate data.updateTime else ''
    image: if data.image then {title:data.image.title, url:util.getImageUrl data.image} else null
    likeCount: data.likeCount or 0
    dislikeCount: data.dislikeCount or 0
    postCount: data.postCount
    scores: data.scores

$getTopic = (topicId) ->  $ ".topic[data-id=#{topicId}]" # long -> $el

findTopic = (id) -> _.findWhere TOPICS, id:id # long -> object

editTopic = (topic) -> topicEditBean.editTopic JSON.stringify topic # long ->

replyTopic = (topicId) ->  topicInputBean.replyTopic topicId # long ->

bindNewTopics = ->
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

addTopics = (topics) -> # [object topic] ->
  TOPICS.push.apply TOPICS, topics
  document.title = "#{PAGE_TITLE} (#{TOPICS.length})"
  h = (renderTopic it for it in topics when it.type is 'review').join ''
  $('.topics').append h
  #$(h).hide().appendTo('.topics').fadeIn()
  bindNewToipcs()

highlightNewTopics = -> $('.topic.topic-new').effect 'highlight', HIGHLIGHT_INTERVAL

addTopic = (topic) -> # object topic ->
  TOPICS.push topic
  document.title = "#{PAGE_TITLE} (#{TOPICS.length})"
  if topic.type is 'review'
    h = renderTopic topic
    $(h).prependTo '.topics'
        #.effect 'highlight', HIGHLIGHT_INTERVAL
    highlightNewTopics()
    bindNewTopics()
  else
    dprint 'addTopic: error: unknown topic type'

updateTopic = (topic) -> # object topic ->
  oldtopic = findTopic topic.id
  if oldtopic
    util.fillObject oldtopic, topic
    $topic = $getTopic topic.id
    if $topic.length
      $h = $ renderTopic topic

      $topic.replaceWith $h

      #$topic = $getTopic topic.id
      #$topic.children('reply').replaceWith $reply

      #$h.effect 'highlight', HIGHLIGHT_INTERVAL
      highlightNewTopics()
      bindNewTopics()
      return

  dprint 'updateTopic: error: topic lost'

# AJAX actions

spin = (t) -> $('#spin').spin if t then 'large' else false

paint = ->
  spin true
  rest.forum.list 'topic',
    data:
      subjectId: GAME_ID
      subjectType: 'game'
      type: 'review'
      sort: 'updateTime'
      asc: false
      limit: TOPIC_LIMIT
      complete: true
    error: ->
      spin false
      growl.warn tr 'Internet error'
    success: (data) ->
      spin false
      if data.length
        addTopics data
      else
        growl.warn tr 'Internet error'

more = ->
  spin true
  rest.forum.list 'topic',
    data:
      subjectId: GAME_ID
      subjectType: 'game'
      sort: 'updateTime'
      asc: false
      complete: true
      first: TOPICS.length
      limit: TOPIC_LIMIT
    error: ->
      spin false
      growl.warn tr 'Internet error'
    success: (data) ->
      spin false
      if data.length
        addTopics data
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

@READY = false # needed by the python view
@addTopic = addTopic
@updateTopic = updateTopic

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
