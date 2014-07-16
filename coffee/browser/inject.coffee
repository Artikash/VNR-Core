# inject.coffee
# 3/28/2014 jichi
# Invoked by QWebFrame::evaluaeJavaScript
# Beans:
# - cdnBean: coffeebean.CdnBean
# - settingsBean: coffeebean.SettingsBean

getdataset = -> # -> string  the value of %body[data-inject[
  ret = if settingsBean.isJlpEnabled() then ['enabled'] else ['disabled']
  ret.push 'tts' if settingsBean.isTtsEnabled()
  ret.join ' '

repaint = -> # ->
  # http://stackoverflow.com/questions/3485365/how-can-i-force-webkit-to-redraw-repaint-to-propagate-style-changes
  v = document.body.className
  document.body.className += ' inject-dummy'
  document.body.className = v
  #if @$
  #  $('body').addClass('inject-dummy').removeClass('inject-dummy')
  #  $('body').addClass('inject-dummy').delay(0).removeClass('inject-dummy')

document.body.dataset.annot = getdataset()

# Make sure this script is only evaluated once
if @injected
  repaint()
else
  @injected = true

  linkcss = (url) -> # string -> el  return the inserted element
    el = document.createElement 'link'
    #el.type = 'text/css'
    el.rel = 'stylesheet'
    el.href = url #+ '.css'
    document.head.appendChild el
    el

  linkjs = (url) -> # string -> el  return the inserted element
    el = document.createElement 'script'
    el.src = url #+ '.js'
    document.body.appendChild el
    el

  linkcss cdnBean.url 'browser.css'
  linkjs cdnBean.url 'browser'

# EOF

#repaint = -> # Force a repaint
#  # http://stackoverflow.com/questions/3485365/how-can-i-force-webkit-to-redraw-repaint-to-propagate-style-changes
#  # http://stackoverflow.com/questions/8840580/force-dom-redraw-refresh-on-chrome-mac
#  if document.body
#    v = document.body.style.display
#    v = 'block'
#    if v isnt 'none'
#      document.body.style.display = 'none'
#      h = document.body.offsetHeight
#      document.body.style.display = v
