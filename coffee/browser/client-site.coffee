# client-site.coffee
# 11/15/2014 jichi
#
# Libraries: jquery, haml
#
# Beans:
# - trBean
#   - translators: -> string
#   - translate: string, string -> string

WAIT_TIMEOUT = 3000

SEL_TEXT = '#hasc_text'
SEL_IFRAME = '#game_frame'

## Templates ##

LAYOUT_ID = 'sk-text'
LAYOUT_HAML = '''\
#sk-main
  .header(title="Draggable")
    .button.close(title="Close") &times;
  .body
    .content
'''

HAML_TEXT = '''\
.text #{text}
'''

HAML_TR = '''\
.tr(data-key="#{key}" title="#{key}") #{text}
'''

## Controller ##

class MainObject
  id: LAYOUT_ID

  constructor: ->
    h = Haml.render LAYOUT_HAML
    @$el = $ h
      .hide()
      .appendTo document.body

    $header = @$el.children '.header'
    $body = @$el.children '.body'
    @$el.draggable handle:$header

    @$text = $body.children '.content'

    @hamlText = Haml HAML_TEXT
    @hamlTr = Haml HAML_TR

  pageBreak: =>
    if @$el.is ':visible'
      @$text.empty()
    else
      @show()

  show: => @$el.fadeIn()
  hide: => @$el.fadeOut()

  addText: (text) => # string ->
    @pageBreak()
    h = @_renderText text
    $ h
      .hide()
      .appendTo @$text
      .fadeIn()

    @_translate text

  addTr: (text, key) => # string, string ->
    h = @_renderTr text, key
    $ h
      .hide()
      .appendTo @$text
      .fadeIn()

  # Render

  _renderText: (text) =>
    @hamlText text:text

  _renderTr: (text, key) =>
    @hamlTr
      text: text
      key: key

  # Translate

  _translate: (text) =>
    s = trBean.translators()
    if s
      for key in s.split ','
        tr = trBean.translate text, key
        if tr
          @addTr tr, key

## Options ##

isTranslationEnabled = -> document.body.classList.contains 'site-opt-tr'

## Main ##

run = ->
  $iframe = $(SEL_IFRAME).contents()
  unless $iframe.length
    console.log 'vnr: wait for iframe ready'
    setTimeout run, WAIT_TIMEOUT
    return
  $text = $iframe.find SEL_TEXT
  unless $text.length
    console.log 'vnr: wait for text ready'
    setTimeout run, WAIT_TIMEOUT
    return

  MAIN = new MainObject
  # http://stackoverflow.com/questions/15657686/jquery-event-detect-changes-to-the-html-text-of-a-div
  $text.bind 'DOMSubtreeModified', -> # invoked when HTML content is changed
    t = $.trim @textContent
    MAIN.addText t if t

init = ->
  if @$ and @$.ui and @Haml # wait until needed modules are available
    $ ->
      if $(SEL_IFRAME).length
        run()
      else
        console.log 'vnr: wait for iframe ready'
        setTimeout init, WAIT_TIMEOUT
  else
    console.log 'vnr: wait for libraries ready'
    setTimeout init, WAIT_TIMEOUT

init()

# EOF
