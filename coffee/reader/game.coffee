# game.coffee
# 8/15/2013 jichi
# Required by game.haml
#
# Beans:
# - gameBean: gameview.GameBean
# - cacheBean: cacheman.CacheCoffeeBean
# - clipBean: skwebkit.SkClipboardProxy
# - i18nBean: coffeebean.I18nBean
# - ttsBean: ttsman.TtsCoffeeBean
# - yakuBean: trman.YakuCoffeeBean
# - youtubeBean: coffeebean.YouTubeBean
# - viewBean: skwebkit.SkViewBean
# - shioriBean: kagami.ShioriBean -- TODO: rename to dicBean
# - yomiBean: mecab.MeCabCoffeeBean -- TODO: rename to jlpBean

#dprint = -> console.log.apply console, arguments
dprint = ->
timer = -> new choco.Timer arguments ...

HOST = 'http://sakuradite.com'

#bind = ->
#  #$('[title]').tooltip() # use bootstrap tooltip

#defer = (interval, fn) -> setTimeout fn, interval
#

DEFAULT_COVER_IMAGE_WIDTH = 230
DEFAULT_SAMPLE_IMAGE_WIDTH = 220
DEFAULT_VIDEO_IMAGE_WIDTH = 220

INVALID_YT_IMG_WIDTH = 120 # invalid youtube thumbnail image width

SCAPE_REVIEW_INIT_SIZE = 10
SCAPE_REVIEW_PAGE_SIZE = 20
SCAPE_REVIEW_MAX_LENGTH = 200 # max content length

HIGHLIGHT_INTERVAL = 1000 # int msecs

# Global translation function
@tr = (text) -> i18nBean.tr text # string -> string
getLangName = (lang) -> i18nBean.getLangShortName lang # string -> string

cacheimg = (url) -> cacheBean.cacheImage url # string -> string
cacheurl = (url) -> cacheBean.cacheUrl url # string -> string

# Delay template creation until i18nBean becomes available
createTemplates = ->
  @HTML_EMPTY = Haml.render ".empty #{'(' + tr('Empty') + ')'}"

  # HAML for sample images
  # - param  url
  @HAML_SAMPLE_IMAGE = Haml '''\
%a(href="#{url}" title="#{url}")
  %img.img-rounded.zoom.zoom-cg(src="#{url}")
'''

  # HAML for youtube video
  # - param  vid
  # - param  date  string or null
  # - param  title
  # - param  img  string url
  @HAML_VIDEO = Haml """\
.video(data-id="${vid}")
  .header
    %a.tts(role="button" data-text="${title}" title="TTS")
      %span.fa.fa-volume-down
    %a.title.cursor-pointer(title="#{tr 'Play'}") = title
    :if date
      .date = date
    .toolbar.pull-right
      %button.close(type="button" title="#{tr 'Close'}") &times;
      %a.btn.btn-link.btn-xs(role="button" href="http://youtube.com/watch?v=${vid}" title="http://youtube.com/watch?v=${vid}")
        %span.fa.fa-external-link
  .image
    %img.img-rounded(src="${img}" title="${title} ${date}")
  .iframe
""".replace /\$/g, '#'

  # HAML for characters
  # - param  name
  # - param  yomi  string or null
  # - param  label  string or null
  # - param  cv  string or null
  # - param  img  string url or null
  #  @HAML_CHARA = Haml '''\
  #.chara
  #  .header
  #    :if it.label == "主人公" || label == "【主人公】"
  #      .text-danger(title="#{yomi}") = name
  #    :else
  #      .text-default(title="#{yomi}") = name
  #  :if img
  #    .body: %a(href="#{img}" title="#{it.name} #{label}")
  #      %img.img-rounded(src="#{img}")
  #  .footer
  #    :if cv
  #      .text-info = cv
  #    :else
  #      %span.muted (CV無し)
  #'''

  # HAML for game.file
  # - param file
  #   - encoding
  #   - hook
  #   - threads
  #     - name
  #       - name
  #       - sig
  #     - scene
  #       - name
  #       - sig
  #
  @HAML_SETTINGS = Haml '''\
.body.form-horizontal
  :if file.encoding
    .row.encoding
      %label.control-label.col-xs-2 = tr('Encoding')
      .form-control-static.col-xs-10 = file.encoding.toUpperCase()
  :if file.threads
    :if file.threads.scene
      .row.thread
        %label.control-label.col-xs-2 対話
        .form-control-static.col-xs-10
          = file.threads.scene.name
          = ' '
          = file.threads.scene.sig.toString(16)
    :if file.threads.name
      .row.thread
        %label.control-label.col-xs-2 名前
        .form-control-static.col-xs-10
          = file.threads.name.name
          = ' '
          = file.threads.name.sig.toString(16)
  .row.hcode
    %label.control-label.col-xs-2 = tr('H-code')
    .form-control-static.col-xs-10
      :if file.hook
        %span.text-danger = file.hook.text
        :if file.hook.locked
          %b.text-info = ' (' + tr('Locked') + ')'
          %i.text-muted.text-xs
            = ' -- '
            = tr('This h-code is definite correct and not allowed to change')
      :else
        %span.text-muted = tr('Not specified')
'''

  # HAML for game.file
  # - param users
  #   [user]
  #   - userAvatarUrl
  #   - userName
  #   - langName
  #   - count
  @HAML_USERS = Haml """\
:for it in users
  :if it.userName && it.count
    .user(data-name="${it.userName}")
      :if it.userAvatarUrl
        %a.user-link(title="#{tr 'Show'}")
          %img.img-circle.avatar(src="${it.userAvatarUrl}")
      .header
        %a.user-link(title="#{tr 'Show'}") @${it.userName}
      .footer ${it.count} / ${it.langName}
""".replace /\$/g, '#'

  # Haml for scape container
  # - entries  html
  # - empty  bool
  @HAML_SCAPE_REVIEWLIST = Haml """\
.list = entries
.footer
  :if empty
    .empty #{'(' + tr('No more') + ')'}
  :else
    %button.btn-more.btn.btn-info(type="button" title="#{tr 'More'}") #{tr 'More'}
"""

  # Haml for single scape review
  # - count  int
  # - user
  # - userUrl
  # - date  nullable
  # - title  nullable
  # - content  nullable
  # - contentLength  int
  # - lessContent  nullable
  # - netabare  bool
  # - score  int
  # - ecchiScore  int
  @HAML_SCAPE_REVIEW = Haml """\
.entry.entry-new(data-content-length="${contentLength}")
  .header
    %span.count = count
    %a.user(href="${userUrl}" title="${userUrl}") @${user}
    :if netabare
      %span.netabare.text-danger = '(ネタバレ)'
    :if ecchiScore
      %span.score.text-info H:${ecchiScore}/5
    :if score
      %span.score.text-danger ${score}/100
    :if date
      %span.date.text-success = date
  .body
    :if title
      .title(title="一言コメント") = title
    :if content
      :if lessContent
        .content.content-more.inactive(title="メモ") = content
        .content.content-less(title="メモ") = lessContent
        %a.btn-more.btn.btn-link(role="button" title="#{tr 'More'}") = "#{tr 'More'} (" + contentLength + ")"
      :else
        .content(title="メモ") = content
""".replace /\$/g, '#'

## Render ##

createTwitterTimeline = (id:id, el:el, callback:callback, options:options) ->
  twttr.widgets.createTimeline id, el, callback, options #if window.twttr

_renderSampleImage = (url) ->
  HAML_SAMPLE_IMAGE url:url

renderSampleImages = (type) -> # -> int count, string html
  l = gameBean.getSampleImages(type).split ','
  [l.length, l.map(_renderSampleImage).join('')]

_renderVideo = (params) ->
  params.date = '' unless params.date? # fill in the missing date
  params.img = gameBean.getYouTubeImageUrl params.vid, true # large = true
  HAML_VIDEO params

renderVideos = -> # -> string  html
  JSON.parse(gameBean.getVideos()).map(_renderVideo).join ''

renderVideoIframe = (vid) -> # string -> string
  """<iframe width="480" height="360" src="http://youtube.com/embed/#{vid}" frameborder="0" allowfullscreen />"""

_renderCharacter = (params) ->
  # I might need some fixes here
  HAML_CHARA params

renderCharacters = (type) -> # string -> string
  JSON.parse(gameBean.getCharacters type).map(_renderCharacter).join ''

renderSettings = (file) -> # game.file
  HAML_SETTINGS file:file

renderUsers = (users) -> # game.file
  for it in users
    #it.url = "#{HOST}/user/#{it.userName}"
    it.langName = getLangName(it.lang) or '*'
    it.userAvatarUrl = cacheimg "http://media.getchute.com/media/#{it.userAvatar}/128x128"
  HAML_USERS users:users

renderReview = (type) -> # string -> string
  if type is 'scape'
    renderScapeReviewList()
  else
    gameBean.getReview type

_renderScapeContent = (t) -> t?.replace /\n/g, '<br/>' # string -> string

SCAPE_REVIEW_COUNT = 0 # current count
_renderScapeReview = (review)-> # -> string
  ++SCAPE_REVIEW_COUNT
  try
    ecchiScore = 0
    if review.okazu_tokuten and review.okazu_tokuten < 0 and review.okazu_tokuten > -10
      ecchiScore = -review.okazu_tokuten # scores are negative, invalid score is -999

    content = lessContent = ''
    if review.memo
      content = _renderScapeContent review.memo
      lessContent = _renderScapeContent review.memo[..SCAPE_REVIEW_MAX_LENGTH] + " ……"

    HAML_SCAPE_REVIEW
      count: SCAPE_REVIEW_COUNT
      user: review.uid
      userUrl: "http://erogamescape.dyndns.org/~ap2/ero/toukei_kaiseki/user_infomation.php?user=#{review.uid}"
      date: review.timestamp
      netabare: review.netabare
      score:  review.tokuten or 0
      ecchiScore: ecchiScore
      title: _renderScapeContent review.hitokoto
      content: content
      contentLength: review.memo?.length
      lessContent: lessContent

  catch # catch in case type error
    '<div class="entry entry-empty"/>'

