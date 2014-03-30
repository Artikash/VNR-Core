# inject.coffee
# 3/28/2014 jichi
# Invoked by QWebFrame::evaluaeJavaScript
# Beans:
# - cdnBean
# - clipBean
# - jlpBean
# - ttsBean

# Make sure this script is only evaluated once
return if @injected
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
