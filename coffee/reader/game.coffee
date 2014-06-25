# game.coffee
# 8/15/2013 jichi
# Required by game.haml

#dprint = -> console.log.apply console, arguments
dprint = ->

#bind = ->
#  #$('[title]').tooltip() # use bootstrap tooltip

#defer = (interval, fn) -> setTimeout fn, interval

# HAML for sample images
HAML_SAMPLE_IMAGE = Haml '''\
%a(href="#{url}",title="#{url}")
  %img.img-rounded(src="#{url}")
'''

## Render ##

_renderSampleImage = (url) ->
  HAML_SAMPLE_IMAGE url:url

renderSampleImages = -> # -> string  html
  gameBean.sampleImages.split(',').map(_renderSampleImage).join ''

## Bindings ##

@bindAmazon = ->
  dprint 'bindAmazon: enter'
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
  dprint 'bindAmazon: leave'

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

@bindYoutube = ->
  dprint 'bindYoutube: enter'
  # @param  id  str
  render = (id) ->
    # width/height = 16/9
    #Haml.render " #%iframe(width="480" height="270" src="http://youtube.com/embed/#{id}" frameborder="0" allowfullscreen)"
    w = Math.round zoomer.youtubeFrameWidth()  # 480 by default
    h = Math.round zoomer.youtubeFrameHeight() # 270 by default
    """<iframe width="#{w}" height="#{h}" src="http://youtube.com/embed/#{id}" frameborder="0" allowfullscreen />"""

  # Bind this at first!
  #$('a.badge').click -> @classList.add 'badge-info' #; false
  #$('a.label,a .label').click -> @classList.add 'label-info' #; false
  #$('.label a').click -> $(@).parent().addclass 'label-info'

  #ITEM_ID = $('body').data 'id'
  $('.btn-dl-img').click -> gameBean.saveImages(); false
  $('.btn-dl-yt').click -> gameBean.saveVideos(); false

  if MAINLAND # sina
    $('.youtube .btn-dl,.youtube .btn-play,.youtube .label,.youtube img').click ->
      vid = $(@).closest('.youtube').data 'id'
      #dprint vid
      youtubeBean.get vid
      false

  else # not sina
    $('.youtube .btn-dl').click ->
      vid = $(@).closest('.youtube').data 'id'
      #dprint vid
      youtubeBean.get vid
      false

    $('.youtube .btn-play,.youtube .label,.youtube img').click -> # click play button, label, image
      $youtube = $(@).closest '.youtube'
      $label = $youtube.find '.label'
      $img = $youtube.find 'img'
      $iframe = $youtube.find 'iframe'
      $btn = $youtube.find '.btn-play'
      if $iframe.length # remove iframe
        $btn.prop 'title', '再生'
            .find('.fa-stop').removeClass('fa-stop').addClass 'fa-play'
        $label.removeClass 'label-info'
              .removeClass 'label-success'
        $iframe.remove()
        w = zoomer.youtubeWidth()
        h = ''
        $youtube.removeClass 'iframe'
                .width w
                .height h
        $img.show()
      else # add iframe
        $btn.prop 'title', '停止'
            .find('.fa-play').removeClass('fa-play').addClass 'fa-stop'
        $label.removeClass 'label-info'
              .addClass 'label-success' # .label.info
        $img.hide()
        w = '100%'
        h = zoomer.youtubeFrameHeight() + 30
        $youtube.addClass 'iframe'
                .width w
                .height h
        id = $youtube.data 'id' # youtube id
        h = render id
        $youtube.append h
      false

  dprint 'bindYoutube: leave'

## Zoom ##

# Must be consistent with game.sass

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
    @ratio = v
    $('img.zoom.zoom-cover').width v * 230
    #$('img.zoom.zoom-cg').width v * 220
    #$('.chara').width v * 100 # TO BE RESTOERD

    # Refresh masonry
    $el = $ 'section.cg .images'
    if $el.length
      $el.children('img').width v * 220
      $el.masonry()

    @zoomYoutube v

  youtubeWidth: (v) => Math.min((v ? @ratio) * 220, @maxWidth())
  zoomYoutube: (v) => # float
    $('.youtube:not(.iframe)').width @youtubeWidth v

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

initBootstrapSwitch = ->
  do -> # CG switch
    $sec = $ 'section.cg'
    $sec.find('input.switch').bootstrapSwitch()
      .on 'switchChange.bootstrapSwitch', (event, checked) ->
        $el = $sec.find '.images'
        unless checked
          $el.fadeOut()
        else if $container.hasClass 'loaded'
          $el.fadeIn()
        else
          $el.hide()
             .html renderSampleImages()
             .addClass 'loaded'
             .fadeIn()
             .masonry itemSelector:'img'
             .imagesLoaded -> $el.masonry() # refresh after images are loaded

## Bootstrap ##

#initBootstrap = ->
#  $('[title]').tooltip()

## Main ##

init = ->
  unless @gameBean?
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    initToolbar()

    initBootstrapSwitch()

    initRuby()

    #initBootstrap()

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF

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