renderScapeReviews = (offset, limit) -> # int, int -> string, bool empty
  data = gameBean.getScapeReviews offset, limit
  unless data
    ['', 0]
  else
    data = JSON.parse data
    h = data.map(_renderScapeReview).join ''
    [h, data.length]

renderScapeReviewList = -> # -> string
  [h, count] = renderScapeReviews 0, SCAPE_REVIEW_INIT_SIZE
  if h
    HAML_SCAPE_REVIEWLIST
      entries: h
      empty: count < SCAPE_REVIEW_INIT_SIZE

## Bindings ##

#@initAmazonReview = ->
#  dprint 'initAmazonReview: enter'
#  #setTimeout(iframe, 3000);
#  # Load function does not work on IE
#  # See: http://stackoverflow.com/questions/4548984/detect-if-the-iframe-content-has-loaded-successfully
#  $iframe = $ 'iframe.amazon'
#  $iframe.load ->
#    $this = $ @
#    $body = $this.contents().find 'body'
#    unless $body.html()
#      $this.remove() # in case of 403 error
#    else
#      $body.css 'background-color', 'transparent' # transparent background
#           .find('.crIFrame').css 'margin-left', '-0.8em' # remove left margin
#      $body.find('.crIFrameLogo,.crIframeHeaderTitle').remove() # remove amazon logo and titles
#      #$body.find('crIFrameReviewList').addClass 'ruby'
#
#    # http://stackoverflow.com/questions/217776/how-to-apply-css-to-iframe
#    #$head = $("iframe.amazon").contents().find 'head'
#    #url = "#{rc.cdn_url('iframe.css')}"
#    #$head.append($('<link/>',
#    #  {rel: 'stylesheet', type: 'text/css', href: url}
#    #));
#
#  $(window).on 'scroll resize', ->
#    # http://stackoverflow.com/questions/10083399/change-iframe-attribute-with-jquery
#    # http://stackoverflow.com/questions/14913784/change-iframe-width-and-height-using-jquery
#    #w = $window.width() # incorrect orz
#    w = @viewBean.width()
#    $iframe.width w - 74 # margin = 550 - 480 +4
#    #$iframe[0].setAttribute 'width', w
#    #$iframe.attr 'width', w
#
#  $('iframe.amazon').error -> $(@).remove() # cannot detect 403 error, though
#  dprint 'initAmazonReview: leave'

#@bindGetchu = ->
#  dprint 'bindGetchu: enter'
#  $iframe = $ 'iframe.getchu'
#
#  $(window).on 'scroll resize', ->
#    #w = $window.width() # incorrect orz
#    w = @viewBean.width()
#    $iframe.width w - 74 # margin = 550 - 480 +4
#
#  $('iframe.getchu').error -> $(@).remove() # on 404 error?
#  dprint 'bindGetchu: leave'

## Zoom ##

# Must be consistent with game.sass

ZOOM_FACTOR = 1

class Zoomer
  constructor: (@viewBean) ->
    @ratio = 1.0 # float
    @bind()

  # Properties
  maxWidth: => Math.max 0, @viewBean.width() - 90

  # Actions

  bind: =>
    $(window).resize => @zoomYoutube @ratio

  zoom: (v) => # v float
    ZOOM_FACTOR = @ratio = v
    $('img.zoom.zoom-cover').width DEFAULT_COVER_IMAGE_WIDTH * (v+1)/2
    #$('img.zoom.zoom-cg').width v * DEFAULT_SAMPLE_IMAGE_WIDTH
    #$('.chara').width v * 100 # TO BE RESTOERD

    $cg = $('img.zoom.zoom-cg')
    if $cg.length
      $cg.width 220 * v
      $('section.cg .images:visible').masonry() #columnWidth: v * DEFAULT_SAMPLE_IMAGE_WIDTH

    @zoomYoutube v

  youtubeWidth: (v) => Math.min((v ? @ratio) * DEFAULT_VIDEO_IMAGE_WIDTH, @maxWidth())
  zoomYoutube: (v) => # float
    $('.youtube:not(.iframe)').width @youtubeWidth * v

  youtubeFrameWidth: =>
    Math.min @ratio * 480, (Math.max 480, @maxWidth()) # at least 480px
  youtubeFrameHeight: =>
    @youtubeFrameWidth() * 9 / 16.0 # at least 270px

initToolbar = ->
  dprint 'bindToolbar: enter'
  @zoomer = new Zoomer @viewBean

  slider = new ZoomSlider (y) => @zoomer.zoom 1 + y * 2

  toolbar = new Toolbar @viewBean,
    width: 60   # height of slider + margin-right
    height: 229 + 15# height of slider + 3 * button + margin-bottom + slider height
    move: slider.reloadOffset
  dprint 'bindToolbar: leave'

