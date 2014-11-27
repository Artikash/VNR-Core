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
  @HTML_EMPTY = "<div class='empty'>(#{tr 'Empty'})</div>"
  @HTML_NOMORE = "<div class='empty'>(#{tr 'No more'})</div>"

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
    %a(href="${userAvatarUrl}" title="#{tr 'Avatar'}")
      %img.img-circle.avatar(src="${userAvatarUrl}" alt="#{tr 'Avatar'}")
  .right
    .header
      %a.item.title(href='javascript:' title="#{tr 'Browse'}") ${title}
      %span.pull-right
        %a.item.user(href="javascript:") @${userName}
        .item.text-minor = lang
        .item.text-minor = createTime
        .item.text-success = updateTime
    :if scores
      .score
        .pp-table.dock
          :if scores.overall != undefined
            .pp-row(data-type='overall')
              .pp-name #{tr 'Score'}:
              .pp-value ${scores.overall}/10
          :if scores.ecchi != undefined
            .pp-row(data-type='ecchi')
              .pp-name H:
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
USER_TOPIC = null # object

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

newTopic = (type) -> topicInputBean.newTopic type # string ->

replyTopic = (topicId) -> topicInputBean.replyTopic topicId # long ->

bindNewTopics = ->
  $('.topic.topic-new').each ->
    $topic = $ @
      .removeClass 'topic-new'

    topicId = $topic.data 'id'
    topic = findTopic topicId

    $header = $topic.find '> .right > .header'
    $footer = $topic.find '> .right > .footer'

    $header.find('a.user').click ->
      mainBean.showUser topic.userName if topic?.userName
      false

    $footer.find('.btn-edit').click ->
      editTopic topic if topic
      false

    $footer.find('.btn-reply').click ->
      replyTopic topic
      false
    $header.find('.title').click ->
      replyTopic topic
      false

    $footer.find('.like-group').removeClass 'fade-in' if topic?.likeCount or topic?.dislikeCount

    $footer.find('.btn.like').click ->
      if topic and USER_NAME and USER_NAME isnt topic.userName
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
            targetType: 'topic'
            targetId: topicId
            type: 'like'
            value: value
          success: =>
            $this.toggleClass 'selected'
            $value = $this.find '.value'
            $value.text (if selected then -1 else 1) + Number $value.text()
      false

    $footer.find('.btn.dislike').click ->
      if topic and USER_NAME and USER_NAME isnt topic.userName
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
            targetType: 'topic'
            targetId: topicId
            type: 'like'
            value: value
          success: =>
            $this.toggleClass 'selected'
            $value = $this.find '.value'
            $value.text (if selected then -1 else 1) + Number $value.text()
      false

addTopics = (topics) -> # [object topic] ->
  topics = _.filter topics, (it) -> it.type is 'review'
  return unless topics.length

  TOPICS.push.apply TOPICS, topics
  document.title = "#{PAGE_TITLE} (#{TOPICS.length})"

  if USER_NAME
    userTopic = _.findWhere topics, userName:USER_NAME
    if userTopic
      USER_TOPIC = userTopic
      console.log 'addTopics: found user topic'
      topics = _.without topics, userTopic
      repaintUserTopic()

  if topics.length
    h = (renderTopic it for it in topics when it.type is 'review').join ''
    $('.topics').append h
  #$(h).hide().appendTo('.topics').fadeIn()
  bindNewTopics()
  repaintMoreButton()

highlightNewTopics = -> $('.topic.topic-new').effect 'highlight', HIGHLIGHT_INTERVAL

addTopic = (topic) -> # object topic ->
  return unless topic.type is 'review'

  TOPICS.push topic
  document.title = "#{PAGE_TITLE} (#{TOPICS.length})"

  if topic.userName is USER_NAME
    USER_TOPIC = topic
    repaintUserTopic()
  else
    h = renderTopic topic
    $('.topics > .middle').after h
  highlightNewTopics()
  bindNewTopics()

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

repaintUserTopic = ->
  return unless USER_TOPIC
  h = renderTopic USER_TOPIC
  $ h
    .addClass '.topic-user'
    .replaceAll '.topic-user'

repaintMoreButton = ->
  if TOPICS.length < TOPIC_LIMIT or TOPICS.length % TOPIC_LIMIT > (if USER_TOPIC then 1 else 0)
    h = if TOPICS.length then HTML_NOMORE else HTML_EMPTY
    $('.sec-topic .btn-more').replaceWith h

# AJAX actions

spin = (t) -> $('#spin').spin if t then 'large' else false

show = -> # invoked only once
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
        _showUserTopic if USER_NAME and not USER_TOPIC and TOPICS.length is TOPIC_LIMIT
      else
        repaintMoreButton()
        #growl tr('Empty') + ' > <'

_showUserTopic = ->
  spin true
  rest.forum.list 'topic',
    data:
      subjectId: GAME_ID
      subjectType: 'game'
      userName: USER_NAME
      type: 'review'
      limit: 1
      complete: true
    error: ->
      spin false
      growl.warn tr 'Internet error'
    success: (data) ->
      spin false
      addTopic data[0] if data.length is 1

more = ->
  spin true
  rest.forum.list 'topic',
    data:
      subjectId: GAME_ID
      subjectType: 'game'
      sort: 'updateTime'
      asc: false
      complete: true
      first:TOPICS.length - (TOPICS.length % TOPIC_LIMIT)
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
  $('.sec-topic .btn-more').click ->
    $this = $ @
    unless $this.data 'locked'
      $this.data 'lock', true
      more()
      $this.data 'lock', false
    false

  $('.sec-topic .btn-new').click ->
    newTopic 'review' # hard code type is review
    false

  $('.sec-btn').click ->
    $this = $ @
    $sec = $this.parent '.sec'
    if $sec.length
      $target = $sec.find '.sec-content'
      unless $target.is(':empty') and $this.hasClass('checked')
        $this.toggleClass 'checked'
        #effect = $this.data('effect') or 'blind'
        unless $target.is ':empty'
          $target.toggle 'blind'
          $sec.find('.sec-footer').toggle 'blind'
    true

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
    show()

    @READY = true

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF
