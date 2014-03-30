# inject.coffee
# 3/28/2014 jichi
# Invoked by QWebFrame::evaluaeJavaScript
# Beans:
# - cdnBean
# - settingsBean

getdataset = ->
  ret = if settingsBean.isJlpEnabled() then ['enabled'] else ['disabled']
  ret.push 'tts' if settingsBean.isTtsEnabled()
  ret.join ' '

document.body.dataset.inject = getdataset()

# Make sure this script is only evaluated once
unless @injected
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