## Ruby Furigana ##

initRuby = ->
  dprint 'bindRuby: enter'

  $.fn.inject = -> # create inject plugin
      @each -> window.injectruby @
  $('.ruby').inject()

  dprint 'bindRuby: leave'

## Spin ##

initSpin = ->
  $.fn.spin.presets.section = # preset for section
    left: '-8px'
    lines: 8
    length: 4
    width: 2
    radius: 4

## Bootstrap Switch ##

initSwitches = ->
  initReviewSwitch()
  initSettingsSwitch()
  initUsersSwitch()
  initTwitterSwitch()
  initYouTubeSwitch()

initSettingsSwitch = ->
  $section = $ 'section.settings'
  $container = $section.find '.contents'
  $spin = $section.find '.spin'
  $msg = $section.find '.msg'
  $section.find('input.switch').bootstrapSwitch()
    .on 'switchChange.bootstrapSwitch', (event, checked) ->
      unless checked
        $container.fadeOut()
      else if $container.hasClass 'rendered'
        $container.fadeIn()
      else
        $container.show()
           .addClass 'rendered'

        $spin.spin 'section'

        rest.forum.query 'game',
          data:
            id: GAME_ID
            select: 'file'
          error: ->
            $spin.spin false
            $container.removeClass 'rendered'
            $msg.addClass 'text-danger'
                .text "(Internet #{tr 'error'})"
          success: (data) ->
            $spin.spin false
            if data.file
              $msg.empty()
              h = renderSettings data.file
              $container
                .hide()
                .html h
                .fadeIn()
            else
              $msg.removeClass 'text-danger'
                  .text " (#{tr 'Empty'})"

initUsersSwitch = ->
  $section = $ 'section.users'
  $container = $section.find '.contents'
  $spin = $section.find '.spin'
  $msg = $section.find '.msg'
  $section.find('input.switch').bootstrapSwitch()
    .on 'switchChange.bootstrapSwitch', (event, checked) ->
      unless checked
        $container.fadeOut()
      else if $container.hasClass 'rendered'
        $container.fadeIn()
      else
        $container.show()
           .addClass 'rendered'

        $spin.spin 'section'

        rest.forum.query 'game',
          data:
            id: GAME_ID
            select: 'users'
          error: ->
            $spin.spin false
            $container.removeClass 'rendered'
            $msg.addClass 'text-danger'
                .text "(Internet #{tr 'error'})"
          success: (data) ->
            $spin.spin false
            users = data.users or data.subs
            if users
              $msg.empty()
              h = renderUsers users
              $container
                .hide()
                .html h
                .fadeIn()
                # bind click
                .find('a.user-link').click ->
                  name = $(@).closest('.user').data 'name'
                  mainBean.showUser name
            else
              $msg.removeClass 'text-danger'
                  .text " (#{tr 'Empty'})"

initReviewSwitch = ->
  $section = $ 'section.stats'
  $container = $section.find '.contents'
  $spin = $section.find '.spin'
  $msg = $section.find '.msg'
  $section.find('input.switch').bootstrapSwitch()
    .on 'switchChange.bootstrapSwitch', (event, checked) ->
      unless checked
        $container.fadeOut()
      else if $container.hasClass 'rendered'
        $container.fadeIn()
      else
        $container.show()
           .addClass 'rendered'

        $spin.spin 'section'

        rest.forum.query 'game',
          data:
            id: GAME_ID
            select: 'users'
          error: ->
            $spin.spin false
            $container.removeClass 'rendered'
            $msg.addClass 'text-danger'
                .text "(Internet #{tr 'error'})"
          success: (data) ->
            $spin.spin false
            users = data.users or data.subs
            if users
              $msg.empty()
              h = renderUsers users
              $container
                .hide()
                .html h
                .fadeIn()
            else
              $msg.removeClass 'text-danger'
                  .text " (#{tr 'Empty'})"

#initCGSwitch = ->
#  $section = $ 'section.cg'
#  $container = $section.find '.images'
#  $section.find('input.switch').bootstrapSwitch()
#    .on 'switchChange.bootstrapSwitch', (event, checked) ->
#      unless checked
#        $container.fadeOut()
#      else if $container.hasClass 'rendered'
#        $container.fadeIn()
#                  .masonry
#      else
#        $container.hide()
#           .html renderSampleImages()
#           .addClass 'rendered'
#           .fadeIn()
#           .masonry
#             itemSelector: 'img'
#             isFitWidth: true # centerize
#           #.imagesLoaded ->
#           #  $container.find('img').width DEFAULT_SAMPLE_IMAGE_WIDTH * ZOOM_FACTOR
#           #  $container.masonry() # refresh after images are loaded
#        $container.find('img').load ->
#          # Saample bad DMM image: http://pics.dmm.com/mono/movie/n/now_printing/now_printing.jpg
#          if ~@src.indexOf('pics.dmm.') and @naturalWidth is 90 and @naturalHeight is 122
#            #@parentNode.removeChild @ # remove this
#            $container.masonry 'remove', @ # remove this
#          else
#            @width = DEFAULT_SAMPLE_IMAGE_WIDTH * ZOOM_FACTOR
#          $container.masonry() # refresh after images are loaded

