# posts.coffee
# 12/30/2014 jichi
# Required by chat.haml
#
# Require
# - jquery
# - haml
# - underscore
#
# Beans:
# - mainBean: coffeebean.MainBean
# - postInputBean: postinput.PostInputManagerBean
# - postEditBean: postedit.PostEditorManagerBean

dprint = ->
  l = Array::slice.call arguments
  l.unshift 'posts:'
  console.log.apply console, l

# Global variables

INIT_POST_COUNT = 10
MORE_POST_COUNT = 20

HIGHLIGHT_INTERVAL = 1500

HAML_POST = null
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
  HAML_POST = Haml """\
.post.post-new(data-id="${id}" data-type="${type}")
  :if userAvatarUrl
    %a(href="${userAvatarUrl}" title="#{tr 'Avatar'}")
      %img.img-circle.avatar(src="${userAvatarUrl}" alt="#{tr 'Avatar'}")
  .right
    .header
      %a.user(href="javascript:" style="${userStyle}") @${userName}
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

# Functions and classes

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

editPost = (post) -> postEditBean.editPost JSON.stringify post # long ->
replyPost = (postId) -> postInputBean.replyPost postId # long ->

# Classes
#
class PostList

  constructor: (container: @$sel, more:$more, topicId:@topicId) ->
    @posts = [] # [object post]

    # bind
    self = @
    $more?.click ->
      $this = $ @
      unless $this.data 'locked'
        $this.data 'lock', true
        self.more()
        $this.data 'lock', false
      false

    @paint()

  # Helper functions

  $getPost: (postId) =>  @$sel.find ".post[data-id=#{postId}]" # long -> $el
  getPost: (postId) => _.findWhere @posts, id:postId # long -> object

  _bindNewPosts: =>
    self = @
    @$sel.find('.post.post-new').each ->
      $post = $ @
        .removeClass 'post-new'

      postId = $post.data 'id'
      post = self.getPost postId

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
        if post and USER_NAME and USER_NAME isnt post.userName
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
        if post and USER_NAME and USER_NAME isnt post.userName
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

  addPosts: (posts) => # [object post] ->
    @posts.push.apply @posts, posts
    #document.title = "#{PAGE_TITLE} (#{@posts.length})"
    h = (renderPost it for it in posts when it.type is 'post').join ''
    @$sel.append h

    replies = (it for it in posts when it.type is 'reply')
    if replies.length
      replies = _.sortBy replies, (it) -> it.createTime
      for it in replies
        $ref = @$getPost it.replyId
        if $ref.length
          h = renderPost it
          $ref.children('.reply').append h
        else
          dprint 'addPosts: error: post lost'

    @_bindNewPosts()

  _highlightNewPosts: =>
    @$sel.find '.post.post-new'
      .effect 'highlight', HIGHLIGHT_INTERVAL

  addPost: (post) => # object post ->
    @posts.push post
    #document.title = "#{PAGE_TITLE} (#{@posts.length})"
    if post.type is 'post'
      h = renderPost post
      @$sel.prepend h
      @_highlightNewPosts()
      @_bindNewPosts()
    else if post.type is 'reply'
      $ref = @$getPost post.replyId
      if $ref.length
        h = renderPost post
        $ref.children('.reply').append h
        @_highlightNewPosts()
        @_bindNewPosts()
      else
        dprint 'addPost: error: post lost'
    else
      dprint 'addPost: error: unknown post type'

  updatePost: (post) => # object post ->
    oldpost = @getPost post.id
    if oldpost
      util.fillObject oldpost, post
      $post = @$getPost post.id
      if $post.length
        $h = $ renderPost post
        $h.children('.reply').replaceWith $post.children '.reply'

        $post.replaceWith $h

        #$post = @$getPost post.id
        #$post.children('reply').replaceWith $reply

        #$h.effect 'highlight', HIGHLIGHT_INTERVAL
        @_highlightNewPosts()
        @_bindNewPosts()
        return

    dprint 'updatePost: error: post lost'

  # AJAX actions

  paint: =>
    self = @
    spin true
    rest.forum.list 'post',
      data:
        topic: @topicId
        sort: 'updateTime'
        asc: false
        limit: INIT_POST_COUNT
      error: ->
        spin false
        growl.warn tr 'Internet error'
      success: (data) ->
        spin false
        if data.length
          self.addPosts data
        else
          growl.warn tr 'Internet error'

  more: =>
    self = @
    spin true
    rest.forum.list 'post',
      data:
        topic: @topicId
        sort: 'updateTime'
        asc: false
        first: @posts.length
        limit: MORE_POST_COUNT
      error: ->
        spin false
        growl.warn tr 'Internet error'
      success: (data) ->
        spin false
        if data.length
          self.addPosts data
        else
          growl tr "No more"

## Export ##

init = ->
  createTemplates()

@postsjs =
  init: init
  PostList: PostList

# EOF
