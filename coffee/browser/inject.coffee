# inject.coffee
# 3/28/2014 jichi
# Invoked by QWebFrame::evaluaeJavaScript
# Beans:
# - jlpBean
# - ttsBean


# Underscore

WHITE_SPACES = [
  ' ', '\n', '\r', '\t', '\f', '\v', '\u00A0', '\u1680', '\u180E',
  '\u2000', '\u2001', '\u2002', '\u2003', '\u2004', '\u2005', '\u2006',
  '\u2007', '\u2008', '\u2009', '\u200A', '\u2028', '\u2029', '\u202F',
  '\u205F', '\u3000'
]

toString = (val) -> if val? then val.toString() else ''

ltrim = (str, chars) ->
  str = toString str
  chars = chars or WHITE_SPACES

  start = 0
  len = str.length
  charLen = chars.length
  found = true

  while found and start < len
    found = false
    i = -1
    c = str.charAt start

    while ++i < charLen
      if c is chars[i]
        found = true
        start++
        break

  if start >= len then '' else str.substr(start, len)

rtrim = (str, chars) ->
  str = toString str
  chars = chars or WHITE_SPACES

  end = str.length - 1
  charLen = chars.length
  found = true

  while found and end >= 0
    found = false
    i = -1
    c = str.charAt end

    while ++i < charLen
      if c is chars[i]
        found = true
        end--
        break

  if end >= 0 then str.substring(0, end + 1) else ''

trim = (str, chars) ->
  str = toString str
  chars = chars or WHITE_SPACES
  ltrim(rtrim(str, chars), chars)

## Render

# %span sentence
#   %ruby(class=word#{number})
#     %rb text
#     %rt ruby
renderruby = (text, ruby, feature, className) ->
  rb = document.createElement 'rb'
  rb.textContent = text
  rt = document.createElement 'rt'
  rt.textContent = ruby
  ret = document.createElement 'ruby'
  ret.title = feature if feature
  ret.className = className
  ret.appendChild rb
  ret.appendChild rt
  ret

rendertext = (text) -> # string -> node
  ret = document.createDocumentFragment()

  data = jlpBean.parse text
  if data
    for sentence in JSON.parse data
      seg = document.createElement 'span'
      seg.className = 'inject-sentence'
      for word in sentence
        #[surf, ruby, feature, className] = word
        ruby = renderruby.apply @, word
        seg.appendChild ruby
      ret.appendChild seg
    ret

## Inject

itertextnodes = (node, callback) -> # DocumentElement, function(DocumentElement) ->
  # Note: the first node is not traversed
  #if node.nodeType is 3
  #  callback node
  node = node.firstChild
  while node
    if node.nodeType is 3
      callback node
    else if node.nodeType is 1
      itertextnodes node, callback
    node = node.nextSibling

# http://stackoverflow.com/questions/9452340/iterating-through-each-text-element-in-a-page
inject = (el) -> # DocumentElement ->
  itertextnodes el, (node) ->
    text = trim node.textContent
    if text
      repl = rendertext text
      if repl
        node.parentNode.replaceChild repl, node

## Main

linkcss = (url) -> # string -> el  return the inserted element
  el = document.createElement 'link'
  #el.type = 'text/css'
  el.rel = 'stylesheet'
  el.href = url #+ '.css'
  document.head.appendChild el
  el

linkcss cdnBean.url 'inject.css'

#window.onload = ->
#  inject document.body
inject document.body

#do ->
#  if @$
#    $ ->
#      #// jQuery plugin, example:
#      $.fn.inject = ->
#          @each -> inject @
#      $('body').inject()
#  else
#    console.log 'vnr:inject: missing jquery'

# EOF