initTwitterSwitch = ->
  $section = $ 'section.twitter'
  $container = $section.find '.widgets'
  $section.find('input.switch').bootstrapSwitch()
    .on 'switchChange.bootstrapSwitch', (event, checked) ->
      if window.twttr
        unless checked
          $container.fadeOut()
        else if $container.hasClass 'rendered'
          $container.fadeIn()
        else
          $container.show()
             .addClass 'rendered'
          l = gameBean.getTwitterWidgets()
          if l
            for id in l.split(',')
              el = document.createElement 'div'
              el.className = 'timeline'
              $container.append el
              createTwitterTimeline
                id: id
                el: el
                options:
                  lang: i18nBean.lang()
                  width: 480 # the same as youtube width
                  height: 500 # smaller than GameView's default height
                  chrome: 'transparent noborders noheader' # nofooter noscrollbar
                  showReplies: true
                  #tweetLimit: 20 # the maximum is 20

initYouTubeSwitch = ->
  $section = $ 'section.youtube'
  $container = $section.find '.videos'
  $section.find('input.switch').bootstrapSwitch()
    .on 'switchChange.bootstrapSwitch', (event, checked) ->
      unless checked
        $container.empty()
      else
        $container.hide()
           .html renderVideos()
           .fadeIn()
        bindYoutube()

bindYoutube = ->
  $videos = $ 'section.youtube .video'
  if $videos.length
    # Image
    $videos.find('.image > img').load ->
      width = @naturalWidth
      if width is INVALID_YT_IMG_WIDTH
        $this = $ @
        vid = $this.closest('.video').data 'id'
        url = gameBean.getYouTubeImageUrl vid, false # large = false
        @src = url
        #$this.parent('.image').addClass 'crop' # image is always not cropped

    # TTS
    $videos.find('.tts').click ->
      ttsBean.speak @dataset.text
    # Iframe
    $videos.find('a.title,img,button.close').click ->
      $video = $(@).closest '.video'
      $img = $video.find 'img'
      $iframe = $video.find 'iframe'
      if $iframe.length # remove iframe
        $video.removeClass 'play'
        $iframe.remove()
      else # add iframe
        $video.addClass 'play'
        vid = $video.data 'id' # youtube id
        h = renderVideoIframe vid
        $video.find('.iframe').html h
      false

## Bootstrap Navigation Pills ##

initPills = ->
  initCGPills()
  #initCharaPills()
  initCharaDescPills()
  initDescPills()
  initReviewPills()

initDescPills = -> # Descriptions
  $sec = $ 'section.descriptions'
  $container = $sec.find '.contents'

  $sec.find('.nav.nav-pills > li > a').click ->
    $li = $(@).parent 'li'
    unless $li.hasClass 'active'
      oldtype = $li.parent('ul').children('li.active')
          .removeClass 'active'
          .data 'type'
      $li.addClass 'active'
      newtype = $li.data 'type'
      if oldtype
        $container.children('.' + oldtype).hide()
      if newtype
        $el = $container.children('.' + newtype)
        if $el.length
          $el.fadeIn()
        else
          h = gameBean.getDescription newtype
          el = document.createElement 'div'
          el.className = newtype + ' description'
          el.innerHTML = h
          #$container.append el
          $(el).hide()
               .appendTo $container
               .fadeIn()
               # Improvement
               .find('a:not([title])').each -> @setAttribute 'title', @href
    false

initCharaDescPills = -> # Descriptions
  $sec = $ 'section.characters'
  $container = $sec.find '.contents'

  $sec.find('.nav.nav-pills > li > a').click ->
    $li = $(@).parent 'li'
    unless $li.hasClass 'active'
      oldtype = $li.parent('ul').children('li.active')
          .removeClass 'active'
          .data 'type'
      $li.addClass 'active'
      newtype = $li.data 'type'
      if oldtype
        $container.children('.' + oldtype).hide()
      if newtype
        $el = $container.children('.' + newtype)
        if $el.length
          $el.fadeIn()
        else
          h = gameBean.getCharacterDescription newtype
          el = document.createElement 'div'
          el.className = newtype + ' character'
          el.innerHTML = h
          #$container.append el
          $(el).hide()
               .appendTo $container
               .fadeIn()
               # Improvement
               .find('a:not([title])').each -> @setAttribute 'title', @href
    false

