# browser.coffee
# 3/28/2014 jichi
# Invoked by inject.coffee
# Beans:
# - cdnBean: beans.CdnBean
# - clipBean: skwebkit.SkClipboardProxy
# - jlpBean: beans.JlpBean
# - ttsBean: beans.TtsBean

# Underscore

WHITE_SPACES = [
  ' ', '\n', '\r', '\t', '\f', '\v', '\u00A0', '\u1680', '\u180E',
  '\u2000', '\u2001', '\u2002', '\u2003', '\u2004', '\u2005', '\u2006',
  '\u2007', '\u2008', '\u2009', '\u200A', '\u2028', '\u2029', '\u202F',
  '\u205F', '\u3000'
]

toString = (val) -> if val? then val.toString() else ''

ltrim = (str, chars=WHITE_SPACES) ->
  str = toString str

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
        ++start
        break

  if start >= len then '' else str.substr(start, len)

rtrim = (str, chars=WHITE_SPACES) ->
  str = toString str

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
        --end
        break

  if end >= 0 then str.substring(0, end + 1) else ''

trim = (str, chars=WHITE_SPACES) ->
  str = toString str
  ltrim(rtrim(str, chars), chars)

## Helpers

# half-width/full-width letters, numbers, and some half-width punctuations
isalnum = (ch) -> /[0-9a-zA-Z０-９ａ-ｚＡ-Ｚ]/.test ch # string ->bool

# half-width punctuations
#ispunct = (ch) -> /['"?!,\.]/.test ch # string -> bool

isalnumpunct = (ch) -> /[0-9a-zA-Z０-９ａ-ｚＡ-Ｚ'"?!,\.]/.test ch # string ->bool

## Render

# %span sentence
#   %ruby(class=word#{number})
#     %rb text
#     %rt ruby
renderruby = (text, ruby, feature, className) -> # must be consistent with parseparagraph in mecabrender.py
  rb = document.createElement 'rb'
  rb.textContent = text
  rt = document.createElement 'rt'
  rt.textContent = ruby
  ret = document.createElement 'ruby'
  ret.title = feature if feature
  ret.className = className
  ret.appendChild rb
  ret.appendChild rt
  ret.ondblclick = ->
    ttsBean.speak text if text and ttsBean.isEnabled()
  ret

renderrepl = (text) -> # string -> node
  data = jlpBean.parse text
  if data
    ret = document.createDocumentFragment()
    for sentence in JSON.parse data
      seg = document.createElement 'span'
      seg.className = 'inject-ruby'
      segtext = ''
      firstletter = lastletter = false
      for word in sentence # word = [surf, ruby, feature, className]
        surf = word[0]

        firstletter = isalnum surf[0]
        if firstletter and lastletter
          segtext += ' ' # not needed, but there is no drawbacks
          space = document.createTextNode ' '
          #space.class = 'inject-space' # not needded
          seg.appendChild space
        lastletter = isalnumpunct surf.slice -1

        segtext += surf
        ruby = renderruby.apply @, word
        seg.appendChild ruby
      if segtext
        seg.onclick = do (segtext) ->-> # bind segtext
          ttsBean.speak segtext if ttsBean.isEnabled()
      ret.appendChild seg
    ret

rendersrc = (el) -> # node -> node
  if el.nodeName is '#text'
    ret = document.createElement 'span'
    ret.className = 'inject-src'
    ret.textContent = el.textContent
    ret
  else
    el.cloneNode()

## Inject

# http://stackoverflow.com/questions/9452340/iterating-through-each-text-element-in-a-page
# http://javascript.info/tutorial/traversing-dom
#itertextnodes = (node, callback) -> # DocumentElement, function(DocumentElement) ->
#  node = node.firstChild
#  while node?
#    if node.nodeType is 3
#      callback node
#    else if node.nodeType is 1
#      itertextnodes node, callback
#    node = node.nextSibling

collecttextnodes = (node, ret) -> # DocumentElement, [] ->
  node = node.firstChild
  while node?
    if node.nodeType is 3
      ret.push node
    else if node.nodeType is 1
      collecttextnodes node, ret
    node = node.nextSibling

inject = (el) -> # DocumentElement ->
  nodes = []
  collecttextnodes el, nodes
  for node in nodes
    text = trim node.textContent
    if text
      repl = renderrepl text
      if repl
        src = rendersrc node
        repl.appendChild src
        node.parentNode.replaceChild repl, node
        #node.className += ' inject-src'
        #node.parentNode.insertBefore repl, node

## Main

do ->
  if document.body?
    inject document.body
  else
    window.onload = -> inject document.body
  #if @$
  #  $ ->
  #    #// jQuery plugin, example:
  #    $.fn.inject = ->
  #        @each -> inject @
  #    $('body').inject()

# EOF
