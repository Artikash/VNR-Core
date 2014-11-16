# inject-site.coffee
# 11/14/2014 jichi
# Invoked by QWebFrame::evaluaeJavaScript
# Beans:
# - cdnBean: coffeebean.CdnBean
# - settingsBean: coffeebean.SettingsBean

(->
  toggleclass = (el, cls, t) -> # element, string, booleaan
    if t then el.classList.add cls else el.classList.remove cls

  toggleclass @, 'site-opt-tr', true
).apply document.body

unless @siteInjected
  @siteInjected = true

  linkcss = (url) -> # string -> el  return the inserted element
    el = document.createElement 'link'
    #el.type = 'text/css'
    el.rel = 'stylesheet'
    el.href = cdnBean.url url #+ '.css'
    document.head.appendChild el
    el

  linkjs = (url) -> # string -> el  return the inserted element
    el = document.createElement 'script'
    el.src = cdnBean.url url #+ '.js'
    document.body.appendChild el
    el

  linkcss 'client-site.css'

  linkjs 'jquery' #unless @$ # force loading multiple version of jquery!

  linkjs 'jquery.ui' unless @$?.ui
  linkjs 'haml' unless @Haml

  linkjs 'client-site'

# EOF