initReviewPills = -> # Reviews
  $sec = $ 'section.reviews'
  $container = $sec.find '.contents'
  $spin = $sec.find '.spin'

  # TODO: Add ajax spin indicator
  $sec.find('.nav.nav-pills > li > a').click ->
    $li = $(@).parent 'li'
    unless $li.hasClass 'active'
      oldtype = $li.parent('ul').children('li.active')
          .removeClass 'active'
          .data 'type'
      $li.addClass 'active'
      newtype = $li.data 'type'
      if oldtype
        $container.children('.' + oldtype).hide()
      if newtype
        $el = $container.children('.' + newtype)
        if $el.length
          $el.fadeIn()
        else
          $spin.spin 'section'
          h = renderReview newtype
          el = document.createElement 'div'
          el.className = newtype + ' review'
          el.innerHTML = h or HTML_EMPTY
          #$container.append el
          $(el).hide()
               .appendTo $container
               .fadeIn()
          bindNewScapeReviews()
          $spin.spin false
          if newtype is 'scape' and h
            bindScapeReviewList $spin
    false

bindNewScapeReviews = ->
  $('.scape .entry.entry-new').each ->
    $(@).removeClass 'entry-new'
      .find('.btn-more').click ->
        $this = $ @
        $parent = $this.closest '.entry'
        $more = $parent.find '.content.content-more'
        $less = $parent.find '.content.content-less'

        $more.toggleClass 'inactive'
        $less.toggleClass 'inactive'

        contentLength = $parent.data 'content-length'
        more = $more.hasClass 'inactive'
        $this.text "#{tr if more then 'More' else 'Less'} (#{contentLength})"

        $parent.effect 'highlight', HIGHLIGHT_INTERVAL

bindScapeReviewList = ($spin) ->
  #$spinMore = $ '.scape .spin.spin-more' # difficult to position
  $('.scape .footer .btn-more').click ->
    $this = $ @
    $parent = $this.closest '.scape'
    $container = $parent.find '.list'
    #size = $container.children('.entry').length
    size = SCAPE_REVIEW_COUNT
    unless (size - SCAPE_REVIEW_INIT_SIZE) % SCAPE_REVIEW_PAGE_SIZE # FIXME: scape page size is not accurate
      $spin.spin 'section'
      #$spinMore.spin 'tiny'
      [h, count] = renderScapeReviews size, SCAPE_REVIEW_PAGE_SIZE
      $spin.spin false
      #$spinMore.spin false
      if h
        #$container.append h
        $(h).hide()
            .appendTo $container
            .fadeIn()
        bindNewScapeReviews()
        return false
    # Empty
    $this.replaceWith HTML_EMPTY
    false

class MasonryAniPauser
  constructor: (@$container, @paused=false) ->
    @timer = timer 600, @pause # larger than 400

  pause: =>
    unless @paused
      @paused = true
      @$container.masonry
        isAnimated: false # disable animation
        transitionDuration: 0 # set to 0 to disable animation
    @timer.start()

  resume: =>
    @timer.stop()
    if @paused
      @paused = false
      @$container.masonry
        isAnimated: true # default = true, enable animation
        transitionDuration: 400 # default = 400, set to 0 to disable animation

class SpinCounter
  constructor: (@$el, @count=0, @preset='section') ->
    @$el.spin 'section' if @count

  inc: (value=1) =>
    if value > 0
      @$el.spin @preset if @count <= 0
      @count += value

  dec: (value=1) =>
    if value > 0
      @count -= value
      @$el.spin false if @count <= 0

initCGPills = -> # Sample images
  $sec = $ 'section.cg'
  $container = $sec.find '.contents'
  $spin = $sec.find '.spin'

  counter = new SpinCounter $spin

  $sec.find('.nav.nav-pills > li > a').click ->
    $li = $(@).parent 'li'
    unless $li.hasClass 'active'
      oldtype = $li.parent('ul').children('li.active')
          .removeClass 'active'
          .data 'type'
      $li.addClass 'active'
      newtype = $li.data 'type'
      if oldtype
        $container.children('.' + oldtype).hide()
      if newtype
        $el = $container.children('.' + newtype)
        if $el.length
          $el.fadeIn()
             .masonry()
        else
          [count, h] = renderSampleImages newtype
          counter.inc count
          el = document.createElement 'div'
          el.className = newtype + ' images'
          el.innerHTML = h
          #$container.append el
          (($div) ->
            pauser = null
            $div.hide()
                .appendTo $container
                .fadeIn()
                .masonry
                  itemSelector: 'img'
                  isFitWidth: true # centerize
                #.imagesLoaded ->
                #  $container.find('img').width DEFAULT_SAMPLE_IMAGE_WIDTH * ZOOM_FACTOR
                #  $container.masonry() # refresh after images are loaded
                .find('img').load ->
                  counter.dec()
                  # Sample bad DMM image: http://pics.dmm.com/mono/movie/n/now_printing/now_printing.jpg
                  if ~@src.indexOf('pics.dmm.') and @naturalWidth is 90 and @naturalHeight is 122
                    pauser = new MasonryAniPauser $div unless pauser?
                    pauser.pause()
                    #@parentNode.removeChild @ # remove this
                    $div.masonry 'remove', @ # remove this
                  else
                    $(@).width DEFAULT_SAMPLE_IMAGE_WIDTH * ZOOM_FACTOR
                  $div.masonry() # refresh after images are loaded
          ) $ el
    false

