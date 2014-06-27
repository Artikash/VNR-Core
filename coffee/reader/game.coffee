# game.coffee
# 8/15/2013 jichi
# Required by game.haml

#dprint = -> console.log.apply console, arguments
dprint = ->

#bind = ->
#  #$('[title]').tooltip() # use bootstrap tooltip

#defer = (interval, fn) -> setTimeout fn, interval
#

DEFAULT_COVER_IMAGE_WIDTH = 230
DEFAULT_SAMPLE_IMAGE_WIDTH = 220
DEFAULT_VIDEO_IMAGE_WIDTH = 220

INVALID_YT_IMG_WIDTH = 120 # invalid youtube thumbnail image width

# Global translation function
@tr = (text) -> i18nBean.tr text # string ->

# Delay template creation until i18nBean becomes available
createTemplates = ->
  # HAML for sample images
  # - param  url
  @HAML_SAMPLE_IMAGE = Haml '''\
%a(href="#{url}" title="#{url}")
  %img.img-rounded.zoom.zoom-cg(src="#{url}")
'''

  # HAML for youtube video
  # - param  vid
  # - param  date
  # - param  title
  # - param  image  url
  @HAML_VIDEO = Haml """\
.video(data-id="${vid}")
  .header
    %a.tts(role="button" data-text="${title}" title="TTS")
      %span.fa.fa-volume-down
    %a.title.cursor-pointer(title="#{tr 'Play'}") ${title}
    :if date
      .date ${date}
    .toolbar.pull-right
      %button.close(type="button" title="#{tr 'Close'}") &times
      %a.btn.btn-link.btn-xs(role="button" href="http://youtube.com/watch?v=${vid}" title="http://youtube.com/watch?v=${vid}")
        %span.fa.fa-external-link
  .image
    %img.img-rounded(src="${img}" title="${title} ${date}")
  .iframe
""".replace /\$/g, '#'

## Render ##

createTwitterTimeline = (id:id, el:el, callback:callback, options:options) ->
  twttr.widgets.createTimeline id, el, callback, options #if window.twttr

_renderSampleImage = (url) ->
  HAML_SAMPLE_IMAGE url:url

renderSampleImages = -> # -> string  html
  gameBean.getSampleImages().split(',').map(_renderSampleImage).join ''

_renderVideo = (params) ->
  params.date = '' unless params.date? # fill in the missing date
  params.img = gameBean.getYouTubeImageUrl params.vid, true # large = true
  HAML_VIDEO params

renderVideos = -> # -> string  html
  JSON.parse(gameBean.getVideos()).map(_renderVideo).join ''

renderVideoIframe = (vid) -> # string -> string
  """<iframe width="480" height="360" src="http://youtube.com/embed/#{vid}" frameborder="0" allowfullscreen />"""

## Bindings ##

@initAmazonReview = ->
  dprint 'initAmazonReview: enter'
  #setTimeout(iframe, 3000);
  # Load function does not work on IE
  # See: http://stackoverflow.com/questions/4548984/detect-if-the-iframe-content-has-loaded-successfully
  $iframe = $ 'iframe.amazon'
  $iframe.load ->
    $this = $ @
    $body = $this.contents().find 'body'
    unless $body.html()
      $this.remove() # in case of 403 error
    else
      $body.css 'background-color', 'transparent' # transparent background
           .find('.crIFrame').css 'margin-left', '-0.8em' # remove left margin
      $body.find('.crIFrameLogo,.crIframeHeaderTitle').remove() # remove amazon logo and titles
      #$body.find('crIFrameReviewList').addClass 'ruby'

    # http://stackoverflow.com/questions/217776/how-to-apply-css-to-iframe
    #$head = $("iframe.amazon").contents().find 'head'
    #url = "#{rc.cdn_url('iframe.css')}"
    #$head.append($('<link/>',
    #  {rel: 'stylesheet', type: 'text/css', href: url}
    #));

  $(window).on 'scroll resize', ->
    # http://stackoverflow.com/questions/10083399/change-iframe-attribute-with-jquery
    # http://stackoverflow.com/questions/14913784/change-iframe-width-and-height-using-jquery
    #w = $window.width() # incorrect orz
    w = @viewBean.width()
    $iframe.width w - 74 # margin = 550 - 480 +4
    #$iframe[0].setAttribute 'width', w
    #$iframe.attr 'width', w

  $('iframe.amazon').error -> $(@).remove() # cannot detect 403 error, though
  dprint 'initAmazonReview: leave'

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

#@bindGyutto = ->
#  dprint 'bindGyutto: enter'
#  $iframe = $ 'iframe.gyutto'
#
#  $(window).on 'scroll resize', ->
#    #w = $window.width() # incorrect orz
#    w = @viewBean.width()
#    $iframe.width w - 74 # margin = 550 - 480 +4
#
#  $('iframe.gyutto').error -> $(@).remove() # on 404 error?
#  dprint 'bindGyutto: leave'

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
      $('section.cg .images').masonry() #columnWidth: v * DEFAULT_SAMPLE_IMAGE_WIDTH

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

## Bootstrap Switch ##

initCGSwitch = ->
  $section = $ 'section.cg'
  $section.find('input.switch').bootstrapSwitch()
    .on 'switchChange.bootstrapSwitch', (event, checked) ->
      $container = $section.find '.images'
      unless checked
        $container.fadeOut()
      else if $container.hasClass 'rendered'
        $container.fadeIn()
                  .masonry
      else
        $container.hide()
           .html renderSampleImages()
           .addClass 'rendered'
           .fadeIn()
           .masonry
             itemSelector: 'img'
             isFitWidth: true # centerize
           .imagesLoaded ->
             $container.find('img').width DEFAULT_SAMPLE_IMAGE_WIDTH * ZOOM_FACTOR
             $container.masonry() # refresh after images are loaded

initTwitterSwitch = ->

  $section = $ 'section.twitter'
  $section.find('input.switch').bootstrapSwitch()
    .on 'switchChange.bootstrapSwitch', (event, checked) ->
      if window.twttr

        $container = $section.find '.widgets'
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
                  width: 300
                  height: 500
                  chrome: 'transparent noborders noheader' # nofooter noscrollbar
                  showReplies: true
                  #tweetLimit: 20 # the maximum is 20

initYouTubeSwitch = ->
  $section = $ 'section.youtube'
  $section.find('input.switch').bootstrapSwitch()
    .on 'switchChange.bootstrapSwitch', (event, checked) ->
      $container = $section.find '.videos'
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

initSwitches = ->
  initCGSwitch()
  initTwitterSwitch()
  initYouTubeSwitch()

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
  $('.draggable').draggable()
  $('.draggable button.close').click ->
    $(@).closest('.draggable').fadeOut()

## Main ##

init = ->
  unless @gameBean?
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    createTemplates()

    initToolbar()

    initSwitches()

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
#  #$('.label a').click -> $(@).parent().addclass 'label-info'
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