#initCGSwitch = ->
#  $section = $ 'section.cg'
#  $container = $section.find '.images'
#  $section.find('input.switch').bootstrapSwitch()
#    .on 'switchChange.bootstrapSwitch', (event, checked) ->
#      unless checked
#        $container.fadeOut()
#      else if $container.hasClass 'rendered'
#        $container.fadeIn()
#                  .masonry
#      else
#        $container.hide()
#           .html renderSampleImages()
#           .addClass 'rendered'
#           .fadeIn()
#           .masonry
#             itemSelector: 'img'
#             isFitWidth: true # centerize
#           #.imagesLoaded ->
#           #  $container.find('img').width DEFAULT_SAMPLE_IMAGE_WIDTH * ZOOM_FACTOR
#           #  $container.masonry() # refresh after images are loaded
#        $container.find('img').load ->
#          # Sample bad DMM image: http://pics.dmm.com/mono/movie/n/now_printing/now_printing.jpg
#          if ~@src.indexOf('pics.dmm.') and @naturalWidth is 90 and @naturalHeight is 122
#            #@parentNode.removeChild @ # remove this
#            $container.masonry 'remove', @ # remove this
#          else
#            @width = DEFAULT_SAMPLE_IMAGE_WIDTH * ZOOM_FACTOR
#          $container.masonry() # refresh after images are loaded


#initCharaPills = -> # Characters
#  $sec = $ 'section.characters'
#  $container = $sec.find '.contents'
#
#  $sec.find('.nav.nav-pills > li > a').click ->
#    $li = $(@).parent 'li'
#    unless $li.hasClass 'active'
#      oldtype = $li.parent('ul').children('li.active')
#          .removeClass 'active'
#          .data 'type'
#      $li.addClass 'active'
#      newtype = $li.data 'type'
#      if oldtype
#        $container.children('.' + oldtype).hide()
#      if newtype
#        $el = $container.children('.' + newtype)
#        if $el.length
#          $el.fadeIn()
#        else
#          h = renderCharacters newtype
#          el = document.createElement 'div'
#          el.className = newtype
#          el.innerHTML = h
#          #$container.append el
#          $(el).hide()
#               .appendTo $container
#               .fadeIn()
#    false

## Ratings ##

initRatings = ->
  $.fn.raty.defaults.path = JQUERY_RATY_PATH
  $.fn.raty.defaults.hints = ['', '', '', '', ''] # empty
  $.fn.raty.round =
    down: .5
    up: 1.0
    full: 1.0

  $('.raty').each ->
    type = @dataset.type
    $(@).raty
      readOnly: true
      half: true
      #score: @topic.scores[$this.data 'type'] / 2

## Bootstrap ##

#initBootstrap = ->
#  $('[title]').tooltip()

## Bindings ##

bindButtons = ->
  $('.btn-dl-img').click -> gameBean.saveImages(); false
  $('.btn-dl-yt').click -> gameBean.saveVideos(); false

bindTts = ->
  $('.tts').click ->
    #tts.speak @getAttribute('data-text'), @getAttribute('data-lang')
    ttsBean.speak @dataset.text #, 'ja'

bindSearch = ->
  $('a.search').click ->
    #tts.speak @getAttribute('data-text'), @getAttribute('data-lang')
    text = @dataset.text or $.trim @textContent
    gameBean.search text

bindDraggable = ->
  $ '.draggable'
    .draggable()
    .find('button.close').click ->
      $(@).closest('.draggable').fadeOut()

## Main ##

init = ->
  unless @gameBean?
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    @GAME_ID = $('.game').data 'id' # global game item id

    initSpin()

    createTemplates()

    initToolbar()

    initSwitches()
    initPills()
    initRatings()

    initRuby()

    bindButtons()
    bindTts()
    bindSearch()
    bindDraggable()

    #initBootstrap()

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF

#@bindYoutube = ->
#  dprint 'bindYoutube: enter'
#  # @param  id  str
#  render = (id) ->
#    # width/height = 16/9
#    #Haml.render " #%iframe(width="480" height="270" src="http://youtube.com/embed/#{id}" frameborder="0" allowfullscreen)"
#    w = Math.round zoomer.youtubeFrameWidth()  # 480 by default
#    h = Math.round zoomer.youtubeFrameHeight() # 270 by default
#    """<iframe width="#{w}" height="#{h}" src="http://youtube.com/embed/#{id}" frameborder="0" allowfullscreen />"""
#
#  # Bind this at first!
#  #$('a.badge').click -> @classList.add 'badge-info' #; false
#  #$('a.label,a .label').click -> @classList.add 'label-info' #; false
#  #$('.label a').click -> $(@).parent().addClass 'label-info'
#
#  #ITEM_ID = $('body').data 'id'
#  $('.btn-dl-img').click -> gameBean.saveImages(); false
#  $('.btn-dl-yt').click -> gameBean.saveVideos(); false
#
#  #if MAINLAND # sina
#  #  $('.youtube .btn-dl,.youtube .btn-play,.youtube .label,.youtube img').click ->
#  #    vid = $(@).closest('.youtube').data 'id'
#  #    #dprint vid
#  #    youtubeBean.get vid
#  #    false
#
#  #else # not sina
#  if true
#    $('.youtube .btn-dl').click ->
#      vid = $(@).closest('.youtube').data 'id'
#      #dprint vid
#      youtubeBean.get vid
#      false
#
#    $('.youtube .btn-play,.youtube .label,.youtube img').click -> # click play button, label, image
#      $youtube = $(@).closest '.youtube'
#      $label = $youtube.find '.label'
#      $img = $youtube.find 'img'
#      $iframe = $youtube.find 'iframe'
#      $btn = $youtube.find '.btn-play'
#      if $iframe.length # remove iframe
#        $btn.prop 'title', '再生'
#            .find('.fa-stop').removeClass('fa-stop').addClass 'fa-play'
#        $label.removeClass 'label-info'
#              .removeClass 'label-success'
#        $iframe.remove()
#        w = zoomer.youtubeWidth()
#        h = ''
#        $youtube.removeClass 'iframe'
#                .width w
#                .height h
#        $img.show()
#      else # add iframe
#        $btn.prop 'title', '停止'
#            .find('.fa-play').removeClass('fa-play').addClass 'fa-stop'
#        $label.removeClass 'label-info'
#              .addClass 'label-success' # .label.info
#        $img.hide()
#        w = '100%'
#        h = zoomer.youtubeFrameHeight() + 30
#        $youtube.addClass 'iframe'
#                .width w
#                .height h
#        id = $youtube.data 'id' # youtube id
#        h = render id
#        $youtube.append h
#      false
#
#  dprint 'bindYoutube: leave'
#
#class Scheduler
#  constructor: (@callback, @interval=50, @timerId=0) ->
#
#  active: => @timerId isnt 0
#
#  schedule: (@interval, @callback) =>
#    @timeId = setTimeout @trigger, @interval
#
#  trigger: =>
#    if @timerId
#      clearTimeout @timerId
#      @timerId = 0
#    @callback?()
#
#initRuby = ->
#  dprint 'bindRuby: enter'
#
#  $.fn.inject = -> # create inject plugin
#      @each -> window.injectruby @
#  $('.ruby').inject()
#
#
#  s = new Scheduler
#
#  readEvent = (e, t, interval) -> # mouse event, unicode t, int interval
#    s.schedule interval, ->
#      clipbean.text = t
#      ttsBean.speak t
#
#  popupEvent = (e, t, interval) -> # mouse event, unicode t, int interval
#    $window = $ window
#    x = e.pageX - $window.scrollLeft() + viewBean.x()
#    y = e.pageY - $window.scrollTop() + viewBean.y()
#    s.schedule interval, ->
#      clipbean.text = t
#      shioriBean.popup t, x, y
#      ttsBean.speak t
#
#  CLICK_TIMEOUT = 150 # about half of double click interval on Windows (250 msecs)
#
#  #$('.ruby:not(.article)').each ->
#  #  @title = '音声合成（クリック）, 振仮名（ダブルクリック）'
#
#  $ '.ruby ruby'
#    .dblclick (e) ->
#      t = $.trim $(@).find('rb').text()
#      popupEvent e, t, 0 if t
#    .click ->
#      t = $.trim $(@).find('rb').text()
#      viewBean.rehighlight t
#
#  $ '.yomi'
#    .each ->
#      unless @title
#        t = $.trim $(@).text()
#        @title = "読: " + yomiBean.toYomi t
#    .dblclick (e) ->
#      t = $.trim $(@).text()
#      popupEvent e, t, 0 if t
#    .click (e) ->
#      t = $.trim $(@).text()
#      #viewBean.rehighlight t # disabled as highlighted bootstrap labels looks ugly
#      readEvent e, t, CLICK_TIMEOUT if t
#
#  $ '.ruby:not(.article),.ruby .sentence'
#    .click (e) ->
#      #h = @innerHTML
#      t = $.trim $(@).find('rb').text()
#      readEvent e, t, CLICK_TIMEOUT if t
#      #false
#    .hover ->
#      unless @title
#        $this = $ @
#        unless $this.hasClass 'pass'
#          $this.addClass 'pass'
#          t = $.trim $this.find('rb').text()
#          if t and yakuBean.enabled()
#            @title = "ちょっとまってて><"
#            t = yakuBean.yaku t
#            if t
#              @title = "訳: " + t
#              #$this.tooltip placement: 'bottom'
#              #$this.tooltip 'show'
#            else
#              @title = ""
#              $this.removeClass 'pass'
#            #tooltip $(@), t if t
#
#  dprint 'bindRuby: leave'
